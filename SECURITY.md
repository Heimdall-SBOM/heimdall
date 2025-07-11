# Security Policy

## Supported Versions

Use this section to tell people about which versions of your project are currently being supported with security updates.

| Version | Supported          |
| ------- | ------------------ |
| 0.2.x   | :white_check_mark: |
| 0.1.x   | :x:                |
| < 0.1   | :x:                |

## Reporting a Vulnerability

We take security vulnerabilities seriously. If you discover a security vulnerability in Heimdall, please follow these steps:

### 1. **DO NOT** create a public GitHub issue
Security vulnerabilities should be reported privately to prevent exploitation.

### 2. Email Security Team
Send an email to: **security@heimdall-sbom.org**

**Include the following information:**
- **Subject**: `[SECURITY] Heimdall Vulnerability Report`
- **Description**: Detailed description of the vulnerability
- **Steps to reproduce**: Clear steps to reproduce the issue
- **Impact assessment**: Potential impact of the vulnerability
- **Suggested fix**: If you have a proposed solution
- **Affected versions**: Which versions are affected
- **Proof of concept**: If applicable, include a minimal PoC

### 3. Response Timeline
- **Initial response**: Within 48 hours
- **Assessment**: Within 5 business days
- **Fix timeline**: Depends on severity (see below)

### 4. Severity Levels

| Level | Response Time | Description |
|-------|---------------|-------------|
| **Critical** | 24-48 hours | Remote code execution, privilege escalation, data breach |
| **High** | 3-5 days | Information disclosure, denial of service |
| **Medium** | 1-2 weeks | Limited information disclosure, minor DoS |
| **Low** | 2-4 weeks | Minor issues, best practice violations |

## Security Best Practices

### For Contributors
- Follow secure coding practices
- Use the provided safe string utilities (`safe_strlen()`, `is_null_terminated()`)
- Avoid unsafe C-style string operations
- Validate all input parameters
- Use bounded operations when dealing with potentially unsafe strings
- Run security tests before submitting PRs

### For Users
- Keep Heimdall updated to the latest version
- Review generated SBOMs for sensitive information
- Use HTTPS for downloading dependencies
- Validate SBOM integrity using checksums
- Monitor for security advisories

## Security Features

### Built-in Protections
- **Safe string handling**: Bounded string operations using `strnlen()`
- **Input validation**: Comprehensive parameter checking
- **Memory safety**: RAII and smart pointer usage
- **Error handling**: Graceful failure without information disclosure

### Security Testing
- **Static analysis**: Clang-tidy and SonarCloud integration
- **Dynamic testing**: Comprehensive test suite with security scenarios
- **Fuzzing**: Automated fuzz testing for input validation
- **Dependency scanning**: Regular security audits of dependencies

## Disclosure Policy

### Private Disclosure
- Vulnerabilities are kept private until a fix is ready
- Coordinated disclosure with affected parties
- No public discussion until patch is available

### Public Disclosure
- Security advisories published on GitHub
- CVE assignments for significant vulnerabilities
- Clear upgrade instructions provided
- Timeline for deprecating vulnerable versions

## Security Contacts

### Primary Security Contact
- **Email**: security@heimdall-sbom.org
- **PGP Key**: [Available on request]

### Security Team
- **Lead**: Project maintainer
- **Backup**: Core contributors
- **External**: Security researchers (coordinated)

## Bug Bounty

Currently, we do not offer a formal bug bounty program. However, we do appreciate security researchers who:

- Follow responsible disclosure practices
- Provide detailed, actionable reports
- Include proof-of-concept code when possible
- Allow reasonable time for fixes

## Security History

### Recent Security Fixes

#### [2024-12-19] Buffer Overread Vulnerability (CRITICAL)
- **CVE**: Pending
- **Description**: Fixed unsafe `strlen()` usage in `string_view` constructor
- **Impact**: Potential buffer overread and information disclosure
- **Fix**: Implemented safe string utilities with bounded operations
- **Versions affected**: All versions prior to current development
- **Status**: Fixed in development branch

## Security Resources

- [CVE Database](https://cve.mitre.org/)
- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [C++ Security Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [LLVM Security Policy](https://llvm.org/docs/Security.html)

## Acknowledgments

We thank the security researchers and contributors who have helped improve Heimdall's security posture through responsible disclosure and code reviews. 