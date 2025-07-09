FROM rockylinux:9
ENV container=docker

RUN dnf update \
   && dnf install -y \
    build-essential \
    cmake \
    ninja-build \
    binutils \
    binutils-dev \
    libssl-dev \
    libelf-dev \
    libgtest-dev \
    llvm-19-dev \
    liblld-19-dev \
    llvm-dev \
    pkg-config

