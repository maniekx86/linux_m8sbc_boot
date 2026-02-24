# linux_m8sbc_boot

x86 Linux 2.2 bootloader designed for booting Linux on the [M8SBC-486](https://maniek86.xyz/projects/m8sbc_486.php) homebrew computer.

## Notes:
- Makefile creates directly ready to flash image, so it requires zImage in the main directory of this repository to properly finish. Result image is split into two partitions: boot (FAT32) and rootfs (EXT2). You need to provide filesystem on the rootfs manually by mounting the image if your kernel needs it. Kernel parameters can be changed in the boot/main.c.
- Memory map and various parameters are fixed (these settings are specifically set for the M8SBC-486). See boot/main.c
- See Makefile for more configuration options

## Acknowledgements

- [FAT32 Library by strawberryhacker](https://github.com/strawberryhacker/fat32)
- [FAT32 bootloader by alexfru](https://github.com/alexfru/BootProg)
