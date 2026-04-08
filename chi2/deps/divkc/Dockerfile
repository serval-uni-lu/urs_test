# ---------- Stage 1: Build ----------
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    zlib1g-dev \
    libboost-dev \
    libgmp10-dev \
    libgmpxx4ldbl \
    ninja-build \
    libboost-program-options1.83-dev \
    libboost-random1.83-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /divkc_build

COPY cppddnnf/ ./cppddnnf/
COPY splitter/ ./splitter/
COPY projection/ ./projection/
COPY D4/d4/ ./d4/
COPY D4/wrapper/ ./wrapper/

WORKDIR /divkc

RUN cd /divkc_build/cppddnnf && \
    g++ gen.cpp -o gen && \
    ./gen && \
    ninja clean && \
    ninja build/appmc build/sampler build/rsampler && \
    cp /divkc_build/cppddnnf/build/appmc /divkc/ && \
    cp /divkc_build/cppddnnf/build/sampler /divkc/ && \
    cp /divkc_build/cppddnnf/build/rsampler /divkc/ && \
    rm -rf /divkc_build/cppddnnf

RUN cd /divkc_build/splitter && \
    g++ gen.cpp -o gen && \
    ./gen && \
    ninja clean && \
    ninja && \
    cp /divkc_build/splitter/build/splitter /divkc/ && \
    rm -rf /divkc_build/splitter

RUN cd /divkc_build/projection && \
    g++ gen.cpp -o gen && \
    ./gen && \
    ninja clean && \
    ninja && \
    cp /divkc_build/projection/build/projection /divkc/ && \
    rm -rf /divkc_build/projection

RUN cd /divkc_build/wrapper && \
    make clean && make -j && \
    cp /divkc_build/wrapper/wrap /divkc/ && \
    rm -rf /divkc_build/wrapper

RUN cd /divkc_build/d4 && \
    make clean && make -j && \
    cp /divkc_build/d4/d4 /divkc/ && \
    rm -rf /divkc_build/d4

# ---------- Stage 2: Runtime ----------
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libboost-program-options1.83.0 \
    libboost-random1.83.0 \
    libgmp10 \
    libgmpxx4ldbl \
    libgomp1 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /divkc/ /divkc
