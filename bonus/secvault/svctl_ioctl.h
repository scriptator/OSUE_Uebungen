#ifndef SVCTL_IOCTL
#define SVCTL_IOCTL

#define SV_IOC_MAGIC  ('j')
	
#define SV_CREATE_SECVAULT 	_IO(SV_IOC_MAGIC,  1)
#define SV_GET_SIZE			_IO(SV_IOC_MAGIC,  2)
#define SV_CHANGE_KEY		_IO(SV_IOC_MAGIC,  3)
#define SV_WIPE_SECVAULT	_IO(SV_IOC_MAGIC,  4)
#define SV_DELETE_SECVAULT	_IO(SV_IOC_MAGIC,  5)

#define SV_IOC_MAXNR 5
#define SECVAULT_KEY_LENGTH (10)

struct ioctl_data {
	unsigned int dev_nr;
	unsigned long size;
	char key[SECVAULT_KEY_LENGTH];
};

#endif /* SVCTL_IOCTL */
