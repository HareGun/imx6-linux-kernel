#!/bin/bash

PWD=$(pwd)
START_DATE=$(date)

# Path
KERNEL_DIR=linux-imx-rel_imx_4.9.x_1.0.0_ga
OUTPUT_DIR=output
GEN_DIR=gen
export INSTALL_MOD_PATH=$PWD/$GEN_DIR/mods

# Config
KERNEL_VERSION=4.9.11
DEFCONFIG=imx_v7_ebf6ull_mini_defconfig
DTB_FILE=imx6ull-14x14-ebf6ull-mini.dtb

print_date(){
  echo "---------------------------------------------------------------------"
  echo "start date:  $START_DATE"
  echo "end date:    $(date)"
  echo "---------------------------------------------------------------------"
}

print_help(){
  echo "Usage:  ./build.sh <opt>"
  echo "opt:  image"
  echo "      dtbs"
  echo "      modules"
  echo "      menuconfig"
  echo "      clean"
  echo "      cleanall"
}

build_dtbs(){
  echo "-----------------build dtbs---------------------"
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR dtbs
	rm $PWD/$GEN_DIR/*.dtb 2>/dev/null
	cp $PWD/$OUTPUT_DIR/arch/$ARCH/boot/dts/$DTB_FILE $PWD/$GEN_DIR 2>/dev/null
	sync
}

build_Image(){
  echo "-----------------build image--------------------"
  make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR zImage -j8
  cp $PWD/$OUTPUT_DIR/arch/$ARCH/boot/zImage $PWD/$GEN_DIR
  sync
}

build_modules(){
  echo "-----------------build modules------------------"
  if [ ! -d $INSTALL_MOD_PATH ] ;then
		mkdir $INSTALL_MOD_PATH
	fi
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR modules
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR modules_install
  rm $INSTALL_MOD_PATH/lib/modules/$KERNEL_VERSION/build
  rm $INSTALL_MOD_PATH/lib/modules/$KERNEL_VERSION/source
	sync
}

build_menuconfig(){
  echo "-----------------build menuconfig---------------"
  make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR menuconfig
	sync
}

build_clean(){
  echo "-----------------build clean--------------------"
  make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR clean
	sync
}

if [ ! -d $PWD/$GEN_DIR ] ;then
	mkdir $GEN_DIR
fi

if [ "$1" == "reconfig" ] ;then
	make -C $KERNEL_DIR O=$PWD/$OUTPUT_DIR $DEFCONFIG
  sync
	exit 0
fi

if [ "$1" == "image" ] ;then
	build_Image
  print_date
	exit 0
fi

if [ "$1" == "dtbs" ] ;then
  build_dtbs
  print_date
	exit 0
fi

if [ "$1" == "modules" ] ;then
	build_modules
  print_date
	exit 0
fi

if [ "$1" == "menuconfig" ] ;then
	build_menuconfig
	exit 0
fi

if [ "$1" == "clean" ] ;then
  read -p "input [y] to clean... "
  if [ "$REPLY" == "y" ] ;then
    build_clean
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

echo "-----------------build all----------------------"
#else, build all
build_dtbs
build_Image
build_modules
print_date



