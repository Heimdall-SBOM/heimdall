# Jenkins Image Mapping

This document describes the mapping between Jenkins Docker image names, compiler (GCC) versions, and C++ standards for each supported Linux distribution.

## Ubuntu
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11          | heimdall-ubuntu-gcc11     |
| C++14        | 14          | heimdall-ubuntu-gcc14     |
| C++17        | 17          | heimdall-ubuntu-gcc17     |
| C++20        | 20          | heimdall-ubuntu-gcc20     |
| C++23        | 23          | heimdall-ubuntu-gcc23     |

## Rocky 9
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11          | heimdall-rocky9-gcc11     |
| C++14        | 14          | heimdall-rocky9-gcc14     |
| C++17        | 17          | heimdall-rocky9-gcc17     |
| C++20        | 20          | heimdall-rocky9-gcc20     |
| C++23        | 23          | heimdall-rocky9-gcc23     |

## CentOS
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11          | heimdall-centos-gcc11     |
| C++14        | 13          | heimdall-centos-gcc13     |
| C++17        | 13          | heimdall-centos-gcc13     |
| C++20        | 14          | heimdall-centos-gcc14     |
| C++23        | 14          | heimdall-centos-gcc14     |

## Fedora
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11          | heimdall-fedora-gcc11     |
| C++14        | 15          | heimdall-fedora-gcc15     |
| C++17        | 15          | heimdall-fedora-gcc15     |
| C++20        | 15          | heimdall-fedora-gcc15     |
| C++23        | 15          | heimdall-fedora-gcc15     |

## Arch
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11 (default 15) | heimdall-arch-gcc11  |
| C++14        | 14          | heimdall-arch-gcc14       |
| C++17        | 15          | heimdall-arch-gcc15       |
| C++20        | 15          | heimdall-arch-gcc15       |
| C++23        | 15          | heimdall-arch-gcc15       |

## Debian Testing
| C++ Standard | GCC Version | Image Name                        |
|--------------|-------------|-----------------------------------|
| C++11        | 12          | heimdall-debian-testing-gcc12     |
| C++14        | 13          | heimdall-debian-testing-gcc13     |
| C++17        | 13          | heimdall-debian-testing-gcc13     |
| C++20        | 13          | heimdall-debian-testing-gcc13     |
| C++23        | 14          | heimdall-debian-testing-gcc14     |

**Note:** Debian Testing provides GCC 12, 13, and 14. C++23 uses GCC 14 for full support.

## Debian
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11          | heimdall-debian-gcc11     |
| C++14        | 11          | heimdall-debian-gcc11     |
| C++17        | 11          | heimdall-debian-gcc11     |
| C++20        | 12          | heimdall-debian-gcc12     |
| C++23        | 12          | heimdall-debian-gcc12     |

**Note:** Debian Bookworm only provides GCC 11 and 12. C++20 and C++23 use GCC 12, while C++14 and C++17 use GCC 11.

## OpenSUSE
| C++ Standard | GCC Version | Image Name                |
|--------------|-------------|---------------------------|
| C++11        | 11          | heimdall-opensuse-gcc11   |
| C++14        | 11          | heimdall-opensuse-gcc11   |
| C++17        | 11          | heimdall-opensuse-gcc11   |
| C++20        | 13          | heimdall-opensuse-gcc13   |
| C++23        | 13          | heimdall-opensuse-gcc13   |

**Note:** OpenSUSE Tumbleweed uses the latest GCC by default. Only unversioned `llvm`, `llvm-devel`, and `lld` packages are available. The image mapping above reflects the supported standards and GCC versions.

---

**Note:**
- For some distributions, not all GCC versions are available. The image may use the closest available version (e.g., Arch uses default GCC 15 for C++11).
