cmd_u-boot.imx := ./tools/mkimage -n board/myir/mys_imx6ull/imximage.cfg.cfgtmp -T imximage -e 0x87800000 -d u-boot.bin u-boot.imx  >/dev/null
