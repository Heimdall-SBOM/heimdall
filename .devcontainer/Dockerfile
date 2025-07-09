FROM rockylinux:9
ENV container=docker

RUN yum update -y \
    yum install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    llvm-devel \
    lld-devel \
    binutils-gold \
    openssl-devel \
    pkgconfig \
    zlib-devel \
    llvm-googletest \
    clang \
    lldb \
    llvm \
    gdb \
    gcc \
    g++ \
    llvm-googletest 

    RUN dnf install -y binutils-devel

RUN scl enable toolset-14 -- bash
