#include <linux/ioctl.h>

#define SECVAULT_MAJOR (231)
#define SECVAULT_NR_DEVS (4)
#define SECVAULT KEY_LENGTH (10)


/*
 * Prototypes for shared functions
 */
int		secvault_open (struct inode *, struct file *);
int		secvault_release (struct inode *, struct file *);
ssize_t secvault_read (struct file *filp, char *buf, size_t count,loff_t *f_pos);
ssize_t secvault_write (struct file *filp, const char *buf, size_t count,loff_t *f_pos);
loff_t  secvault_llseek (struct file *filp, loff_t off, int whence);
		
int		secvault_ctl_open (struct inode *, struct file *);
int		secvault_ctl_release (struct inode *, struct file *);
int     secvault_ctl_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

/* Chardev structures */
struct sv_dev {
	unsigned long size,
	unsigned char key[KEY_LENGTH],
	struct semaphore sem,
	struct cdev cdev;     /* Char device structure      */
};

struct sv_ctl_dev {
	struct semaphore sem,
	struct cdev cdev;     /* Char device structure      */
};
