# spectre-google-glass
An (non-working) attempt at an implementation of Spectre v1 for Google Glass. The exploit runs, but doesn't work - data is not exfiltrated correctly.

Produced as part of a coursework for CO332 (Advanced Computer Architecture) at Imperial College. 

Based heavily on https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6 and https://github.com/crozone/SpectrePoC.

### Requirements

- A pair of rooted Google Glass(es?). Tested with Explorer Edition, with firmware XE23.
- `arm-linux-androideabi-gcc` (installable with, for example, `sudo apt-get install gcc-arm-linux-androideabi`)
- The Android NDK (https://developer.android.com/ndk/downloads)
- A copy of the OMAP Android Kernel (https://android.googlesource.com/kernel/omap/+refs)
    - I checked out on the `glass-omap-xrw85` branch for this.
    - Follow the instructions here (https://developers.google.com/glass/tools-downloads/system) to build the kernel.
    
### Usage

```sh
export KDIR=<path_to_kernel>
export NDK_SYS_ROOT=<path_to_ndk>/sysroot
make run
```

- `make` will build the both kernel module and the exploit.
- `make install` will do all of the above, and copy the files to your device (in particular, to `/data/spectre/`)
- `make run` will do all of the above, and run the exploit using `adb shell`
