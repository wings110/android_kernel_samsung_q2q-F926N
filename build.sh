#!/bin/bash

# 设置编译环境
export ARCH=arm64
ln -s /usr/bin/python2.7 $HOME/python
export PATH=$HOME/clang/bin:$PATH # 设置 Proton 的路径
mkdir -p out
clear

# 通用编译参数
export ARGS="
CC=clang
LD=ld.lld
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
CROSS_COMPILE_ARM32=arm-linux-gnueabi-
CLANG_TRIPLE=aarch64-linux-gnu-
AR=llvm-ar
NM=llvm-nm
AS=llvm-as
READELF=llvm-readelf
OBJCOPY=llvm-objcopy
OBJDUMP=llvm-objdump
OBJSIZE=llvm-size
STRIP=llvm-strip
LLVM_AR=llvm-ar
LLVM_DIS=llvm-dis
LLVM_NM=llvm-nm
LLVM=1
"

# 清理和初始化
make -j$(nproc) -C $(pwd) O=$(pwd)/out ${ARGS} clean
make -j$(nproc) -C $(pwd) O=$(pwd)/out ${ARGS} mrproper

# 配置内核
make -j$(nproc) -C $(pwd) O=$(pwd)/out ${ARGS} vendor/q2q_kor_singlex_defconfig

# 手动配置（可选）
make -j$(nproc) -C $(pwd) O=$(pwd)/out ${ARGS} menuconfig

# 编译内核，添加自定义内核名称
make -j$(nproc) -C $(pwd) O=$(pwd)/out ${ARGS} LOCALVERSION="-wings110-lxc" CONFIG_LOCALVERSION_AUTO=n

# 安装内核模块，并启用 INSTALL_MOD_STRIP=1
make -j$(nproc) -C $(pwd) O=$(pwd)/out ${ARGS} INSTALL_MOD_STRIP=1 modules_install INSTALL_MOD_PATH=$(pwd)/out/modules
echo "Stripped kernel modules installed in the 'out/modules' folder."

# 将所有内核模块（.ko）复制到 "modules" 文件夹
cd out/modules
mkdir -p stripped_modules
find . -type f -name "*.ko" -exec cp -n {} stripped_modules/ \;
echo "Stripped module files copied to the 'stripped_modules' folder."

