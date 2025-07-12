# Heimdall Build Server Implementation Plan

## Overview

This document outlines the implementation plan for **heimdall-build**, a centralized build server hosted on Linode that will provide automated building and verification of the Heimdall project across multiple compiler configurations and platforms.

## Goals

- **Multi-Platform Support**: Linux, macOS, and Windows builds
- **Multi-Compiler Support**: GCC and LLVM (Clang) across multiple versions
- **Multi-Standard Support**: C++11, C++14, C++17, C++20, and C++23
- **Automated Verification**: Build, test, and validate each configuration
- **Scalable Architecture**: Docker-based containerization for isolation and scalability
- **CI/CD Integration**: Seamless integration with existing development workflows

## Architecture

### Server Infrastructure

```
heimdall-build.linode.com
├── Load Balancer (Nginx)
├── Build Orchestrator (Python/Flask)
├── Docker Registry (Harbor)
├── Artifact Storage (MinIO)
├── Database (PostgreSQL)
└── Monitoring (Prometheus + Grafana)
```

### Build Matrix

| Platform | Compiler | C++ Standards | LLVM Versions | Container Base |
|----------|----------|---------------|---------------|----------------|
| Linux (Ubuntu 22.04) | GCC 7-20 | 11, 14, 17, 20, 23 | 7-21 | ubuntu:22.04 |
| Linux (CentOS 8) | GCC 7-20 | 11, 14, 17, 20, 23 | 7-21 | centos:8 |
| macOS (Monterey) | Clang 7-20 | 11, 14, 17, 20, 23 | 7-21 | mcr.microsoft.com/mac |
| Windows (Server 2022) | MSVC 2019-2022 | 14, 17, 20, 23 | 7-21 | mcr.microsoft.com/windows |

## Implementation Phases

### Phase 1: Infrastructure Setup (Week 1-2)

#### 1.1 Linode Server Provisioning
- **Server Specifications**:
  - CPU: 8 cores (minimum)
  - RAM: 32GB (minimum)
  - Storage: 500GB SSD
  - OS: Ubuntu 22.04 LTS
  - Network: 1Gbps

- **Security Setup**:
  - SSH key-based authentication
  - UFW firewall configuration
  - SSL/TLS certificates (Let's Encrypt)
  - Regular security updates

#### 1.2 Docker Environment
```bash
# Install Docker and Docker Compose
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER

# Install Docker Compose
sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose
```

#### 1.3 Base Infrastructure Services
- **Nginx**: Reverse proxy and load balancer
- **PostgreSQL**: Build metadata and results storage
- **Redis**: Job queue and caching
- **MinIO**: Artifact storage (S3-compatible)
- **Harbor**: Private Docker registry

### Phase 2: Build System Core (Week 3-4)

#### 2.1 Build Orchestrator Service
**Technology Stack**: Python 3.11 + Flask + Celery + Redis

**Key Components**:
- **Build Scheduler**: Manages build queue and resource allocation
- **Container Manager**: Orchestrates Docker containers for builds
- **Result Collector**: Gathers build artifacts and test results
- **Notification System**: Reports build status via webhooks/email

**API Endpoints**:
```python
POST /api/v1/builds          # Submit new build request
GET  /api/v1/builds          # List all builds
GET  /api/v1/builds/{id}     # Get build details
GET  /api/v1/builds/{id}/logs # Get build logs
DELETE /api/v1/builds/{id}   # Cancel build
```

#### 2.2 Build Configuration Management
**Configuration Schema**:
```yaml
build_config:
  platform: "linux" | "macos" | "windows"
  compiler:
    type: "gcc" | "clang" | "msvc"
    version: "7" | "8" | "9" | "10" | "11" | "12" | "13" | "14" | "15" | "16" | "17" | "18" | "19" | "20"
  cxx_standard: "11" | "14" | "17" | "20" | "23"
  llvm_version: "7" | "8" | "9" | "10" | "11" | "12" | "13" | "14" | "15" | "16" | "17" | "18" | "19" | "20" | "21"
  build_type: "Debug" | "Release" | "RelWithDebInfo" | "MinSizeRel"
  enable_tests: true | false
  enable_coverage: true | false
  enable_sanitizers: true | false
```

### Phase 3: Docker Container Images (Week 5-6)

#### 3.1 Base Images for Each Platform

**Linux Base Image** (`docker/linux-base.Dockerfile`):
```dockerfile
FROM ubuntu:22.04

# Install essential packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# Install multiple GCC versions
RUN for version in 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do \
    apt-get update && apt-get install -y gcc-${version} g++-${version}; \
    done

# Install multiple LLVM versions
RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh
RUN for version in 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21; do \
    ./llvm.sh ${version}; \
    done

# Install Boost for C++11/14 compatibility
RUN apt-get update && apt-get install -y \
    libboost-filesystem-dev \
    libboost-system-dev

WORKDIR /workspace
```

**macOS Base Image** (`docker/macos-base.Dockerfile`):
```dockerfile
FROM mcr.microsoft.com/mac:monterey

# Install Xcode Command Line Tools
RUN xcode-select --install

# Install Homebrew
RUN /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install multiple LLVM versions via Homebrew
RUN for version in 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21; do \
    brew install llvm@${version}; \
    done

# Install Boost
RUN brew install boost

WORKDIR /workspace
```

**Windows Base Image** (`docker/windows-base.Dockerfile`):
```dockerfile
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Install Visual Studio Build Tools
RUN powershell -Command \
    Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_buildtools.exe" -OutFile "vs_buildtools.exe" && \
    .\vs_buildtools.exe --quiet --wait --norestart --nocache \
    --installPath C:\BuildTools \
    --add Microsoft.VisualStudio.Workload.VCTools \
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041

# Install multiple LLVM versions
RUN for version in 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21; do \
    powershell -Command \
    Invoke-WebRequest -Uri "https://github.com/llvm/llvm-project/releases/download/llvmorg-${version}.0.0/LLVM-${version}.0.0-win64.exe" -OutFile "LLVM-${version}.exe" && \
    .\LLVM-${version}.exe /S; \
    done

WORKDIR C:\workspace
```

#### 3.2 Build-Specific Images
Create specialized images for each build configuration:

```bash
# Example: Linux + GCC 11 + C++17
docker build -f docker/linux-gcc11-cpp17.Dockerfile -t heimdall-build:linux-gcc11-cpp17 .

# Example: macOS + Clang 15 + C++20
docker build -f docker/macos-clang15-cpp20.Dockerfile -t heimdall-build:macos-clang15-cpp20 .

# Example: Windows + MSVC 2022 + C++20
docker build -f docker/windows-msvc2022-cpp20.Dockerfile -t heimdall-build:windows-msvc2022-cpp20 .
```

### Phase 4: Build Execution Engine (Week 7-8)

#### 4.1 Build Job Execution
**Job Queue Management**:
```python
# Celery task for build execution
@celery.task(bind=True)
def execute_build(self, build_config, source_url, branch):
    try:
        # 1. Clone source code
        repo_path = clone_repository(source_url, branch)
        
        # 2. Select appropriate container
        container_image = select_container_image(build_config)
        
        # 3. Execute build in container
        result = run_build_container(container_image, repo_path, build_config)
        
        # 4. Collect artifacts
        artifacts = collect_build_artifacts(result)
        
        # 5. Run tests
        test_results = run_tests(result)
        
        # 6. Generate SBOMs
        sboms = generate_sboms(result)
        
        # 7. Store results
        store_build_results(build_config, result, artifacts, test_results, sboms)
        
        return {
            'status': 'success',
            'build_id': result['build_id'],
            'artifacts': artifacts,
            'test_results': test_results,
            'sboms': sboms
        }
    except Exception as e:
        self.update_state(state='FAILURE', meta={'error': str(e)})
        raise
```

#### 4.2 Container Build Execution
```python
def run_build_container(image, repo_path, config):
    client = docker.from_env()
    
    # Mount source code and output directories
    volumes = {
        repo_path: {'bind': '/workspace/src', 'mode': 'ro'},
        '/tmp/build-output': {'bind': '/workspace/output', 'mode': 'rw'}
    }
    
    # Environment variables for build configuration
    environment = {
        'CXX_STANDARD': config['cxx_standard'],
        'BUILD_TYPE': config['build_type'],
        'ENABLE_TESTS': str(config['enable_tests']).lower(),
        'ENABLE_COVERAGE': str(config['enable_coverage']).lower(),
        'LLVM_VERSION': config['llvm_version'],
        'COMPILER_VERSION': config['compiler']['version']
    }
    
    # Execute build script
    container = client.containers.run(
        image,
        command='/workspace/scripts/build_in_container.sh',
        volumes=volumes,
        environment=environment,
        detach=True
    )
    
    # Stream logs
    for log in container.logs(stream=True):
        yield log.decode('utf-8')
    
    # Wait for completion
    result = container.wait()
    return {
        'exit_code': result['StatusCode'],
        'logs': container.logs().decode('utf-8'),
        'build_id': str(uuid.uuid4())
    }
```

### Phase 5: Integration with Existing Scripts (Week 9-10)

#### 5.1 Container Build Script
Create `scripts/build_in_container.sh` that adapts existing build scripts:

```bash
#!/bin/bash
# Build script for Docker containers

set -e

# Source environment based on compiler and LLVM version
source /workspace/scripts/setup_compiler_env.sh

# Navigate to source directory
cd /workspace/src

# Run existing build script with container-specific options
./build.sh \
    --build-type "${BUILD_TYPE:-Release}" \
    --cxx-standard "${CXX_STANDARD:-17}" \
    --tests \
    --coverage

# Run tests if enabled
if [ "${ENABLE_TESTS:-true}" = "true" ]; then
    echo "Running tests..."
    ctest --output-on-failure
fi

# Generate SBOMs
echo "Generating SBOMs..."
./scripts/generate_build_sboms.sh build/

# Copy artifacts to output directory
echo "Copying artifacts..."
cp -r build/ /workspace/output/
cp -r sboms/ /workspace/output/ 2>/dev/null || true

echo "Build completed successfully"
```

#### 5.2 Environment Setup Script
Create `scripts/setup_compiler_env.sh`:

```bash
#!/bin/bash
# Setup compiler environment based on configuration

# Set compiler based on platform and version
case "${PLATFORM:-linux}" in
    linux)
        if [ "${COMPILER_TYPE:-gcc}" = "gcc" ]; then
            export CC="gcc-${COMPILER_VERSION}"
            export CXX="g++-${COMPILER_VERSION}"
        else
            export CC="clang-${COMPILER_VERSION}"
            export CXX="clang++-${COMPILER_VERSION}"
        fi
        ;;
    macos)
        export CC="clang"
        export CXX="clang++"
        # Set LLVM paths
        export LLVM_CONFIG="/opt/homebrew/opt/llvm@${LLVM_VERSION}/bin/llvm-config"
        ;;
    windows)
        # Setup Visual Studio environment
        source "C:/BuildTools/VC/Auxiliary/Build/vcvars64.bat"
        export CC="cl"
        export CXX="cl"
        ;;
esac

# Set LLVM configuration
if [ -n "${LLVM_VERSION}" ]; then
    export LLVM_CONFIG="llvm-config-${LLVM_VERSION}"
fi

echo "Environment setup complete:"
echo "  CC: $CC"
echo "  CXX: $CXX"
echo "  LLVM_CONFIG: $LLVM_CONFIG"
```

### Phase 6: Web Interface and API (Week 11-12)

#### 6.1 Web Dashboard
**Technology**: React + TypeScript + Material-UI

**Features**:
- Build queue monitoring
- Real-time build logs
- Build history and results
- Artifact download
- Build configuration management
- System status and metrics

#### 6.2 REST API
**Endpoints**:
```python
# Build Management
POST   /api/v1/builds                    # Submit build
GET    /api/v1/builds                    # List builds
GET    /api/v1/builds/{id}               # Get build details
DELETE /api/v1/builds/{id}               # Cancel build

# Build Results
GET    /api/v1/builds/{id}/logs          # Get build logs
GET    /api/v1/builds/{id}/artifacts     # List artifacts
GET    /api/v1/builds/{id}/artifacts/{file} # Download artifact
GET    /api/v1/builds/{id}/sboms         # Get SBOMs

# Configuration
GET    /api/v1/configurations            # List build configurations
POST   /api/v1/configurations            # Create configuration
PUT    /api/v1/configurations/{id}       # Update configuration
DELETE /api/v1/configurations/{id}       # Delete configuration

# System Status
GET    /api/v1/status                    # System status
GET    /api/v1/metrics                   # System metrics
GET    /api/v1/queue                     # Build queue status
```

### Phase 7: CI/CD Integration (Week 13-14)

#### 7.1 GitHub Actions Integration
```yaml
# .github/workflows/build-server.yml
name: Build Server Integration

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-server:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Submit to Build Server
        run: |
          curl -X POST https://heimdall-build.linode.com/api/v1/builds \
            -H "Content-Type: application/json" \
            -d '{
              "repository": "${{ github.repository }}",
              "branch": "${{ github.ref_name }}",
              "commit": "${{ github.sha }}",
              "configurations": [
                {"platform": "linux", "compiler": "gcc", "version": "11", "cxx_standard": "17"},
                {"platform": "linux", "compiler": "clang", "version": "15", "cxx_standard": "20"},
                {"platform": "macos", "compiler": "clang", "version": "15", "cxx_standard": "20"},
                {"platform": "windows", "compiler": "msvc", "version": "2022", "cxx_standard": "20"}
              ]
            }'
```

#### 7.2 Build Status Reporting
```python
# Webhook handler for build completion
@app.route('/webhooks/build-complete', methods=['POST'])
def build_complete_webhook():
    data = request.json
    
    # Update GitHub commit status
    update_github_status(
        repo=data['repository'],
        commit=data['commit'],
        state=data['status'],
        description=f"Build {data['build_id']} completed"
    )
    
    # Send notifications
    send_notifications(data)
    
    return jsonify({'status': 'success'})
```

### Phase 8: Monitoring and Maintenance (Week 15-16)

#### 8.1 Monitoring Stack
- **Prometheus**: Metrics collection
- **Grafana**: Dashboards and alerting
- **ELK Stack**: Log aggregation and analysis
- **Health Checks**: Automated system monitoring

#### 8.2 Backup and Recovery
- **Database Backups**: Daily automated backups
- **Artifact Backups**: S3-compatible storage replication
- **Configuration Backups**: Version-controlled configuration
- **Disaster Recovery**: Automated recovery procedures

## Deployment Architecture

### Docker Compose Configuration
```yaml
# docker-compose.yml
version: '3.8'

services:
  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
      - ./ssl:/etc/nginx/ssl
    depends_on:
      - build-orchestrator

  build-orchestrator:
    build: ./build-orchestrator
    environment:
      - DATABASE_URL=postgresql://heimdall:password@postgres:5432/heimdall
      - REDIS_URL=redis://redis:6379
      - MINIO_URL=http://minio:9000
    depends_on:
      - postgres
      - redis
      - minio

  celery-worker:
    build: ./build-orchestrator
    command: celery -A app.celery worker --loglevel=info
    environment:
      - DATABASE_URL=postgresql://heimdall:password@postgres:5439/heimdall
      - REDIS_URL=redis://redis:6379
    depends_on:
      - postgres
      - redis

  postgres:
    image: postgres:15
    environment:
      - POSTGRES_DB=heimdall
      - POSTGRES_USER=heimdall
      - POSTGRES_PASSWORD=password
    volumes:
      - postgres_data:/var/lib/postgresql/data

  redis:
    image: redis:7-alpine
    volumes:
      - redis_data:/data

  minio:
    image: minio/minio
    command: server /data --console-address ":9001"
    environment:
      - MINIO_ROOT_USER=admin
      - MINIO_ROOT_PASSWORD=password123
    volumes:
      - minio_data:/data

  prometheus:
    image: prom/prometheus
    ports:
      - "9090:9090"
    volumes:
      - ./prometheus.yml:/etc/prometheus/prometheus.yml

  grafana:
    image: grafana/grafana
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
    volumes:
      - grafana_data:/var/lib/grafana

volumes:
  postgres_data:
  redis_data:
  minio_data:
  grafana_data:
```

## Security Considerations

### Network Security
- **Firewall Rules**: Restrict access to necessary ports only
- **SSL/TLS**: Encrypt all communications
- **VPN Access**: Secure remote access for administration
- **Rate Limiting**: Prevent abuse of build resources

### Container Security
- **Image Scanning**: Regular vulnerability scans
- **Non-Root Users**: Run containers as non-root users
- **Resource Limits**: Prevent resource exhaustion
- **Network Isolation**: Isolate build containers

### Data Security
- **Encryption at Rest**: Encrypt stored artifacts and databases
- **Access Control**: Role-based access control (RBAC)
- **Audit Logging**: Comprehensive audit trails
- **Backup Encryption**: Encrypt backup data

## Cost Estimation

### Linode Server Costs (Monthly)
- **CPU**: 8 cores × $48 = $384
- **RAM**: 32GB × $24 = $768
- **Storage**: 500GB × $0.10 = $50
- **Bandwidth**: 4TB included
- **Total**: ~$1,202/month

### Additional Services
- **Domain**: $12/year
- **SSL Certificate**: Free (Let's Encrypt)
- **Monitoring**: Free (self-hosted)
- **Backup Storage**: $50/month (S3-compatible)

### Total Estimated Cost: ~$1,252/month

## Success Metrics

### Performance Metrics
- **Build Time**: Average build completion time < 30 minutes
- **Queue Time**: Average wait time < 10 minutes
- **Uptime**: 99.9% availability
- **Resource Utilization**: < 80% average CPU/RAM usage

### Quality Metrics
- **Build Success Rate**: > 95% successful builds
- **Test Pass Rate**: > 98% test pass rate
- **Artifact Integrity**: 100% artifact verification
- **SBOM Generation**: 100% successful SBOM generation

### User Experience Metrics
- **API Response Time**: < 500ms average
- **Web Interface Load Time**: < 3 seconds
- **User Satisfaction**: > 4.5/5 rating
- **Support Response Time**: < 4 hours

## Risk Mitigation

### Technical Risks
- **Container Resource Exhaustion**: Implement resource limits and monitoring
- **Build Timeouts**: Set appropriate timeout values and retry mechanisms
- **Storage Overflow**: Implement artifact retention policies
- **Network Failures**: Implement retry logic and fallback mechanisms

### Operational Risks
- **Server Downtime**: Implement high availability and backup procedures
- **Data Loss**: Regular backups and disaster recovery procedures
- **Security Breaches**: Regular security audits and updates
- **Cost Overruns**: Monitor resource usage and implement alerts

## Conclusion

The heimdall-build server will provide a robust, scalable, and secure build infrastructure that supports the project's multi-platform and multi-compiler requirements. The phased implementation approach ensures steady progress while maintaining system stability and quality.

The server will integrate seamlessly with existing development workflows while providing comprehensive monitoring, reporting, and artifact management capabilities. The Docker-based architecture ensures consistency across all build environments and simplifies maintenance and scaling.

## Next Steps

1. **Infrastructure Setup**: Begin with Phase 1 server provisioning
2. **Team Training**: Provide training on Docker and container orchestration
3. **Pilot Testing**: Start with a subset of build configurations
4. **Gradual Migration**: Migrate existing CI/CD pipelines to the new system
5. **Continuous Improvement**: Monitor metrics and optimize performance