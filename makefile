BUILD_DIR ?= build-firmware
TOOLCHAIN ?= cmake/gcc-arm-none-eabi.cmake
BUILD_TYPE ?= Release

all: build

configure:
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) -j

flash: build
	cmake --build $(BUILD_DIR) --target flash

dfu: build
	cmake --build $(BUILD_DIR) --target dfu

clean:
	cmake --build $(BUILD_DIR) --target clean

.PHONY: all configure build flash dfu clean
