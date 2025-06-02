#!/bin/bash

mkdir -p ./output

brew install --quiet pkg-config nasm automake libtool cmake

brew_install() {
    brew list $1 &>/dev/null || brew install --quiet $1
}
brew install glew glfw
brew_install freetype
brew_install fontconfig
brew_install fribidi
brew_install webp
brew_install libvpx
brew_install opus
brew_install libaom
brew_install libass
brew_install lame
brew_install fdk-aac
brew_install x264
brew_install x265
brew_install libxml2

export PKG_CONFIG_PATH="./output/lib/pkgconfig:$PKG_CONFIG_PATH"

if [ ! -d "x264" ]; then
    git clone https://code.videolan.org/videolan/x264.git
    cd x264
    ./configure --prefix=$(pwd)/../output --enable-static --enable-shared
    make -j$(sysctl -n hw.logicalcpu)
    make install
    cd ..
fi

