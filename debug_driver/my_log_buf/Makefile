KER_DIR=/work/systems/kernel/linux-2/linux-2.6.22.6
all:
	make -C $(KER_DIR) M=`pwd` modules
clean:
	make -C $(KER_DIR) M=`pwd` modules clean
	rm -fr modules.order

obj-m += log_buf_test.o
obj-m += my_log_buf.o
