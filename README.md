# rk-rtsp
> 这个项目是基于RK芯片平台用于RTSP拉流&解码的组件

## 环境
- 硬件：Friendly NanoPi-R6S
- 固件：rk3588-usb-debian-bullseye-minimal-6.1-arm64-20240131

## 第三方依赖

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

## 使用
> 注意加双引号""，否则会被shell解释为管道符
```shell
./test "rtsp://xxxxxxxx"
```