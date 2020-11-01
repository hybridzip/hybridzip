# Builder
FROM archlinux:20200908 AS build

RUN pacman -Sy

RUN pacman -S cmake clang make mesa libglvnd --noconfirm

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN ./scripts/package.sh

# Runner
FROM archlinux:20200908

ENV HZIP_API_THREADS=4

ENV HZIP_API_PORT=1729

ENV HZIP_PROCESSOR_THREADS=4

ENV HZIP_API_TIMEOUT=120

ENV HZIP_API_KEY=hybridzip

ENV HZIP_MAX_MEM_USAGE=1073741824

RUN mkdir /hybridzip

COPY --from=build /app/package.tar.gz /hybridzip

WORKDIR /hybridzip

RUN tar -xzvf package.tar.gz

RUN sh package/install.sh

CMD ["hybridzip"]