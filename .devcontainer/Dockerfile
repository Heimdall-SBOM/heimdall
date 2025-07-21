FROM ubuntu:22.04

ARG GCC_VERSION=11
ARG LLVM_VERSION=18
ARG LLVM_TAG=llvmorg-18.1.8

# Install prerequisites
RUN apt-get update && \
    apt-get install -y \
      software-properties-common \
      wget \
      gnupg \
      lsb-release \
      curl

# Always add the Ubuntu Toolchain PPA so both GCC 11 and 13 are available
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y && apt-get update

# Install build tools and all needed GCC/G++
RUN apt-get update && \
    apt-get install -y \
      build-essential \
      gcc-11 g++-11 \
      gcc-13 g++-13 \
      cmake \
      ninja-build \
      python3 \
      python3-pip \
      git \
      binutils-gold \
      libssl-dev \
      libelf-dev \
      pkg-config \
      libboost-filesystem-dev \
      libboost-system-dev

# Set GCC alternatives
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} 100

# Install requested LLVM version
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh ${LLVM_VERSION}

RUN apt-get update && \
    apt-get install -y \
      llvm-${LLVM_VERSION}-dev \
      lld-${LLVM_VERSION}

# Create default symlinks for LLVM
RUN ln -sf /usr/bin/llvm-config-${LLVM_VERSION} /usr/bin/llvm-config && \
    ln -sf /usr/bin/ld.lld-${LLVM_VERSION} /usr/bin/ld.lld && \
    ln -sf /usr/bin/lld-${LLVM_VERSION} /usr/bin/lld

# Install LLD headers for the selected LLVM version
RUN mkdir -p /usr/local/include/lld/Common && \
    cd /usr/local/include/lld/Common && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Driver.h -o Driver.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Args.h -o Args.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/ErrorHandler.h -o ErrorHandler.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Filesystem.h -o Filesystem.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/LLVM.h -o LLVM.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Memory.h -o Memory.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Reproduce.h -o Reproduce.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Strings.h -o Strings.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Timer.h -o Timer.h && \
    curl -s https://raw.githubusercontent.com/llvm/llvm-project/${LLVM_TAG}/lld/include/lld/Common/Version.h -o Version.h 
