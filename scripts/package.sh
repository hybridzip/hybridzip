#!/usr/bin/bash

set -e
# Install all dependencies
echo "Installing dependencies for hybridzip ..."

rm -rf /usr/lib/hzip
mkdir /usr/lib/hzip

# Install libraries at /usr/lib/hzip

if [ "$#" -gt 0 ]; then
  case "$1" in
    -b|--build)
    echo "Installing tensorflow-lite ..."
    sh "$(dirname "$0")/install_tflite.sh" "/usr/lib/hzip"
  esac
  else
    echo "Installing tensorflow-lite ..."
    sh "$(dirname "$0")/download_tflite.sh" "/usr/lib/hzip"
fi


# Compile hybridzip

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release . && make hybridzip


# Package all dependencies ...
echo "Packaging all dependencies ..."

cp /usr/lib/hzip/libtensorflowlite.so ./lib/libtensorflowlite.so
cp /usr/lib/hzip/libtensorflowlite_gpu_delegate.so ./lib/libtensorflowlite_gpu_delegate.so

# Compress package
echo "Compressing package ..."

mkdir package
mv ./bin ./package/bin
mv ./lib ./package/lib
cp ./scripts/install.sh ./package/install.sh

tar -zcvf package.tar.gz package

echo "Completed packaging successfully ..."

