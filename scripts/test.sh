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

# Compile tests

echo "Compiling tests ..."

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release . && make gtest_run

# Download test data
rm -rf /test-space

mkdir /test-space

echo "Downloading linear.tflite ..."

wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1DOmn7K-QM_K8mImwnTEuCLNTDU1NBkw1' -O "/test-space/linear.tflite"

echo "Running GTest"

./bin/gtest_run