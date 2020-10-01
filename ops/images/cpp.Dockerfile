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

COPY --from=build /app/bin/hybridzip ./

#Run hybridzip
RUN ./hybridzip