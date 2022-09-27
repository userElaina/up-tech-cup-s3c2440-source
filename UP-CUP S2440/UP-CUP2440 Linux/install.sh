#!/bin/sh

#***********************************************************************
#  install armv4l-linux develop environment
#
#  Create  by Zhengguang Qian   <qzg_uptech@126.com>
#	 Modify  by sprife <tdd_uptech@126.com> 
#  Date: 2009-6-24
#************************************************************************

PREFIX=uptech
TARGETDIR=/UP-CUP2440/SRC
VERSION=V1.0

clear
echo
echo
echo "    ***********************************************************"
echo "    UP-CUP2440 linux develop environment version 1.0"
echo "                           2011-04-24"
echo "    ***********************************************************"
echo
echo 


# make needed directory
if [ -f $TARGETDIR/img/zImage ] ; then 
	echo "The target dir is already exist! " 
	echo "Please backup your files or change to another directory"
	exit
fi
mkdir /UP-CUP2440
mkdir $TARGETDIR 2>/dev/null

inst_arm_gcc(){
	echo "install arm-linux-gcc and libraray ..."

	tar -xjf arm-linux-gcc-3.4.6-glibc-2.3.6.tar.bz2  -C /usr/local/
	
}

inst_uboot(){
	echo "install u-boot..."
	mkdir $TARGETDIR/u-boot 2>/dev/null
	cp -arf ./*   $TARGETDIR/u-boot  || exit
	tar -xjf arm-linux-gcc-4.3.2.tar.bz2 -C /usr/local/

}
inst_linux(){
	echo "install linux-2.6.24..."
	mkdir $TARGETDIR/kernel 2>/dev/null
	tar -xjf linux-2.6.24.4.tar.bz2  -C $TARGETDIR/kernel  || exit 

}

inst_rootfs(){
	echo "install busybox-1.12.2..."
	mkdir $TARGETDIR/rootfs 2>/dev/null
	tar -xjf busybox-1.12.2.tar.bz2 -C $TARGETDIR/rootfs || exit 
	echo "install rootfs..."
	tar -xjf rootfs.tar.bz2 -C $TARGETDIR/rootfs || exit 
}


inst_exp(){
	echo "install examples programs..."
	mkdir $TARGETDIR/exp 2>/dev/null
	cp -arf ./*   $TARGETDIR/exp  || exit
}

inst_gui(){
	echo "install gui programs"
	mkdir $TARGETDIR/gui 2>/dev/null
	cp -arf ./*		$TARGETDIR/gui  || exit
}

#............................
cd CrossTools || exit
inst_arm_gcc
cd ..

cd SRC/kernel || exit
inst_linux
cd ../../

cd SRC/rootfs || exit
inst_rootfs
cd ../../

cd SRC/exp || exit
inst_exp
cd ../../

cd SRC/gui || exit
inst_gui
cd ../../

cd SRC/u-boot || exit
inst_uboot
cd ../../


echo copy image files......
cp -arf IMG  /UP-CUP2440 

echo unpack demos....
mkdir /UP-CUP2440/demos 2>/dev/null
cd IMG
tar jxf run_exp.tar.bz2 -C /UP-CUP2440/demos/ 2>/dev/null
cd ../../


echo 'export PATH=/usr/local/arm/gcc-3.4.6-glibc-2.3.6/arm-linux/bin/:$PATH' >>~/.bash_profile
echo '#export PATH=/usr/local/arm/4.3.2/bin/:$PATH' >>~/.bash_profile
source ~/.bash_profile 
cd /UP-CUP2440 
mkdir /UP-CUP2440/SRC/bin
echo
echo
echo arm-linux develop enviroment for UP-CUP2440  install  completed!
echo please login in again!!
echo
echo
