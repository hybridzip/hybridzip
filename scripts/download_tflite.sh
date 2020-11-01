#!/usr/bin/bash

set -e

pacman -S wget --noconfirm

wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=11jb46TAh3ZsRn2KdedyZj_yqvairPGpb' -O "$1/libtensorflowlite.so"

wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=145QXEb7wd1wKqxmkWSJyS1_r0ejxuWQT' -O "$1/libtensorflowlite_gpu_delegate.so"