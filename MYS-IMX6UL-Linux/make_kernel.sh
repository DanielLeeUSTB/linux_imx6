#!/bin/bash 
source /opt/myir-imx-fb/4.1.15-2.0.1/environment-setup-cortexa7hf-neon-poky-linux-gnueabi
make distclean
make mys_imx6_defconfig
make zImage dtbs moudles