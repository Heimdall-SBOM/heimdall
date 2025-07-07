# Multi-stage Dockerfile for Heimdall SBOM Generator
# Stage 1: Build environment
FROM ubuntu:25.04 AS builder

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_BUILD_TYPE=Release
ENV CMAKE_CXX_STANDARD=23
ENV BUILD_LLD_PLUGIN=ON
ENV BUILD_GOLD_PLUGIN=ON
ENV BUILD_SHARED_CORE=ON
ENV BUILD_TESTS=ON
ENV BUILD_EXAMPLES=ON

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libelf-dev \
    binutils-dev \
    libbfd-dev \
    pkg-config \
    wget \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install LLVM 19 (recommended for full DWARF support)
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - && \
echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" | sudo tee /etc/apt/sources.list.d/llvm.list && \
sudo apt-get update && \
sudo apt-get install -y llvm-19-dev liblld-19-dev 
# Set compiler environment
ENV CC=clang-19
ENV CXX=clang++-19

# Copy source code
WORKDIR /src
COPY . .

# Configure and build
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_CXX_COMPILER=${CXX} \
    -DBUILD_LLD_PLUGIN=${BUILD_LLD_PLUGIN} \
    -DBUILD_GOLD_PLUGIN=${BUILD_GOLD_PLUGIN} \
    -DBUILD_SHARED_CORE=${BUILD_SHARED_CORE} \
    -DBUILD_TESTS=${BUILD_TESTS} \
    -DBUILD_EXAMPLES=${BUILD_EXAMPLES} \
    -DLLVM_DIR=/usr/lib/llvm-19/lib/cmake/llvm

RUN cmake --build build --parallel $(nproc)

# Run tests
RUN cd build && ctest --output-on-failure --verbose

# Install
RUN cmake --install build --prefix /opt/heimdall

# Stage 2: Runtime environment
FROM ubuntu:22.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libelf1 \
    binutils \
    libbfd-2.38-system \
    && rm -rf /var/lib/apt/lists/*

# Copy installed files from builder
COPY --from=builder /opt/heimdall /opt/heimdall

# Create symlinks for easy access
RUN ln -s /opt/heimdall/lib/heimdall-plugins/heimdall-lld.so /usr/local/lib/heimdall-lld.so && \
    ln -s /opt/heimdall/lib/heimdall-plugins/heimdall-gold.so /usr/local/lib/heimdall-gold.so && \
    ln -s /opt/heimdall/lib/libheimdall-core.so /usr/local/lib/libheimdall-core.so

# Set environment variables
ENV LD_LIBRARY_PATH=/opt/heimdall/lib:/usr/local/lib:$LD_LIBRARY_PATH
ENV PATH=/opt/heimdall/bin:$PATH

# Create non-root user
RUN useradd -m -s /bin/bash heimdall
USER heimdall

# Set working directory
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]

# Stage 3: Development environment (optional)
FROM runtime AS dev

USER root

# Install development tools
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    clang-tidy \
    cppcheck \
    && rm -rf /var/lib/apt/lists/*

# Switch back to non-root user
USER heimdall

# Stage 4: Minimal runtime (for production)
FROM ubuntu:22.04 AS minimal

# Install only essential runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libelf1 \
    && rm -rf /var/lib/apt/lists/*

# Copy only the core library and plugins
COPY --from=builder /opt/heimdall/lib/libheimdall-core.so /usr/local/lib/
COPY --from=builder /opt/heimdall/lib/heimdall-plugins/ /usr/local/lib/heimdall-plugins/

# Set environment variables
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Create non-root user
RUN useradd -m -s /bin/bash heimdall
USER heimdall

WORKDIR /workspace

# Metadata
LABEL maintainer="The Heimdall Authors"
LABEL description="Heimdall SBOM Generator - A comprehensive Software Bill of Materials generator for C++ projects"
LABEL version="1.0.0"
LABEL license="Apache-2.0"

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD test -f /usr/local/lib/libheimdall-core.so || exit 1 
