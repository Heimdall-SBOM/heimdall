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
    pandoc \
    llvm-googletest 

    RUN dnf install -y binutils-devel \
                       libffi-devel \
                       elfutils-libelf-devel \
                       llvm-devel \
                       lld-devel \
                       epel-release \
                       pandoc \
                       texlive
