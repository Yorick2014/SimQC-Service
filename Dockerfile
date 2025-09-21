FROM gcc:latest AS build

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    git \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Сборка GoogleTest вручную (libgtest-dev содержит только исходники)
WORKDIR /tmp/gtest
RUN git clone https://github.com/google/googletest.git . && \
    cmake -S . -B build && \
    cmake --build build && \
    cp build/lib/libgtest*.a /usr/lib/

WORKDIR /app
COPY . .

RUN cmake -S . -B build && \
    cmake --build build --config Release

# Этап запуска --------------------------------------------------
FROM ubuntu:latest

# Установим только нужные рантайм-библиотеки
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Создадим непривилегированного пользователя
RUN groupadd -r sample && useradd -r -g sample sample
USER sample

WORKDIR /app

COPY --from=build /app/build/qc_service .

COPY cfg ./cfg

ENTRYPOINT ["./qc_service"]
