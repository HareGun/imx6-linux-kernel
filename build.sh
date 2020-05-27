#!/bin/bash

PWD=$(pwd)
START_DATE=$(date)

# Path
KERNEL_DIR=linux-imx-rel_imx_4.9.x_1.0.0_ga
OUTPUT_DIR=output
GEN_DIR=gen
export INSTALL_MOD_PATH=$PWD/$GEN_DIR/mods

# Config
DEFCONFIG=imx_v7_mfg_defconfig
DTB_FILE=imx6ull-14x14-evk.dtb

print_date(){
  echo "---------------------------------------------------------------------"
  echo "start date:  $START_DATE"
  echo "end date:    $(date)"
  echo "---------------------------------------------------------------------"
}

if [ ! -d $PWD/$GEN_DIR ] ;then
	mkdir $GEN_DIR
fi

if [ "$1" == "reconfig" ] ;then
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR $DEFCONFIG
  sync
	exit 0
fi

if [ "$1" == "dtbs" ] ;then
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR dtbs
	rm $PWD/$GEN_DIR/*.dtb 2>/dev/null
	cp $PWD/$OUTPUT_DIR/arch/$ARCH/boot/dts/$DTB_FILE $PWD/$GEN_DIR 2>/dev/null
	sync
	exit 0
fi

if [ "$1" == "module" ] ;then
	if [ ! -d $INSTALL_MOD_PATH ] ;then
		mkdir $INSTALL_MOD_PATH
	fi
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR modules
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR modules_install
  print_date
	sync
	exit 0
fi

if [ "$1" == "menuconfig" ] ;then
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR menuconfig
	sync
	exit 0
fi

if [ "$1" == "clean" ] ;then
  read -p "input [y] to clean output... "
  if [ "$REPLY" == "y" ] ;then
    rm -rf $PWD/$OUTPUT_DIR
    echo "clean output done"
  fi	
	sync
	exit 0
fi

if [ "$1" == "cleanall" ] ;then
  read -p "input [y] to clean all... "
  if [ "$REPLY" == "y" ] ;then
    rm -rf $PWD/$OUTPUT_DIR $PWD/$GEN_DIR
    echo "clean all done"
  fi	
	sync
	exit 0
fi

make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR zImage -j8
cp $PWD/$OUTPUT_DIR/arch/$ARCH/boot/zImage $PWD/$GEN_DIR
sync
print_date



