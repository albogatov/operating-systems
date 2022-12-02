obj-m += driver.o

 

KDIR = /lib/modules/$(shell uname -r)/build

KBUILD_CFLAGS   +=  -g -Wall

 

 

all:

	make -C $(KDIR)  M=$(shell pwd) modules

 

clean:

	make -C $(KDIR)  M=$(shell pwd) clean