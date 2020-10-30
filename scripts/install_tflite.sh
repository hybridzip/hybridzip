#!/usr/bin/bash

pacman -Sy

pacman -S git python3 python-pip which gcc clang libglvnd mesa --noconfirm

pip install numpy

mkdir /build-space-tflite && cd /build-space-tflite

git clone https://github.com/tensorflow/tensorflow

cd tensorflow

cd "/usr/bin" && curl -fLO https://releases.bazel.build/3.1.0/release/bazel-3.1.0-linux-x86_64 && chmod +x bazel-3.1.0-linux-x86_64

cd /build-space-tflite/tensorflow

sh ./tensorflow/lite/tools/make/download_dependencies.sh

bazel build -c opt tensorflow/lite:libtensorflowlite.so

bazel build -c opt --copt -DMESA_EGL_NO_X11_HEADERS --copt -DEGL_NO_X11 tensorflow/lite/delegates/gpu:libtensorflowlite_gpu_delegate.so

cp bazel-bin/tensorflow/lite/libtensorflowlite.so "$1/libtensorflowlite.so"

cp bazel-bin/tensorflow/lite/delegates/gpu/libtensorflowlite_gpu_delegate.so "$1/libtensorflowlite_gpu_delegate.so"

#Cleanup builds
cd / && rm -rf build-space
