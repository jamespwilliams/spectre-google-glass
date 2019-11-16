PROGRAM = spectre-glass.out
SOURCE  = spectre-glass.c

CFLAGS += -march=armv7-a -mfloat-abi=softfp -mfpu=neon -std=c99 -O0 --sysroot=${NDK_SYS_ROOT}
     
all: $(PROGRAM)

     
$(PROGRAM): $(SOURCE)
	arm-linux-androideabi-gcc $(CFLAGS) -o $(PROGRAM) $(SOURCE)
     
clean:
	rm -f $(PROGRAM)