#
# This Makefile creates some pre-built glue code that is kept (in binary form)
# in the Halide repo; this is intended to allow end users to compile for
# Hexagon targets without requiring the Hexagon SDK to be present on the build
# system. (The Hexagon SDK is of course required to build the glue code itself.)
# At present, only Linux systems are supported for building and testing; this
# could probably be made to work on Windows (via MinGW) with a little effort.
#
# In order to build this, you'll need to ensure that the following
# env vars are set correctly:
#
# HEXAGON_SDK_ROOT   : path to Qualcom Hexagon SDK
# HEXAGON_TOOLS_ROOT : path to Qualcomm Hexagon Tools
# HEXAGON_QAIC       : path to Qualcomm qaic compiler
#
# Note that all build products are put in a subdirectory of a local bin/
# directory (even machine-generated C++ files), to make clear which files
# are "real" source vs machine-generated. This is a rare case in which
# a bin/ folder is meant to be kept in source control.
#
# Note that -- despite the efforts above -- the Hexagon SDK *is* required
# in order to *use* the simulator (which requires proprietary binary blobs
# from the SDK that cannot be redistributed with Halide).
#

HEXAGON_SDK_ROOT ?= ${HOME}/Qualcomm/Hexagon_SDK/3.0
HEXAGON_TOOLS_ROOT ?= ${HOME}/Qualcomm/HEXAGON_Tools/8.0/Tools

ANDROID_NDK_ROOT ?= ${HEXAGON_SDK_ROOT}/tools/android-ndk-r10d

# QAIC compiler may vary depending on your build env.
HEXAGON_QAIC ?= ${HEXAGON_SDK_ROOT}/tools/qaic/Ubuntu14/qaic

# Some SDK versions use "inc/", some use "incs/"
HEXAGON_SDK_INCLUDES ?= "${HEXAGON_SDK_ROOT}/incs"

# Some SDK versions use "lib/", some use "libs/"
HEXAGON_SDK_LIBS ?= "${HEXAGON_SDK_ROOT}/libs"

COMMON_FLAGS = -I ${HEXAGON_SDK_INCLUDES}/stddef -I../
COMMON_CCFLAGS = ${COMMON_FLAGS} -O3 -I ${HEXAGON_SDK_LIBS}/common/remote/ship/android_Release
HEXAGON_QAICFLAGS = ${COMMON_FLAGS}

# rpcmem doesn't have a precompiled library in the SDK, so we just
# include its source directly.
# TODO: There must be a better way to do this, without requiring users
# to go to the SDK and build rpcmem with the SDK makefiles
# (non-trivial effort).
COMMON_CCFLAGS += \
	-I ${HEXAGON_SDK_LIBS}/common/rpcmem/inc \
	-I ${HEXAGON_SDK_INCLUDES}/qlist \
	-I ${HEXAGON_SDK_INCLUDES}/a1std
RPCMEM_C = ${HEXAGON_SDK_LIBS}/common/rpcmem/src/rpcmem_android.c

CC-host = ${CXX}
CXX-host = ${CXX}

CC-v60 = ${HEXAGON_TOOLS_ROOT}/bin/hexagon-clang
CXX-v60 = ${HEXAGON_TOOLS_ROOT}/bin/hexagon-clang++
LD-v60 = $(HEXAGON_TOOLS_ROOT)/bin/hexagon-link

CC-arm-64-android = ${ANDROID_NDK_ROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-gcc
CXX-arm-64-android = ${ANDROID_NDK_ROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-g++
CCFLAGS-arm-64-android = --sysroot ${ANDROID_NDK_ROOT}/platforms/android-21/arch-arm64

CC-arm-32-android = ${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc
CXX-arm-32-android = ${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++
CCFLAGS-arm-32-android = --sysroot ${ANDROID_NDK_ROOT}/platforms/android-21/arch-arm

CCFLAGS-host := ${CCFLAGS} -I ${HEXAGON_TOOLS_ROOT}/include/iss/ -fPIC \
	-L${HEXAGON_TOOLS_ROOT}/lib/iss/ -lwrapper

CCFLAGS-v60 := $(CCFLAGS-v60) ${COMMON_CCFLAGS} -I ${HEXAGON_SDK_INCLUDES} -I ${HEXAGON_SDK_LIBS}/common/qurt/ADSPv60MP/include

CCFLAGS-arm-64-android := $(CCFLAGS-arm-64-android) ${COMMON_CCFLAGS} -llog -fPIE -pie \
	-L${HEXAGON_SDK_LIBS}/common/remote/ship/android_Release_aarch64/ -ladsprpc \
	${HEXAGON_SDK_LIBS}/common/adspmsgd/ship/android_Release_aarch64/adspmsgd.a
CCFLAGS-arm-32-android := $(CCFLAGS-arm-32-android) ${COMMON_CCFLAGS} -llog -fPIE -pie \
	-L${HEXAGON_SDK_LIBS}/common/remote/ship/android_Release/ -ladsprpc \
	${HEXAGON_SDK_LIBS}/common/adspmsgd/ship/android_Release/adspmsgd.a

.PHONY: all
all: hosts remotes

.PHONY: hosts
hosts: bin/arm-64-android/libhalide_hexagon_host.so bin/arm-32-android/libhalide_hexagon_host.so bin/host/libhalide_hexagon_host.so

.PHONY: remotes
remotes: bin/v60/libhalide_hexagon_remote_skel.so bin/v60/hexagon_sim_remote

bin/src/halide_hexagon_remote.h bin/src/halide_hexagon_remote_skel.c bin/src/halide_hexagon_remote_stub.c: halide_hexagon_remote.idl
	mkdir -p $(@D)
	$(HEXAGON_QAIC) $(HEXAGON_QAICFLAGS) $^ -o $(@D)

bin/%/halide_hexagon_remote_skel.o: bin/src/halide_hexagon_remote_skel.c
	mkdir -p $(@D)
	$(CC-$*) $(CCFLAGS-$*) -fPIC -c $^ -o $@

bin/%/halide_remote.o: halide_remote.cpp
	mkdir -p $(@D)
	$(CXX-$*) $(CCFLAGS-$*) -fPIC -c halide_remote.cpp -o $@

bin/%/halide_host.o: halide_host.cpp
	mkdir -p $(@D)
	$(CXX-$*) $(CCFLAGS-$*) -fPIC -c halide_host.cpp -o $@

# Build rules for the hexagon implementation.
bin/%/libhalide_hexagon_remote_skel.so: bin/%/halide_remote.o bin/%/halide_hexagon_remote_skel.o
	mkdir -p $(@D)
	$(CC-$*) -m$* -O0 -mG0lib -G0 -fpic -shared -lc -o $@ \
	    -Wl,-Bsymbolic -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=free \
	    -Wl,--wrap=realloc -Wl,--wrap=memalign -Wl,--wrap=__stack_chk_fail \
	    -Wl,-soname=libhalide_hexagon_remote_skel.so \
	    -Wl,--start-group  bin/$*/halide_remote.o bin/$*/halide_hexagon_remote_skel.o -Wl,--end-group

bin/%/libhalide_hexagon_host.so: bin/src/halide_hexagon_remote_stub.c bin/%/halide_host.o
	mkdir -p $(@D)
	$(CC-$*) $(CCFLAGS-$*) -Wl,-soname,libhalide_hexagon_host.so $^ ${RPCMEM_C} -shared -o $@

# Build rules for the simulator implementation.
bin/%/sim_remote.o: sim_remote.cpp sim_protocol.h elf.h
	mkdir -p $(@D)
	$(CXX-$*) $(CCFLAGS-$*) -DSIMULATOR -c sim_remote.cpp -o $@

bin/%/sim_host.o: sim_host.cpp sim_protocol.h
	mkdir -p $(@D)
	$(CXX-$*) -std=c++11 $(CCFLAGS-$*) -c sim_host.cpp -o $@


CRT0_STANDALONE=$(shell $(CXX-v60) -G0 -print-file-name=crt0_standalone.o)
CRT0           =$(shell $(CXX-v60) -G0 -print-file-name=crt0.o)
INIT           =$(shell $(CXX-v60) -G0 -print-file-name=init.o)
LIB_STANDALONE =$(shell $(CXX-v60) -G0 -print-file-name=libstandalone.a)
LIB_C          =$(shell $(CXX-v60) -G0 -print-file-name=libc.a)
LIB_GCC        =$(shell $(CXX-v60) -G0 -print-file-name=libgcc.a)
FINI           =$(shell $(CXX-v60) -G0 -print-file-name=fini.o)
LIBDL          =$(HEXAGON_TOOLS_ROOT)/target/hexagon/lib/v60/G0/libdl.a

bin/%/hexagon_sim_remote: bin/%/sim_remote.o
	mkdir -p $(@D)
	$(LD-$*) -o $@ $(CRT0_STANDALONE) $(CRT0) $(INIT) bin/$*/sim_remote.o $(LIBDL) \
	    --start-group  $(LIB_STANDALONE) --whole-archive $(LIB_C) --no-whole-archive $(LIB_GCC) --end-group $(FINI) \
	    --dynamic-linker= -E --force-dynamic

bin/host/libhalide_hexagon_host.so: bin/host/sim_host.o
	mkdir -p bin/host
	$(CC-host) $^ $(CCFLAGS-host) -Wl,-soname,libhalide_hexagon_host.so -shared -o $@

.PHONY: clean
clean:
	rm -rf bin/
