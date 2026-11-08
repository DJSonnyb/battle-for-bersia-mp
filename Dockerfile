# Build environment matching the v1.4 release (Ubuntu 24.04: clang/lld 18.1.3, nasm 2.16.01, ImageMagick 6.9.12).
FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        python3 python3-pil clang lld llvm nasm imagemagick binutils coreutils && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /src
