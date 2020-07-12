# Builder
FROM ubuntu:latest AS build

RUN apt-get update && apt-get install -y cmake gcc g++

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN cmake -DCMAKE_BUILD_TYPE=Release . && make

# Runner
FROM ubuntu:latest

COPY --from=build /app/bin/hybridzip ./

#Run hybridzip
RUN ./hybridzip
