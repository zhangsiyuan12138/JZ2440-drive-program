KERN_DIR=/work/systems/kernel/linux-2/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make  -C $(KERN_DIR) M=`pwd`  modules clean
	rm -fr modules.order                      

obj-m +=first_driver.ocat 

#  Document/Kbuild/modules 
#  make -C <path-to-kernel> M=`pwd`    当模块不在内核模块树中时        编译驱动模块的语法
#                                      make M=dir to specify directory of external module to build
#
#   modules: $(module-dirs)   
#	@echo '  Building modules, stage 2.';
#	$(Q)$(MAKE) -f $(srctree)/scripts/Makefile.modpost 
#
#   @make -f  ./scripts/Makefile.modpost
#   -f    make 在当前目录下找到 makefile/Makefile 并执行
#
#   scripts/Makefile.modpost
#    step1   
#           编译生成        first_drvice.o
#           链接所有的       <module>.o 文件
#
#   step2    
#          创建 first_drvice.mod.c    (预编译模块)
#          创建 Modules.symvers       (包含全局符号的CRC)
#          编译全部的    <module>.mod.c  模块
#          链接生成  first_drvice.ko    
#
#   step3  
#          在可执行文件中添加一些信息  （内核版本， GCC版本...）（模块版本，模块许可证...）
#
#   step4
#          Step 4 is solely used to allow module versioning in external modules,
#
#
#
#