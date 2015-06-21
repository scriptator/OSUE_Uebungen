obj-m += secvault.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) ARCH=um V=1 modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) ARCH=um V=1 clean
