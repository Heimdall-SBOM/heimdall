# Multi-stage Dockerfile for Heimdall SBOM Generator
# Stage 1: Build environment
FROM rockylinux:9 AS builder

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
RUN dnf install -y \
    build-essential \
    cmake \
    openssl-devel \
    elfutils-libelf-devel \
    binutils-devel \
    binutils \
    pkgconfig \
    wget \
    git \
    && dnf clean all

# Install LLVM 19 (recommended for full DWARF support)
RUN dnf config-manager --add-repo https://yum.llvm.org/rocky/9/llvm-toolchain-rocky9-19/ && \
    dnf install -y llvm19-dev liblld19-dev

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
    -DLLVM_DIR=/usr/lib/llvm19/lib/cmake/llvm

RUN cmake --build build --parallel $(nproc)

# Run tests
RUN cd build && ctest --output-on-failure --verbose

# Install
RUN cmake --install build --prefix /opt/heimdall

# Stage 2: Runtime environment
FROM rockylinux:9 AS runtime

# Install runtime dependencies
RUN dnf install -y \
    openssl-libs \
    elfutils-libelf \
    binutils \
    && dnf clean all

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
RUN dnf install -y \
    gdb \
    valgrind \
    clang-tools-extra \
    cppcheck \
    && dnf clean all

# Switch back to non-root user
USER heimdall

# Stage 4: Minimal runtime (for production)
FROM rockylinux:9 AS minimal

# Install only essential runtime dependencies
RUN dnf install -y \
    openssl-libs \
    elfutils-libelf \
    && dnf clean all

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
