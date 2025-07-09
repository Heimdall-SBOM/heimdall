# Multi-stage Dockerfile for Heimdall SBOM Generator
# Stage 1: Build environment
FROM rockylinux:9
LABEL maintainer="Heimdall Authors"
ENV container=docker

# Set the working directory to /workspaces
WORKDIR /workspaces

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_BUILD_TYPE=Release
ENV CMAKE_CXX_STANDARD=17
ENV BUILD_LLD_PLUGIN=ON
ENV BUILD_GOLD_PLUGIN=ON
ENV BUILD_SHARED_CORE=ON
ENV BUILD_TESTS=ON
ENV BUILD_EXAMPLES=ON

# Install build dependencies
RUN yum install -y \
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
    binutils-devel

# Metadata
LABEL maintainer="The Heimdall Authors"
LABEL description="Heimdall SBOM Generator - A comprehensive Software Bill of Materials generator for C++ projects"
LABEL version="1.0.0"
LABEL license="Apache-2.0"

