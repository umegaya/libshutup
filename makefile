.PHONY: test
test: testlib
	make -C test run

debug: 
	lldb test/build/t

testlib:
	- mkdir build.testlib
	cd build.testlib && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/testlib.cmake .. && make

lib:
	- mkdir build
	cd build && cmake .. && make

bundle:
	- mkdir build.bundle
	cd build.bundle && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/bundle.cmake .. && make

ios:
	- mkdir build.ios
	cd build.ios && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/ios.cmake .. && make

android:
	- mkdir build.android
	cd build.android && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/android.cmake .. && make	

clean:
	rm -rf build build.android build.ios build.bundle build.testlib

all: lib bundle android ios

