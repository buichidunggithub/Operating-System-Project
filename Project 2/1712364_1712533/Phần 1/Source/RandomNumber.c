#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/random.h> //For get_random_bytes function
#include <linux/uaccess.h>
#define DEVICE_NAME "RandomMachine"
#define CLASS_NAME "Random"

MODULE_LICENSE("GPL v1"); //provides information about licensing terms of the module that you have developved
MODULE_AUTHOR("Dung-Khoa");
MODULE_DESCRIPTION("Generating randomly integer device"); 
MODULE_VERSION("0.1");

static int count = 0;
static int major; 

// class struct pointer driver
static struct class *randomNumberClass  = NULL; 
// device struct pointer driver
static struct device *randomNumberDevice = NULL; 


static int Open(struct inode *inodePtr, struct file *filep);
static ssize_t Read(struct file *filep, char *buffer, size_t size, loff_t *offset);
static int Release(struct inode *inodePtr, struct file *filep);

//This structure points to all of the device functions
static struct file_operations fops =
{
   .open = Open,
   .read = Read,
   .release = Release,
};


//Called when a process opens our device
static int Open(struct inode *inodePtr, struct file *filep)
{
	count++;
   	printk(KERN_ALERT "Device connected %d time(s)\n", count);
   	return 0;
}

//Called when a process reads our device
static ssize_t Read(struct file *filep, char *buffer, size_t size, loff_t *offset)
{

	int x;
	int error = 0;
    	/*sizeof(int)=4bytes*/
	get_random_bytes(&x, sizeof(int));
    	
	/*This below function copies the size bytes pointed at by x, which must exist in
	kernel-space, to buffer, which must exist in user-space. This function returns
	the number of bytes not copied, which means on success it returns zero
	*/
	error = copy_to_user(buffer, &x, sizeof(x));
 	
   	if (error==0){
    	printk(KERN_ALERT "Sent number %d to the user\n", x);
      	return (sizeof(x));
   	}
   	else {
      	printk(KERN_ALERT "Failed to send number %d to the user\n", x);
      	return -EFAULT;              
   	}

}


//@brief The LKM initialization function
/*The static keyword restricts the visibility of the function to within this C file. The __init macro
means that for a built-in driver (not a LKM) the function is only used at the initialization time
and that it can be discarded and its memory freed up after that point. Return returns 0 if successful*/
static int __init randNum_init(void)
{
	//Try to register character device
	//allocate major number
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0)
	{
		printk(KERN_ALERT "Could not register device: %d\n", major);
		return major;
	}

	//Register device class
	randomNumberClass = class_create(THIS_MODULE, CLASS_NAME);

	if (IS_ERR(randomNumberClass))
	{
		device_destroy(randomNumberClass, MKDEV(major, 0));
		printk(KERN_ALERT "Failed to register random number class\n");
		return PTR_ERR(randomNumberClass);
	}
	
	//Register device driver
	randomNumberDevice = device_create(randomNumberClass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	
	if (IS_ERR(randomNumberDevice))
	{
		class_unregister(randomNumberClass);
		class_destroy(randomNumberClass);
		unregister_chrdev(major, DEVICE_NAME); 
		printk(KERN_ALERT "Failed to register random number device\n");
		return PTR_ERR(randomNumberDevice);
	}

	printk(KERN_ALERT "Device class created correctly");
	return 0;
}


//@brief The LKM cleanup function
/* Similar to the initialization function, it is static. The __exit macro notifies that if this code
is used for a built-in driver (not LKM) that this function is not required*/
static void __exit randNum_exit(void)
{
	device_destroy(randomNumberClass, MKDEV(major, 0)); //Destroy device
	class_unregister(randomNumberClass);		    //Unregister device class	
	class_destroy(randomNumberClass);		    //Delete device class
	unregister_chrdev(major, DEVICE_NAME); 		    //unregister device major number
}

//Called when a process closes our device
static int Release(struct inode *inodePtr, struct file *filep)
{
	printk(KERN_ALERT "Device closed successfully\n");
	return 0;

}
module_init(randNum_init);
module_exit(randNum_exit);