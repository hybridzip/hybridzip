#!/usr/bin/bash

set -e
# Install all dependencies
echo "Installing dependencies for hybridzip ..."

rm -rf /usr/lib/hzip
mkdir /usr/lib/hzip

# Install libraries at /usr/lib/hzip

# Compile tests

echo "Compiling tests ..."

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release . && make gtest_run

# Download test data
rm -rf /test-space

mkdir /test-space

echo "Running GTest"

./bin/gtest_run