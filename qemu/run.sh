#!/bin/bash
QEMU_PATH=/home/eric/Works/x1000/TEST/ingenic/prebuilts/mips-qemu/bin #QEMU 的安装路径
QEMU=qemu-system-mipsel rootfs=/home/eric/Works/x1000/TEST/ingenic/out/product/halley2/system #要启动的文件系统
kernel=/home/eric/Works/x1000/TEST/ingenic/kernel/vmlinux #要启动的 kernel
echo "------------------------------ Xburst1 X1000 is booting $1 rootfs----------------------------"
$QEMU_PATH/${QEMU} \
-M phoenix -cpu xburst1-x1000 \
-kernel ${kernel} \
-append "console=ttyS1,57600n8 rdinit=/linuxrc root=/dev/ram0 rw
mem=256M@0x0mem=768M@0x30000000" \
-initrd ${rootfs} \
-serial /dev/tty -serial /dev/tty \
-nographic \
-net nic,model=rtl8139 \
-net user,hostfwd=tcp::2030-:22,hostfwd=tcp::2089-:80
#定义的 ssh 端口号 #定义的 http 的端口号
