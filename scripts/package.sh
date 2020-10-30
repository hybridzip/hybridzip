#!/usr/bin/bash

set -e
# Install all dependencies
echo "Installing dependencies for hybridzip ..."

echo "Installing tensorflow-lite ..."
# Install tflite libraries at /usr/lib
sh "$(dirname "$0")/install_tflite.sh" "/usr/lib"

# Compile hybridzip

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release . && make hybridzip


# Package all dependencies ...
echo "Packaging all dependencies ..."

cp /usr/lib/libtensorflowlite.so ./lib/libtensorflowlite.so
cp /usr/lib/libtensorflowlite_gpu_delegate.so ./lib/libtensorflowlite_gpu_delegate.so

# Compress package
echo "Compressing package ..."

mkdir package
mv ./bin ./package/bin
mv ./lib ./package/lib
cp ./scripts/install.sh ./package/install.sh

tar -zcvf package.tar.gz package

echo "Completed packaging successfully ..."

