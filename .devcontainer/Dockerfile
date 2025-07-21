FROM archlinux:latest

ARG GCC_VERSION=11
ARG LLVM_VERSION=18
ARG LLVM_TAG=llvmorg-18.1.8

# Install prerequisites
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
      base-devel \
      cmake \
      ninja \
      python \
      python-pip \
      git \
      wget \
      curl \
      openssl \
      elfutils \
      pkg-config \
      boost \
      boost-libs

# Install GCC versions (Arch has latest by default, but we can install specific versions)
RUN if [ "$GCC_VERSION" = "11" ]; then \
      echo "GCC 11 not available in Arch, using default GCC (15)"; \
    elif [ "$GCC_VERSION" = "13" ]; then \
      echo "GCC 13 not available in Arch, using default GCC (15)"; \
    elif [ "$GCC_VERSION" = "14" ]; then \
      pacman -S --noconfirm gcc14; \
    fi

# Set GCC alternatives for GCC 14
RUN if [ "$GCC_VERSION" = "14" ]; then \
      ln -sf /usr/bin/gcc-14 /usr/bin/gcc && \
      ln -sf /usr/bin/g++-14 /usr/bin/g++; \
    fi

# Arch uses default GCC (currently 15), no symlinks needed

# Install requested LLVM version
RUN pacman -S --noconfirm llvm${LLVM_VERSION} lld${LLVM_VERSION}

# Create default symlinks for LLVM
RUN ln -sf /usr/bin/llvm-config-${LLVM_VERSION} /usr/bin/llvm-config && \
    ln -sf /usr/bin/ld.lld-${LLVM_VERSION} /usr/bin/ld.lld && \
    ln -sf /usr/bin/lld-${LLVM_VERSION} /usr/bin/lld

# Create symlinks for CMake config files so they can be found automatically
RUN mkdir -p /usr/lib/cmake && \
    ln -sf /usr/lib/llvm${LLVM_VERSION}/lib/cmake/llvm /usr/lib/cmake/llvm

# Create symlinks for LLVM headers so they can be found in standard include path
RUN mkdir -p /usr/include && \
    ln -sf /usr/lib/llvm${LLVM_VERSION}/include/llvm /usr/include/llvm && \
    ln -sf /usr/lib/llvm${LLVM_VERSION}/include/llvm-c /usr/include/llvm-c

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
