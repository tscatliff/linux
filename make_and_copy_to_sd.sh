#!/bin/sh -x
make -j4 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- zImage modules dtbs
result=$?
echo "Result: $result"
if [ $result -ne 0 ]
then
  exit 1
fi
#sudo mount /dev/sdb1 /media/pi/boot
#sudo mount /dev/sdb2 /media/pi/rootfs
sudo env PATH=$PATH make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=/media/pi/rootfs modules_install
sudo cp /media/pi/boot/kernel.img /media/pi/boot/kernel-backup.img
sudo cp arch/arm/boot/zImage /media/pi/boot/kernel.img
sudo cp arch/arm/boot/dts/*.dtb /media/pi/boot/
sudo cp arch/arm/boot/dts/overlays/*.dtb* /media/pi/boot/overlays/
sudo cp arch/arm/boot/dts/overlays/README /media/pi/boot/overlays/
#sudo umount /media/pi/boot
#sudo umount /media/pi/rootfs

