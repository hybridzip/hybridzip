#!/usr/bin/bash

set -e
# Install all dependencies
echo "Installing dependencies for hybridzip ..."

rm -rf /usr/lib/hzip
mkdir /usr/lib/hzip

# Install libraries at /usr/lib/hzip

# Compile hybridzip

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release . && make hybridzip


# Package all dependencies ...
echo "Packaging all dependencies ..."

# Compress package
echo "Compressing package ..."

mkdir package
mv ./bin ./package/bin
mv ./lib ./package/lib
cp ./scripts/install.sh ./package/install.sh

tar -zcvf package.tar.gz package

echo "Completed packaging successfully ..."

