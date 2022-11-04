# FFmpeg++ (FFmpeg-Plus-Plus) Base on 4.2.4

[中文](README_CN.md) | [English](README.md)

* FFmpeg Filter支持特效/转场/画中画、特效/LUT贴纸能力, 支持OpenGL Shader
* FFmpeg 支持转码HEVC/AV1/Opus编码 的FLV流

## 联系

* QQ Group Number: 925466059 
* Email: porschegt23@foxmail.com
* WeChat: numberwolf11 (尽量加QQ群，除了靠谱技术合作 不接受单聊)

## 0. 功能

* OpenGL(GLSL) AVFilter   
[How to use FFmpeg + OpenGL Filter/Effects/Transition?](./Plus-OpenGL-Patch/README.MD)
    * Filter 滤镜
    * Effect 特效
    * Transition 转场

* FLV
    * HEVC/H.265
    * AV1
    * Opus

## 1. 编译构建

### 1.1 FFmpeg + OpenGL + FLV(265/AV1/Opus)

* FLV HEVC CodecID = 12 `FLV_CODECID_HEVC`
* FLV AV1 CodecID = 13 `FLV_CODECID_AV1`
* FLV Opus CodecID = 13 `FLV_CODECID_OPUS`

```shell
bash build-all.sh
```

### 1.2 FFmpeg + OpenGL

```shell
bash build-only-opengl.sh
```

### 1.3 FFmpeg + FLV(265/AV1/Opus)

```shell
bash build-with-flv_265-flv_av1_opus.sh
```

## 2.使用

### 2.1 FFmpeg + OpenGL

[How to use FFmpeg + OpenGL Filter/Effects/Transition?](./Plus-OpenGL-Patch/README.MD)

### 2.2 FFmpeg + FLV(265/AV1/Opus)

#### 测试 FFmpeg H.265

* Execute Command

```shell
ffmpeg -i hevctest.flv
```

* Output
```
Input #0, flv, from 'hevctest.flv':
  Metadata:
    major_brand     : isom
    minor_version   : 512
    compatible_brands: isomiso2mp41
    encoder         : Lavf58.29.100
  Duration: 00:00:05.10, start: 0.059000, bitrate: 855 kb/s
    Stream #0:0: Video: hevc (Main), yuv420p(tv), 1280x720, 25 fps, 25 tbr, 1k tbn, 25 tbc
    Stream #0:1: Audio: aac (LC), 48000 Hz, stereo, fltp, 128 kb/s
```




#### 测试 FFmpeg Av1+Opus

* Execute Command

```shell
ffmpeg -i av1test_opus.flv
```

* Output
```
[libaom-av1 @ 0x7fdbf0814600] dimension change! 0x0 -> 1280x720
Input #0, flv, from 'av1test_opus.flv':
  Metadata:
    major_brand     : isom
    minor_version   : 512
    compatible_brands: isomiso2mp41
    encoder         : Lavf58.29.100
  Duration: 00:00:05.01, start: 0.000000, bitrate: 331 kb/s
    Stream #0:0: Video: av1 (Main), yuv420p(tv), 1280x720, 256 kb/s, 25 fps, 25 tbr, 1k tbn
    Stream #0:1: Audio: opus, 48000 Hz, stereo, fltp, 96 kb/s
[libaom-av1 @ 0x7fdbf1808000] 2.0.0
```



