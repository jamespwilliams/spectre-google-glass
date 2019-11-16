# Adapted from https://github.com/thoughtpolice/enable_arm_pmu/blob/master/ko/Makefile

CFLAGS += -march=armv7-a -mfloat-abi=softfp -mfpu=neon -std=c99 -O0 --sysroot=${NDK_SYS_ROOT}

obj-m	:= enable_arm_pmu.o
PWD	    := $(shell pwd)

all:
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- $(MAKE) CFLAGS_MODULE=-fno-pic CFLAGS='$(CFLAGS)' -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
