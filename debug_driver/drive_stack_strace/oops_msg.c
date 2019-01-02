
# ./drv_stack_strace_test
Unable to handle kernel paging request at virtual address 56000050
pgd = c3e5c000
[56000050] *pgd=00000000
Internal error: Oops: 5 [#2]
Modules linked in: drv_stack_strace_drv
CPU: 0    Not tainted  (2.6.22.6 #19)
PC is at first_drv_open+0x18/0x3c [drv_stack_strace_drv]
LR is at chrdev_open+0x14c/0x164
pc : [<bf000018>]    lr : [<c008cd18>]    psr: a0000013
sp : c06d1e88  ip : c06d1e98  fp : c06d1e94
r10: 00000000  r9 : c06d0000  r8 : c04d05a0
r7 : 00000000  r6 : 00000000  r5 : c3e2d0a0  r4 : c06c1240
r3 : bf000000  r2 : 56000050  r1 : bf0008c4  r0 : 00000000
Flags: NzCv  IRQs on  FIQs on  Mode SVC_32  Segment user
Control: c000717f  Table: 33e5c000  DAC: 00000015
Process drv_stack_strac (pid: 752, stack limit = 0xc06d0258)


Stack: (0xc06d1e88 to 0xc06d2000)
1e80:                   c06d1ebc c06d1e98 c008cd18 bf000010 c06d1f04 c04d05a0
                        first_drv_open'sp'   lr             chrdev_open'sp'
                        
1ea0: c3e2d0a0 c008cbcc c0465e20 c04f4c38 c06d1ee4 c06d1ec0 c0089248 c008cbdc
															   lr
															   
1ec0: c04d05a0 c06d1f04 00000003 ffffff9c c002b044 c04f6000 c06d1efc c06d1ee8
      __dentry_open'sp'
      
1ee0: c0089364 c0089158 00000000 00000002 c06d1f68 c06d1f00 c00893b8 c0089340
         lr             nameidata_to_filp'sp'                   lr
         
1f00: c06d1f04 c04f4c38 c0465e20 00000000 00000000 c3e5d000 00000101 00000001
      do_filp_open'sp'
      
1f20: 00000000 c06d0000 c0473748 c0473740 ffffffe8 c04f6000 c06d1f68 c06d1f48
1f40: c008956c c009f100 00000003 00000000 c04d05a0 00000002 bee86ecc c06d1f94
1f60: c06d1f6c c00896f4 c0089388 00008520 bee86ec4 0000860c 00008670 00000005
                   lr            do_sys_open'sp'
                   
1f80: c002b044 4013365c c06d1fa4 c06d1f98 c00897a8 c00896b0 00000000 c06d1fa8
                                              lr            sys_open'sp'
                                              
1fa0: c002aea0 c0089794 bee86ec4 0000860c 00008720 00000002 bee86ecc 00000001
        lr              ret_fast_syscall'sp'                                            // ret_fast_syscall 通过swi指令被调用
        
1fc0: bee86ec4 0000860c 00008670 00000001 00008520 00000000 4013365c bee86e98
1fe0: 00000000 bee86e74 0000266c 400c98e0 60000010 00008720 00000000 00000000



Backtrace:
[<bf000000>] (first_drv_open+0x0/0x3c [drv_stack_strace_drv]) from [<c008cd18>] (chrdev_open+0x14c/0x164)
[<c008cbcc>] (chrdev_open+0x0/0x164) from [<c0089248>] (__dentry_open+0x100/0x1e8)
 r8:c04f4c38 r7:c0465e20 r6:c008cbcc r5:c3e2d0a0 r4:c04d05a0
[<c0089148>] (__dentry_open+0x0/0x1e8) from [<c0089364>] (nameidata_to_filp+0x34/0x48)
[<c0089330>] (nameidata_to_filp+0x0/0x48) from [<c00893b8>] (do_filp_open+0x40/0x48)
 r4:00000002
[<c0089378>] (do_filp_open+0x0/0x48) from [<c00896f4>] (do_sys_open+0x54/0xe4)
 r5:bee86ecc r4:00000002
[<c00896a0>] (do_sys_open+0x0/0xe4) from [<c00897a8>] (sys_open+0x24/0x28)
[<c0089784>] (sys_open+0x0/0x28) from [<c002aea0>] (ret_fast_syscall+0x0/0x2c)
Code: e24cb004 e59f1024 e3a00000 e5912000 (e5923000)
