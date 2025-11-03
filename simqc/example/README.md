Для того, чтобы запустить example можно использовать CMake и Docker.

Пример CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.14)
project(QKD_Simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(qkd_lib
    src/bb84.cpp
    src/laser.cpp
    src/sequence_generator.cpp
    src/modulator.cpp
)

target_include_directories(qkd_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(simple-bb84 example/simple-bb84.cpp)
target_link_libraries(simple-bb84 PRIVATE qkd_lib)
```

Пример Dockerfile:
```
FROM gcc:latest AS build

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    git \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build && \
    cmake --build build --config Release

FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd -r qkduser && useradd -r -g qkduser qkduser
USER qkduser

WORKDIR /app

COPY --from=build /app/build/simple-bb84 .
COPY cfg ./cfg

ENTRYPOINT ["./simple-bb84"]

```
