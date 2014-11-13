Caraboot
========

U-Boot for Carambola2 based boards


Build
-------

Use buildroot toolchain (http://buildroot.org/download.html) to build Caraboot. OpenWRT toolchain is known to generate broken binaries.
To build suitable toolchain in buildroot's menuconfig select: MIPS big endian architecture (mips32 r2 variant), and GCC version - 4.7.x (other GCC versions untested, but may work)

Change Makefile's CONFIG_TOOLCHAIN_PREFIX variable to your buildroot path, for example:
```
CONFIG_TOOLCHAIN_PREFIX=~/build/buildroot/output/host/usr/bin/mips-linux-
```

run ```make```, binary will be in ```bin/carambola2_u-boot.bin```

