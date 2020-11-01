# Tester
FROM archlinux:20200908 AS build

RUN pacman -Sy

RUN pacman -S cmake clang make mesa libglvnd --noconfirm

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN ./scripts/test.sh