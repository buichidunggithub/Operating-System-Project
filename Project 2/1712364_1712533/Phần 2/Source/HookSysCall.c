#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable_types.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dung-Khoa");
//MY sys_call_table address = 0xffffffff820001e0

void **system_call_table_addr;
//my custom syscall that takes process name
asmlinkage int (*custom_syscall) (char* current);

/*hook*/
//Hook function, replace the system call OPEN
//Take the same parameter as the system call OPEN
asmlinkage int captain_hook(const char *pathname, int flags){
    
    //Print calling process name:
	printk(KERN_INFO "Calling process: %s\n", current->comm);

	//Print opening file
	printk(KERN_INFO "Opening file: %s\n", pathname);
    return custom_syscall(const char *pathname, int flags);
}

//Hook function, replace the system call WRITE
//Take the same parameter as the system call WRITE
ssize_t captain_hook(int fd, const void *buf, size_t count)
{
    // Print calling process name
    printk(KERN_INFO "Calling process:%s\n",current->comm);

    char pathname[255];
    fd_to_pathname(fd,pathname);
    printk(KERN_INFO "Written file: %s\n",pathname);

    int written_bytes = custom_syscall(fd, buf, count);

    printk(KERN_INFO "Number of written bytes:%d\n", written_bytes);

    return written_bytes;
}

static int __init entry_point(void){
    

	printk(KERN_INFO "Captain Hook loaded successfully..\n");
    /*MY sys_call_table address*/
    system_call_table_addr = (void*)0xffffffff820001e0;
  	
  	/*Assign custom_syscall to system call OPEN*/
    custom_syscall = system_call_table_addr[__NR_open];
    
    /*Disable page protection*/
    make_rw((unsigned long)system_call_table_addr);
    
    /*Change syscall to our syscall function*/
    system_call_table_addr[__NR_open] = captain_hook;
    
    return 0;
}


/*Make page writeable*/
int make_rw(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    if(pte->pte &~_PAGE_RW){
        pte->pte |=_PAGE_RW;
    }
    return 0;
}
/* Make the page write protected */
int make_ro(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}

static int __exit exit_point(void){
    
    printk(KERN_INFO "Unloaded Captain Hook successfully\n");
    
    /*Restore original system call */
    system_call_table_addr[__NR_open] = custom_syscall;
    
    /*Renable page protection*/
    make_ro((unsigned long)system_call_table_addr);
    return 0;
}
module_init(entry_point);
module_exit(exit_point);