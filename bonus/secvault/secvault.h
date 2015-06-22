#ifndef SECVAULT_H
#define SECVAULT_H

#define SECVAULT_MAJOR (231)
#define SECVAULT_NR_DEVS (4)

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
long    secvault_ctl_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);

/* Chardev structures */
struct sv_dev {
	struct cdev cdev;     /* Char device structure      */
	char *data;
	char key[SECVAULT_KEY_LENGTH];
	struct semaphore sem;
	ssize_t cur_size;
	ssize_t size;
};

struct sv_ctl_dev {
	struct cdev cdev;     /* Char device structure      */
	struct semaphore sem;
};

#endif /* SECVAULT_H */
