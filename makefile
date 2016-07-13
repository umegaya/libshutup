.PHONY: build
build:
	- mkdir build
	cd build && cmake .. && make

ios:
	- mkdir build.ios
	cd build.ios && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/ios.cmake .. && make

android:
	- mkdir build.android
	cd build.android && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/android.cmake .. && make	

clean:
	rm -rf build build.android build.ios

all: build android ios

