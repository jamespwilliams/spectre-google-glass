PROGRAM = spectre-glass.out
KMODULE = ko/enable_arm_pmu.ko
SOURCE  = spectre-glass.c

CFLAGS += -march=armv7-a -mfloat-abi=softfp -mfpu=neon -std=c99 -O0 --sysroot=${NDK_SYS_ROOT}
     
all: $(PROGRAM) $(KMODULE)

install: all
	adb shell mkdir -p /data/spectre/
	adb push $(PROGRAM) /data/spectre/$(PROGRAM)
	adb push spectre-glass.sh /data/spectre/spectre-glass.sh
	adb push $(KMODULE) /data/spectre/$(KMODULE)

run: install
	adb shell chmod 777 /data/spectre/spectre-glass.sh
	adb shell sh data/spectre/spectre-glass.sh

$(KMODULE):
	$(MAKE) -C ko

$(PROGRAM): $(SOURCE)
	arm-linux-androideabi-gcc $(CFLAGS) -o $(PROGRAM) $(SOURCE)
     
clean:
	rm -f $(PROGRAM)
	$(MAKE) clean -C ko

