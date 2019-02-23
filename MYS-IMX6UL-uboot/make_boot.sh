#!/bin/bash 
source /opt/myir-imx-fb/4.1.15-2.0.1/environment-setup-cortexa7hf-neon-poky-linux-gnueabi
make distclean
make mys_imx6ull_14x14_emmc_defconfig
#make mys_imx6ull_14x14_nand_defconfig
make