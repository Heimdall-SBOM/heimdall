# [Choice] Debian version (use bullseye on local arm64/Apple Silicon): bookworm, bullseye, buster
ARG VARIANT="bookworm"
FROM buildpack-deps:${VARIANT}-curl

RUN apt-get update \
   && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    binutils \
    binutils-dev \
    libssl-dev \
    libelf-dev \
    libgtest-dev \
    pkg-config

# Install LLVM 19 (recommended for full DWARF support)
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
&& echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" | sudo tee /etc/apt/sources.list.d/llvm.list \
&& apt-get update \
&& apt-get install -y llvm-19-dev liblld-19-dev llvm-dev 
