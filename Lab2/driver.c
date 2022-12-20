/***************************************************************************//**
*  \file       driver.c
*
*  \details    Lsmod & FPU State Linux device driver (IOCTL)
*
*  \author     abogatov
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 
#include <linux/uaccess.h>              
#include <linux/err.h>
#include <linux/pid.h> 
#include <asm/uaccess.h>
#include <linux/pid_namespace.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include "driver.h"
#define BUFFER  4096
static char lsmod[BUFFER];
static char* lsmod_ptr;
static char* lsmod_ptr_current;
struct task_struct* task;
struct thread_struct* threadp;
struct pid* pid;
struct fpu* fpu_kernel;
int32_t value = 0;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev driver_cdev;
/*
** Function Prototypes
*/
static int      __init driver_driver_init(void);
static void     __exit driver_driver_exit(void);
static int      driver_open(struct inode *inode, struct file *file);
static int      driver_release(struct inode *inode, struct file *file);
static ssize_t  driver_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  driver_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = driver_read,
        .write          = driver_write,
        .open           = driver_open,
        .unlocked_ioctl = driver_ioctl,
        .release        = driver_release,
};
/*
** This function will be called when we open the Device file
*/
static int driver_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int driver_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t driver_read(struct file *filp, char __user *buf, size_t length, loff_t *off)
{
        pr_info("Read Function\n");
        	if (*lsmod_ptr == 0) {
        		pr_info("nothing to transmit%s", lsmod_ptr);
				return 0;
        	}
		  while (length && *lsmod_ptr)  {
		    put_user(*(lsmod_ptr++), buf++);
		    length--;
		  }
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}
/*
** This function will be called when we write IOCTL on the Device file
*/
static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
         	case WR_VALUE:
         		if (copy_from_user(&value, (int32_t*) arg, sizeof(value))) {
         		pr_err("Data Write: Err\n");
         	}
         	break;
		case IOCTL_GET_MSG:
			int i;
                	struct list_head* list;
                	struct list_head* list_inner;
                	struct module* mod;
                	struct module_use* mod_inner;
                	struct module* this_mod = THIS_MODULE;
			pr_info("%-18s %-10s %-3s", "Module", "Size", "Used by");
			lsmod_ptr_current += sprintf(lsmod_ptr_current, "%-18s %-10s %-3s\n", "Module", "Size", "Used by");
			lsmod_ptr_current += sprintf(lsmod_ptr_current, "%-18s %-10d %-3u\n", this_mod->name, this_mod->core_layout.size, this_mod->refcnt);   
                	list_for_each(list, &(THIS_MODULE->list)) {
                		    mod = list_entry(list, struct module, list);
		        	    if (strcmp(mod->name, "") != 0) {
					    lsmod_ptr_current += sprintf(lsmod_ptr_current, "%-18s %-10d %-3ld", mod->name, mod->core_layout.size, module_refcount(mod));   
					    list_for_each_entry(mod_inner, &(mod->source_list), source_list) {
		        			lsmod_ptr_current += sprintf(lsmod_ptr_current, "%s, ", mod_inner->source->name);  
					    }
					    lsmod_ptr_current += sprintf(lsmod_ptr_current, "\n");   
				    }
                	}
                	pr_info("Here we are!\n");
                	driver_read(file, (char*) arg, BUFFER, 0);
                        break;
                case RD_FPU_STATE:
                	pr_info("Well it's started");
                	threadp = kmalloc(sizeof(struct thread_struct), GFP_KERNEL);
                	fpu_kernel = kmalloc(sizeof(struct fpu), GFP_KERNEL);
                	pid = find_get_pid((int) value);
        		task = pid_task(pid, PIDTYPE_PID);
        		threadp = &(task->thread);
        		fpu_kernel = &(threadp->fpu);
        		struct fpstate* fpu_state_kernel = kmalloc(sizeof(struct fpstate), GFP_KERNEL);
        		fpu_state_kernel = fpu_kernel->fpstate;
        		struct user_fpu_state user_fpstate = { 
        			.control_word = fpu_state_kernel->regs.xsave.i387.cwd,
        			.state_word = fpu_state_kernel->regs.xsave.i387.swd,
        			.last_instr_opcode = fpu_state_kernel->regs.xsave.i387.fop,
        			.instr_pointer = fpu_state_kernel->regs.xsave.i387.rip,
        			.data_pointer = fpu_state_kernel->regs.xsave.i387.rdp,
        			.ip_offset = fpu_state_kernel->regs.xsave.i387.fip,
        			.ip_selector = fpu_state_kernel->regs.xsave.i387.fcs,
        			.operand_offset = fpu_state_kernel->regs.xsave.i387.foo,
        			.operand_selector = fpu_state_kernel->regs.xsave.i387.fos,
        		};
        		pr_info("Copy init");
                        if(copy_to_user((struct user_fpu_state *) arg, &user_fpstate, sizeof(user_fpstate)) )
                        {
                                pr_err("Data Read : Err!\n");
                        }            
                        pr_info("Copy finished");
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}
/*
** Module Init function
*/
static int __init driver_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "driver_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 	    /*Creating cdev structure*/
        cdev_init(&driver_cdev,&fops);
        /*Adding character device to the system*/
        if((cdev_add(&driver_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"driver_class"))){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"driver_device"))){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
        lsmod_ptr = lsmod;
        lsmod_ptr_current = lsmod;
        return 0;
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
/*
** Module exit function
*/
static void __exit driver_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&driver_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
module_init(driver_driver_init);
module_exit(driver_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("abogatov");
MODULE_DESCRIPTION("Lsmod & FPU State Linux device driver (IOCTL)");
MODULE_VERSION("1.5");