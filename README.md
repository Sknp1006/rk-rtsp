# rk-rtsp
> 这个项目是基于RK芯片平台用于RTSP拉流&解码的组件

## 环境
- 硬件：Friendly NanoPi-R6S
- 固件：rk3588-usb-debian-bullseye-minimal-6.1-arm64-20240131

## 第三方依赖

- 自带的jellyfin为 `4.3.4-1rockchip` 版本，实测可用
- jellyfin-ffmpeg ([v6.0.1-3](https://github.com/jellyfin/jellyfin-ffmpeg/releases/tag/v6.0.1-3))
```shell
./configure --enable-rkmpp --enable-version3 --enable-libdrm --enable-static --disable-shared --enable-pthreads --enable-zlib --disable-doc --disable-debug --disable-lzma --disable-vaapi --disable-vdpau --enable-shared

make -j8
sudo make install
```
默认安装路径为`/usr/local/lib`

- opencv (自带4.5.1)
- SDL2 (自带2.0.14,缺少头文件)
```shell
sudo apt install libsdl2-dev
```

- spdlog (通过apt安装 1.8.1)
```shell
sudo apt install libspdlog-dev
```

- rockchip_mpp (自带1.3.8)
- libdrm (自带2.4.104)
- librga (自带1.9.0)
```bash
> strings /usr/lib/aarch64-linux-gnu/librga.so |grep rga_api |grep version
rga_api version 1.9.2_[0]
```

## test示例
### 编译示例
> 执行编译脚本，默认编译所有模块
```bash
./build.sh
```
### 编辑启动脚本
> 在 `install/bin` 目录下创建 run.sh 脚本，内容如下:
```bash
#!/bin/bash
set -e

# 获取绝对路径
ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${ROOT_PWD}/../lib
cd ${ROOT_PWD}

./test $1
```

### 执行启动脚本
> 需要传入一个RTSP地址作为参数
```bash
> cd install/bin
> ./run.sh "rtsp://your_rtsp_url:port/stream"
```

### 效果演示
![example](./test/example.gif) 

## 致谢

<div align="center">
<image src="https://resources.jetbrains.com/storage/products/company/brand/logos/jb_beam.svg" />
<div>
感谢 <a href=https://jb.gg/OpenSourceSupport>JetBrains</a> 为本项目提供的大力支持
</div>
</div>
