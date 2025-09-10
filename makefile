all: build

build:
	mkdir -p build
	arduino-cli compile --build-path build

dfu:
	dfu-util -a 0 -D build/dSIB-Z-Si-firmware.ino.bin -s 0x08000000


.PHONY: all build dfu
