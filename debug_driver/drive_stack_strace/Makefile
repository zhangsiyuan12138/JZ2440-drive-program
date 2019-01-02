KERN_DIR = /work/systems/kernel/linux-2/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
    arm-linux-gcc -o drv_stack_strace_test drv_stack_strace_test.c
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= drv_stack_strace_drv.o
