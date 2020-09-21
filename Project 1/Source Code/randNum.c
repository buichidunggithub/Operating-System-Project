#include <linux/init.h>
#include <linux/random.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "RandomNumberDevice"
#define CLASS_NAME "RandomNummoduleClass"

MODULE_LICENSE("GPL");          
MODULE_AUTHOR("Anh Quoc");    
MODULE_DESCRIPTION("A character device which generate integer randomly"); 
MODULE_VERSION("0.1");

static int openCount = 0;
static int majorNumber;
static struct class*  randomNumberClass  = NULL; ///< The device-driver class struct pointer
static struct device* randomNumberDevice = NULL; ///< The device-driver device struct pointer

static int Open(struct inode *inodePtr, struct file *filePtr);
static int Release(struct inode *inodePtr, struct file *filePtr);
static ssize_t Read(struct file *filePtr, char * buffer, size_t size, loff_t * offset);

static struct file_operations fileOperations =
{
   .open = Open,
   .read = Read,
   .release = Release,
};


static int __init randNum_init(void){
	//Register major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fileOperations);
	if (majorNumber < 0)
	{

		printk(KERN_INFO "RandomNumberDevice: Failed to register major number\n");
		return majorNumber;
	}

	printk(KERN_INFO "RandomNumberDevice: Registered correctly with major number is %d\n", majorNumber);

	//Create class
	randomNumberClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(randomNumberClass))
	{
		device_destroy(randomNumberClass, MKDEV(majorNumber, 0));
		printk(KERN_INFO "RandomNumberDevice: Failed to register random number class\n");
		return PTR_ERR(randomNumberClass);
	}


	//Create device
	randomNumberDevice = device_create(randomNumberClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(randomNumberDevice))
	{
		class_unregister(randomNumberClass);
		class_destroy(randomNumberClass);
		unregister_chrdev(majorNumber, DEVICE_NAME); 
		printk(KERN_INFO "RandomNumberDevice: Failed to register random number device\n");
		return PTR_ERR(randomNumberDevice);
	}
	return 0;
}

static void __exit randNum_exit(void){
	//destroy device
	device_destroy(randomNumberClass, MKDEV(majorNumber, 0));
	//destroy class
	class_unregister(randomNumberClass);
	class_destroy(randomNumberClass);

	//unregister major number
	unregister_chrdev(majorNumber, DEVICE_NAME); 
}

static int Open(struct inode *inodePtr, struct file *filePtr)
{
	openCount++;
   	printk(KERN_INFO "RandomNumberDevice: Device has been opened %d time(s)\n", openCount);
   	return 0;
}

static int Release(struct inode *inodePtr, struct file *filePtr)
{
	printk(KERN_INFO "RandomNumberDevice: Device successfully closed\n");
    return 0;

}

static ssize_t Read(struct file *filePtr, char * buffer, size_t size, loff_t * offset)
{
	int error_count = 0;
    int x;
    get_random_bytes(&x, sizeof(int));
    error_count = copy_to_user(buffer, &x, sizeof(x));
 
   	if (error_count==0){
    	printk(KERN_INFO "RandomNumberDevice: Sent number %d to the user\n", x);
      	return (sizeof(x));
   	}
   	else {
      	printk(KERN_INFO "RandomNumberDevice: Failed to send number %d to the user\n", x);
      	return -EFAULT;              
   }

}


module_init(randNum_init);
module_exit(randNum_exit);