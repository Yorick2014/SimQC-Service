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
