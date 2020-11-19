obj-m := demomem.o
KDIR?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o *~ .depend.* *.mod *.order *.md *.ko *.symvers *.mod.c
