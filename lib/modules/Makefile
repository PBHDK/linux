KDIR = $(HOME)/src/git/DoitLK-linux/
obj-m = lkm.o

kbuild:
	make -C $(KDIR) M=`pwd`

# IR: x86_IR arm64_IR

arm64_IR:
	cd arm64 && make

x86_IR:
	cd x86 && make 

clean:
	make -C $(KDIR) M=`pwd` clean