# Builder
FROM alpine:latest AS build

RUN apk update && apk add make cmake clang gcc g++ boost-dev jpeg libexecinfo-dev

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Release . && make

# Runner
FROM alpine:latest

COPY --from=build /app/hybridzip ./

#Install libraries that have runtime relevance.
RUN apk update && apk add libstdc++ boost libpthread-stubs

#Run hybridzip
RUN ./hybridzip
