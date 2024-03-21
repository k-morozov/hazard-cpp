#!/usr/bin/env bash

VERSION_CLANG=16
CLANG=clang++-$VERSION_CLANG
BUILD_TYPE=Release
CONAN_VERSION=1.59.0
BUILD_DIR=./cmake-build-release

sudo apt-get update
sudo apt-get install -y $CLANG pip python3-venv

sudo ln -sf /usr/bin/$CLANG /usr/bin/c++
sudo ln -sf /usr/bin/$CLANG /usr/bin/clang++

#/usr/bin/c++ --version
#/usr/bin/clang++ --version
#/usr/bin/clang++-18 --version

python3 -m venv venv
source venv/bin/activate

pip install conan==$CONAN_VERSION

conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
conan profile update settings.compiler=clang default
conan profile update settings.compiler.version=$VERSION_CLANG default
conan profile update settings.build_type=$BUILD_TYPE default
conan install --build missing -if $BUILD_DIR .
conan profile show default