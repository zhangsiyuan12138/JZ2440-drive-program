KERN_DIR = /work/systems/kernel/linux-2/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	arm-linux-gcc -o register_rw_test register_rw_test.c
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= register_rw.o

