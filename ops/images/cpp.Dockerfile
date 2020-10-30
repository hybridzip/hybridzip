# Builder
FROM archlinux:latest AS build

RUN pacman -Sy

RUN pacman -S cmake clang make --noconfirm

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN ./scripts/package.sh

# Runner
FROM archlinux:latest

ENV HZIP_API_THREADS=4

ENV HZIP_API_PORT=1729

ENV HZIP_PROCESSOR_THREADS=4

ENV HZIP_API_TIMEOUT=120

ENV HZIP_API_KEY=hybridzip

RUN mkdir /hybridzip

COPY --from=build /app/package.tar.gz /hybridzip

WORKDIR /hybridzip

RUN tar -xzvf package.tar.gz

RUN sh package/install.sh

CMD ["hybridzip"]