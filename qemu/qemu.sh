#qemu-system-arm -M vexpress-a9 -m 512M -kernel /home/eric/Works/x1000/TEST/qemu/linux-3.16.4/arch/arm/boot/zImage -dtb  /home/eric/Works/x1000/TEST/qemu/linux-3.16.4/arch/arm/boot/dts/vexpress-v2p-ca9.dtb -nographic -append "console=ttyAMA0"


qemu-system-arm -M vexpress-a9 -m 512M -kernel /home/eric/Works/x1000/TEST/qemu/linux-2.6.39/arch/arm/boot/zImage  -nographic -append "root=/dev/ram0  rdinit=/linuxrc  console=ttyAMA0" 
#qemu-system-arm -M vexpress-a9 -m 512M -kernel /home/eric/Works/x1000/TEST/qemu/linux-3.16.4/arch/arm/boot/zImage -nographic -append "root=/dev/mmcblk0  console=ttyAMA0" -sd a9rootfs.ext3
