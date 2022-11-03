#!/bin/bash
# Author: changyanlong01
# Created Time: 五  2/19 14:44:12 2021
FFMPEG_GL_PATCH_PATH="./Plus-OpenGL-Patch"
cp $FFMPEG_GL_PATCH_PATH/Plus-GL-Shader/vf_fadeglshader.c ./libavfilter/
cp $FFMPEG_GL_PATCH_PATH/Plus-GL-Shader/vf_plusglshader.c ./libavfilter/
cp $FFMPEG_GL_PATCH_PATH/Plus-GL-Shader/vf_lutglshader.c ./libavfilter/
cp $FFMPEG_GL_PATCH_PATH/Plus-GL-Shader/vf_pipglshader.c ./libavfilter/

#make clean

#:<<!
./configure \
  --enable-cross-compile \
  --pkg-config-flags="--static" \
  --extra-ldflags="-lm -lz -llzma -lpthread" \
  --extra-libs=-lpthread \
  --extra-libs=-lm \
  --enable-gpl \
  --enable-libfdk_aac \
  --enable-libfreetype \
  --enable-libmp3lame \
  --enable-libopus \
  --enable-libvpx \
  --enable-encoder=libvpx_vp8 --enable-encoder=libvpx_vp9 --enable-decoder=vp8 --enable-decoder=vp9 --enable-parser=vp8 --enable-parser=vp9 \
  --enable-libx264 \
  --enable-libx265 \
  --enable-libaom \
  --enable-decoder=h264 \
  --enable-decoder=h265 \
  --enable-decoder=hevc \
  --enable-libass \
  --enable-libfreetype       \
  --enable-libfontconfig     \
  --enable-libfribidi        \
  --enable-libwebp           \
  --enable-demuxer=dash     \
  --enable-libxml2 \
  --enable-nonfree \
  --enable-shared \
  --enable-opengl \
  --enable-opencl \
  --extra-libs='-lGLEW -lglfw' \
  --enable-filter=plusglshader \
  --enable-filter=lutglshader \
  --enable-filter=fadeglshader \
  --enable-filter=pipglshader \
  --enable-filter=gltransition
  #--enable-static \
#!
#make clean
make -j16
sudo make install
