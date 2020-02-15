# Builder
FROM alpine:latest AS build

RUN apk update && apk add make cmake gcc g++ boost-dev

RUN mkdir /app

COPY . /app

WORKDIR /app

RUN cmake -DDISABLE_PROFILER=ON . && make

# Runner
FROM alpine:latest

COPY --from=build /app/hybridzip ./

#Install libraries that have runtime relevance.
RUN apk update && apk add libstdc++ boost libpthread-stubs

#Run hybridzip
RUN ./hybridzip
