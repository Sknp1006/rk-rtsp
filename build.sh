#!/bin/bash
set -e

TARGET_SOC="RK3588"
BUILD_TYPE=${1:-"Release"}

# 获取绝对路径
ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

# 创建build目录
BUILD_DIR=${ROOT_PWD}/build/build_linux_aarch64
if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake -B . -S ${ROOT_PWD} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DTARGET_SOC=${TARGET_SOC} \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_MAKE_PROGRAM=ninja \
    -GNinja

ninja -j6
ninja install
