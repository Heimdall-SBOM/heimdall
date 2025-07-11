## Description

Please include a summary of the change and which issue is fixed. Please also include relevant motivation and context.

**Fixes**: #(issue)

## Type of Change

Please delete options that are not relevant.

- [ ] **Bug fix** (non-breaking change which fixes an issue)
- [ ] **New feature** (non-breaking change which adds functionality)
- [ ] **Breaking change** (fix or feature that would cause existing functionality to not work as expected)
- [ ] **Documentation update** (updates to documentation)
- [ ] **Performance improvement** (improves performance without changing functionality)
- [ ] **Refactoring** (no functional changes, code cleanup)
- [ ] **Test update** (updates to tests)
- [ ] **Build system change** (changes to build scripts, CI/CD)
- [ ] **Security fix** (fixes a security vulnerability)

## Testing

### Test Coverage

- [ ] **Unit tests** added/updated and passing
- [ ] **Integration tests** added/updated and passing
- [ ] **Manual testing** performed
- [ ] **Cross-platform testing** performed (Linux, macOS)
- [ ] **Compiler compatibility** tested (GCC, Clang)
- [ ] **C++ standard compatibility** tested (C++11, C++14, C++17, C++20, C++23)

### Test Commands

```bash
# Commands used to test the changes
./scripts/build.sh --standard 17 --compiler gcc --tests
./scripts/build.sh --standard 20 --compiler clang --tests
```

### Test Results

```
# Paste test output here
```

## Build Verification

### Build Commands

```bash
# Commands used to build the project
./scripts/build.sh --standard 17 --compiler gcc --all
```

### Build Results

```
# Paste build output here
```

## Code Quality

### Static Analysis

- [ ] **Clang-tidy** passes without warnings
- [ ] **SonarCloud** analysis passes
- [ ] **Code formatting** follows project standards
- [ ] **No compiler warnings** in debug and release builds

### Code Review Checklist

- [ ] **Code follows** project coding standards
- [ ] **Comments added** for complex logic
- [ ] **Error handling** implemented appropriately
- [ ] **Memory management** follows RAII principles
- [ ] **Security considerations** addressed
- [ ] **Backward compatibility** maintained (if applicable)
- [ ] **Performance impact** considered
- [ ] **Cross-platform compatibility** maintained

## Documentation

### Documentation Updates

- [ ] **README.md** updated (if applicable)
- [ ] **API documentation** updated (if applicable)
- [ ] **Usage examples** updated (if applicable)
- [ ] **CHANGELOG.md** updated
- [ ] **Comments** added to code (if applicable)

### Documentation Files Modified

- [ ] `README.md`
- [ ] `docs/usage.md`
- [ ] `docs/heimdall-developers-guide.md`
- [ ] `docs/heimdall-users-guide.md`
- [ ] `CHANGELOG.md`
- [ ] `SECURITY.md`
- [ ] Other: [specify]

## Security Considerations

### Security Impact

- [ ] **No security impact** - changes don't affect security
- [ ] **Security improvement** - changes improve security
- [ ] **Security fix** - changes fix a security vulnerability
- [ ] **Security review needed** - changes may have security implications

### Security Checklist

- [ ] **Input validation** implemented where needed
- [ ] **Safe string operations** used (avoiding `strlen()` on untrusted input)
- [ ] **Memory safety** maintained
- [ ] **No information disclosure** in error messages
- [ ] **Authentication/authorization** considered (if applicable)

## Breaking Changes

### Breaking Change Description

If this is a breaking change, please describe:
- **What changed**: [Description of what changed]
- **Why changed**: [Reason for the change]
- **Migration guide**: [How users can migrate]

### Migration Path

```bash
# Example migration commands or steps
```

## Performance Impact

### Performance Considerations

- [ ] **No performance impact** - changes don't affect performance
- [ ] **Performance improvement** - changes improve performance
- [ ] **Performance regression** - changes may impact performance
- [ ] **Performance testing** performed

### Performance Metrics

```
# Include performance test results if applicable
```

## Dependencies

### New Dependencies

- [ ] **No new dependencies** added
- [ ] **New dependencies** added: [list dependencies]
- [ ] **Dependency versions** updated: [list updates]

### Dependency Impact

- [ ] **No impact** on existing dependencies
- [ ] **Dependency added**: [describe impact]
- [ ] **Dependency updated**: [describe impact]
- [ ] **Dependency removed**: [describe impact]

## Platform Compatibility

### Platform Testing

- [ ] **Linux** tested (Ubuntu, CentOS, Fedora)
- [ ] **macOS** tested
- [ ] **Windows** tested (if applicable)
- [ ] **Cross-compilation** tested (if applicable)

### Compiler Compatibility

- [ ] **GCC** tested (versions: [list versions])
- [ ] **Clang** tested (versions: [list versions])
- [ ] **MSVC** tested (if applicable)

## Checklist

### Pre-submission Checklist

- [ ] **Code compiles** without errors
- [ ] **Tests pass** on all supported platforms
- [ ] **Documentation updated** to reflect changes
- [ ] **CHANGELOG.md updated** with appropriate entry
- [ ] **Security considerations** addressed
- [ ] **Performance impact** assessed
- [ ] **Breaking changes** documented (if any)
- [ ] **Dependencies** reviewed and updated (if any)

### Code Review Checklist

- [ ] **Code follows** project style guidelines
- [ ] **Comments** are clear and helpful
- [ ] **Error handling** is appropriate
- [ ] **Memory management** is correct
- [ ] **Security** is considered
- [ ] **Performance** is acceptable
- [ ] **Tests** are comprehensive
- [ ] **Documentation** is accurate

## Additional Information

### Related Issues

- **Closes**: #(issue number)
- **Relates to**: #(issue number)
- **Depends on**: #(issue number)

### Screenshots

If applicable, add screenshots to help explain your changes.

### Additional Context

Add any other context about the pull request here.

---

## Review Process

### For Reviewers

Please review the following aspects:

- [ ] **Functionality**: Does the code work as intended?
- [ ] **Code quality**: Is the code well-written and maintainable?
- [ ] **Testing**: Are the tests comprehensive and appropriate?
- [ ] **Documentation**: Is the documentation clear and complete?
- [ ] **Security**: Are there any security concerns?
- [ ] **Performance**: Is the performance acceptable?
- [ ] **Compatibility**: Does it work across supported platforms?

### For Contributors

- [ ] **I have tested** my changes thoroughly
- [ ] **I have updated** all relevant documentation
- [ ] **I have followed** the project's coding standards
- [ ] **I am ready** for code review and feedback
- [ ] **I understand** the contribution guidelines

---

**Note**: This template helps ensure that all necessary information is provided for effective code review. Please fill out all relevant sections to the best of your ability. 