---
name: Bug report
about: Create a report to help us improve Heimdall
title: '[BUG] '
labels: ['bug', 'needs-triage']
assignees: ''
---

## Bug Description

A clear and concise description of what the bug is.

## Steps to Reproduce

1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

## Expected Behavior

A clear and concise description of what you expected to happen.

## Actual Behavior

A clear and concise description of what actually happened.

## Environment Information

### System Information
- **OS**: [e.g., Ubuntu 22.04, macOS 13.0, Windows 11]
- **Architecture**: [e.g., x86_64, ARM64]
- **Kernel**: [e.g., Linux 5.15.0]

### Heimdall Information
- **Version**: [e.g., 0.2.0, development branch]
- **Build Type**: [e.g., Release, Debug]
- **Compiler**: [e.g., GCC 11.3, Clang 14.0]
- **C++ Standard**: [e.g., C++17, C++20]

### Dependencies
- **LLVM Version**: [e.g., 18.1.7]
- **CMake Version**: [e.g., 3.24.0]
- **OpenSSL Version**: [e.g., 3.2.2]

## Build Information

### Build Command
```bash
# Paste the exact build command you used
./scripts/build.sh --standard 17 --compiler gcc --tests
```

### Build Output
```
# Paste relevant build output or errors
```

## Runtime Information

### Command Used
```bash
# Paste the exact command that triggered the bug
```

### Error Messages
```
# Paste any error messages, stack traces, or logs
```

### Debug Output
```
# If applicable, paste debug output with HEIMDALL_DEBUG_ENABLED=1
```

### Related Issues
- **Previous Issues**: [Link to any related issues]
- **Similar Problems**: [Describe any similar problems you've encountered]

## Reproducibility

- [ ] **Always**: The bug occurs every time
- [ ] **Sometimes**: The bug occurs intermittently
- [ ] **Rarely**: The bug occurs occasionally
- [ ] **Once**: The bug occurred once and hasn't repeated

## Impact Assessment

- [ ] **Critical**: Prevents basic functionality
- [ ] **High**: Significant functionality affected
- [ ] **Medium**: Minor functionality affected
- [ ] **Low**: Cosmetic or minor issue

## Proposed Solution

If you have a suggested fix or workaround, please describe it here.

## Additional Files

Please attach any relevant files:
- [ ] Build logs
- [ ] Error logs
- [ ] Configuration files
- [ ] Input files that trigger the issue
- [ ] Output files that show the problem
- [ ] Screenshots (if applicable)

---

**Note**: For security-related issues, please do NOT use this template. Instead, email security@heimdall-sbom.org directly. 
