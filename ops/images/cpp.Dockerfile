# Builder
FROM archlinux:latest AS build

RUN pacman -Syu --noconfirm

RUN pacman -S cmake clang make --noconfirm

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release . && make hybridzip

# Runner
FROM archlinux:latest

ENV HZIP_API_THREADS=4

ENV HZIP_API_PORT=1729

ENV HZIP_PROCESSOR_THREADS=4

ENV HZIP_API_TIMEOUT=120

ENV HZIP_API_KEY=hybridzip

RUN mkdir /hybridzip

COPY --from=build /app/bin/hybridzip /hybridzip

WORKDIR /hybridzip

CMD ["./hybridzip"]