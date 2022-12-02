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

static ssize_t driver_read(struct file *filp, char __user *buf, size_t len, loff_t *off)

{

        pr_info("Read Function\n");

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

         		pr_err("DAta Write: Err\n");

         	}

         	break;

                case RD_LSMOD:

			char* ptr;

			struct file *f1;

			struct file *f2;

			char* file_name = kmalloc(32, GFP_KERNEL);

			char buff1[32];

			int i;

			for (i = 0; i < 32; i++)

				buff1[i] = 0;

			char buff2[32];

			for (i = 0; i < 32; i++)

				buff2[i] = 0;

                	struct user_lsmod begin = (struct user_lsmod) {

                		.module_names = THIS_MODULE->name,

				.module_ = THIS_MODULE,

                		.next = NULL

                	};

                	struct user_lsmod begin2 = (struct user_lsmod) {

                		.module_names = "",

                		.next = NULL

                	};

                	struct user_lsmod* user_modules = &begin;

                	struct user_lsmod* next_mod = &begin2;

                	struct list_head* list;

                	struct module* mod;

                	struct module* this_mod = THIS_MODULE;

                	//WARN_ON(0);

			pr_info("%-18s %-10s %-3s", "Module", "Size", "Used by");

                	list_for_each(list, &this_mod->list) {

                	    if(list_entry(list, struct module, list) != NULL)

                	    mod = list_entry(list, struct module, list);

                	    next_mod->module_names = mod->name;

                	    if (strcmp(mod->name, "") != 0) {

				user_modules->next = next_mod;

		        	    //pr_info("Module name is %s\n", mod->name);

		        	    strcpy(file_name, "/sys/module/");

		        	    strcat(file_name, mod->name);

		        	    strcat(file_name, "/coresize");

				    f1 = filp_open(file_name, O_RDONLY, 0);

				    if (f1 == NULL)

				    	pr_err("Failed to open file");

				    else {

				    //pr_info("open file");

				    	//fs = get_fs();

				    	//set_fs(get_ds());

				    	//f->f_op->read(f, buff, 256, &f->f_pos);

				    	kernel_read(f1, buff1, 16, &f1->f_pos);

				    	//set_fs(fs);

				    	filp_close(f1, NULL);

				    	for (ptr = buff1 + strlen(buff1) - 1; (ptr >= buff1) && isspace(*ptr); --ptr);

    					ptr[1] = '\0';

				    }

				    //strcat(buff, "   ");

				    strcpy(file_name, "/sys/module/");

		        	    strcat(file_name, mod->name);

		        	    strcat(file_name, "/refcnt");

				    f2 = filp_open(file_name, O_RDONLY, 0);

				    if (f2 == NULL)

				    	pr_err("Failed to open file");

				    else {

				    //pr_info("open file");

				    	//fs = get_fs();

				    	//set_fs(get_ds());

				    	//f->f_op->read(f, buff, 256, &f->f_pos);

				    	kernel_read(f2, buff2, 16, &f2->f_pos);

				    	//set_fs(fs);

				    	filp_close(f2, NULL);

				    	for (ptr = buff2 + strlen(buff2) - 1; (ptr >= buff2) && isspace(*ptr); --ptr);

    					ptr[1] = '\0';

				    }

				    //pr_info("Module: %s\n", mod->name);

				    //pr_info("Size: %s\n", buff1);

				    //pr_info("Used by: %s\n", buff2);

				    pr_info("%-18s %-10s %-3s", mod->name, buff1, buff2);

		        	    next_mod->next = NULL;

				    next_mod->module_ = mod;

				    user_modules = next_mod;

			    }

                	}

                	pr_info("Here we are!\n");

                	pr_info("We recorded first module %s\n", begin.module_names);

                	 if(copy_to_user((struct user_lsmod*) arg, &begin, sizeof(begin)))

				        {

				                pr_err("Data Write1 : Err!\n");

				        }

                        //pr_info("Value = %d\n", value);

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

        			.fpu_state_ = fpu_state_kernel

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