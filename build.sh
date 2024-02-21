#!/bin/bash
set -e

TARGET_SOC="RK3588"

# 获取绝对路径
ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

# 创建build目录
BUILD_DIR=${ROOT_PWD}/build/build_linux_aarch64
if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake -B . -S ${ROOT_PWD} \
    -DCMAKE_BUILD_TYPE=Release \
    -DTARGET_SOC=${TARGET_SOC} \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
make -j4
make install
