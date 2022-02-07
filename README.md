Caraboot
========

U-Boot for Carambola2/Lima based boards


Build
-------

1) Build buildroot toolchain (http://buildroot.org)

Download:
```
cd your_work_dir
git clone git://git.buildroot.net/buildroot
git checkout 2016.02
```
  
Configure:
```
cd buildroot
make menuconfig
```
In Target Options select Target Architecture as MIPS (big endian) and Target Architecture Variant as mips 32r2.
In Toolchain select GCC Compiler version as gcc 4.7.x.
Save and exit.

Build:
```
make toolchain
```

2) Build Caraboot image

Download:
```
cd your_work_dir
git clone https://github.com/8devices/Caraboot.git
```

Configure:
Open Caraboot Makefile and change CONFIG_TOOLCHAIN_PREFIX to your buildroot binary path, i.e ```CONFIG_TOOLCHAIN_PREFIX=your_work_dir/buildroot/output/host/usr/bin/mips-linux- ```
```
cd Caraboot
vi/nano/gedit Makefile
```

Build:
```
make BOARD_TYPE=carambola2
```
or
```
make BOARD_TYPE=lima
```

The bootloader binary will be saved to ```bin/carambola2_u-boot.bin``` / ```bin/lima_u-boot.bin``` file.
You can now use this file to upgrade your bootloader on Carambola2 (http://8devices.com/wiki/carambola2:uboot) or Lima (http://8devices.com/wiki/lima:uboot).


