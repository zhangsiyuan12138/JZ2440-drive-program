KERN_DIR = /work/systems/kernel/linux-2/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= touch_screen.o
obj-m   += touch_screen_input.o
