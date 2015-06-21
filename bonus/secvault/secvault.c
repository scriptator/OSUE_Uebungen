/* Secvault module */
/* Author: Johannes Vass <johannes.vass@student.tuwien.ac.at> */

#include <linux/module.h>
#include <linux/init.h>
//#include <linux/malloc.h>   /* kmalloc() */
#include <linux/cdev.h>
#include <linux/fs.h> 
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/moduleparam.h>
#include "secvault.h"

static int debug;
module_param(debug, bool, S_IRUGO);

struct file_operations secvault_fops = {
	.owner =	THIS_MODULE,
	.llseek =	secvault_llseek,
	.read =		secvault_read,
	.write =	secvault_write,
	.open =		secvault_open,
	.release =  secvault_release;
};

struct file_operations secvault_ctl_fops = {
	.owner =	THIS_MODULE,
	.ioctl =	secvault_ctl_ioctl,
	.open =		secvault_ctl_open,
	.release =  secvault_ctl_release;
};

static struct sv_ctl_dev control_dev;
static struct sv_dev[SECVAULT_NR_DEVS];

/************** Prototypes **********************/

static int  secvault_init(void);
static void secvault_cleanup(void);
static void DEBUG(char *msg);



/************** Implementations ******************/

static void secvault_cleanup(void)
{
	DEBUG("Starting Cleanup");
	
	cdev_del(&control_dev.cdev);
	unregister_chrdev_region(MKDEV(SECVAULT_MAJOR, 0),  SECVAULT_NR_DEVS + 1);
}

static int __init secvault_init(void)
{
	DEBUG("Starting the secvault-module.");
	
	/* Register character-device-region for all 5 devices */
	dev_t first_major = MKDEV(SECVAULT_MAJOR, 0);
    int result = register_chrdev_region(first_major, SECVAULT_NR_DEVS + 1, "secvault");
    if (result < 0) {
        printk(KERN_WARNING "secvault: can't get major %d\n",scull_major);
        return result;
    }	
	
	/* Create sv_ctl device now, the secvaults themselves get created via IOCTL later */
	sema_init(&control_dev.sem, 1);
	cdev_init(&control_dev.cdev, &secvault_ctl_fops);
	control_dev.cdev.owner = THIS_MODULE;
	
	result = cdev_add(&control_dev.cdev, first_major, 1);
	if (result < 0) {
        printk(KERN_WARNING "secvault: can't add control device\n");
		unregister_chrdev_region(first_major,  SECVAULT_NR_DEVS + 1);
        return result;
	}

	DEBUG("Successfully started the secvault-module.");
	return 0;
}

static void __exit secvault_exit(void)
{
	secvault_cleanup();
	DEBUG("Secvault successfully unloaded.");
}

static void DEBUG(char *msg)
{
	if (debug) {
		printk("Secvault DBG: %s\n", msg);
	}
}

module_init(secvault_init);
module_exit(secvault_exit);

MODULE_LICENSE("GPL");
