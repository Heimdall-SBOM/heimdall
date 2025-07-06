# Heimdall SBOM Validation Guide

This guide explains how to validate Heimdall-generated SBOMs for standards compliance.

## Overview

Heimdall generates SBOMs in two formats:
- **CycloneDX** (JSON format) - ISO/IEC 5962:2021 standard
- **SPDX** (tag-value format) - ISO/IEC 5962:2021 standard

Both formats are validated for compliance with their respective specifications.

## Validation Methods

### 1. Automated Validation Scripts

Heimdall provides two validation scripts:

#### Bash Validation Script
```bash
./scripts/validate_sboms.sh build
```

**What it validates:**
- JSON syntax for CycloneDX files
- Required CycloneDX fields (bomFormat, specVersion, version, metadata, components)
- SPDX structure and required fields
- File format compliance

**Output:**
- Detailed validation logs in `build/validation_results/`
- Summary report with pass/fail status
- Individual validation logs for each SBOM

#### Python Validation Script
```bash
python3 scripts/validate_sboms_online.py build
```

**What it validates:**
- Advanced CycloneDX schema validation
- SPDX structure validation (both package and file formats)
- Detailed error reporting
- Links to online validation tools

### 2. Online Validation Tools

#### CycloneDX Validation
- **CycloneDX Tool Center**: https://cyclonedx.org/tool-center/
- **Schema Documentation**: https://cyclonedx.org/schema/
- **Specification**: https://cyclonedx.org/specification/

**Steps:**
1. Upload your `.json` SBOM file to the tool center
2. The tool will validate against the official CycloneDX schema
3. Review any validation errors or warnings

#### SPDX Validation
- **SPDX Validator**: https://tools.spdx.org/app/validate/
- **Specification**: https://spdx.github.io/spdx-spec/
- **Tools Repository**: https://github.com/spdx/tools

**Steps:**
1. Upload your `.spdx` file to the validator
2. The tool will validate against the SPDX specification
3. Review validation results

### 3. Manual Validation

#### CycloneDX Manual Checks
```bash
# Check JSON syntax
jq empty your-sbom.cyclonedx.json

# Validate required fields
jq '.bomFormat, .specVersion, .version, .metadata, .components' your-sbom.cyclonedx.json

# Check component structure
jq '.components[] | {type, name, version}' your-sbom.cyclonedx.json
```

#### SPDX Manual Checks
```bash
# Check required header fields
grep -E "^(SPDXVersion|DataLicense|DocumentName|DocumentNamespace):" your-sbom.spdx

# Check for package or file information
grep -E "^(PackageName|FileName):" your-sbom.spdx

# Validate SPDX version
grep "^SPDXVersion:" your-sbom.spdx
```

## Validation Results

### Expected Validation Outcomes

#### CycloneDX Files
- ✅ **bomFormat**: Must be "CycloneDX"
- ✅ **specVersion**: Must be one of "1.0", "1.1", "1.2", "1.3", "1.4", "1.5"
- ✅ **version**: Must be a positive integer
- ✅ **metadata**: Must contain timestamp and tools information
- ✅ **components**: Must be a non-empty array with valid component objects

#### SPDX Files
- ✅ **SPDXVersion**: Must start with "SPDX-"
- ✅ **DataLicense**: Must be a valid license identifier
- ✅ **DocumentName**: Must be present
- ✅ **DocumentNamespace**: Must be a valid URI
- ✅ **Package/File Information**: Must contain either PackageName or FileName entries

### Common Validation Issues

#### CycloneDX Issues
1. **Missing required fields**: Ensure all top-level fields are present
2. **Invalid specVersion**: Use a supported CycloneDX version
3. **Empty components array**: SBOM should contain at least one component
4. **Invalid component structure**: Each component must have type and name

#### SPDX Issues
1. **Missing header fields**: All required SPDX header fields must be present
2. **Invalid version format**: SPDX version must follow the correct format
3. **No package/file information**: SBOM must contain component information
4. **Invalid license identifiers**: Use valid SPDX license identifiers

## Extended DWARF Information Validation

Heimdall SBOMs include extended DWARF information in CycloneDX format:

### Custom Properties
- `heimdall:source-files`: Array of source file paths
- `heimdall:functions`: Array of function names and locations
- `heimdall:compile-units`: Array of compile unit information
- `heimdall:contains-debug-info`: Boolean indicating debug info presence

### Validation Commands
```bash
# Check for extended DWARF information
jq '.components[] | select(.properties) | .properties[] | select(.name | startswith("heimdall:"))' your-sbom.cyclonedx.json

# Validate source files property
jq '.components[] | select(.properties) | .properties[] | select(.name == "heimdall:source-files")' your-sbom.cyclonedx.json

# Validate functions property
jq '.components[] | select(.properties) | .properties[] | select(.name == "heimdall:functions")' your-sbom.cyclonedx.json
```

## Integration with CI/CD

### GitHub Actions Example
```yaml
- name: Generate SBOMs
  run: ./scripts/generate_build_sboms.sh build

- name: Validate SBOMs
  run: ./scripts/validate_sboms.sh build

- name: Upload validation results
  uses: actions/upload-artifact@v3
  with:
    name: sbom-validation-results
    path: build/validation_results/
```

### Jenkins Pipeline Example
```groovy
stage('SBOM Validation') {
    steps {
        sh './scripts/generate_build_sboms.sh build'
        sh './scripts/validate_sboms.sh build'
        archiveArtifacts artifacts: 'build/validation_results/**/*'
    }
}
```

## Troubleshooting

### Validation Script Issues
1. **Permission denied**: Ensure scripts are executable (`chmod +x scripts/*.sh`)
2. **Missing dependencies**: Install required tools (`jq`, `python3`)
3. **No SBOM files**: Run the generation script first

### Online Validation Issues
1. **File too large**: Some online tools have file size limits
2. **Network issues**: Use local validation as fallback
3. **Format issues**: Ensure files are properly formatted

### Common Fixes
1. **JSON syntax errors**: Use `jq` to format and validate JSON
2. **SPDX format issues**: Check field formatting and required fields
3. **Missing components**: Ensure the build process generated components

## Additional Resources

### Standards Documentation
- [CycloneDX Specification](https://cyclonedx.org/specification/)
- [SPDX Specification](https://spdx.github.io/spdx-spec/)
- [ISO/IEC 5962:2021](https://www.iso.org/standard/81870.html)

### Validation Tools
- [OWASP Dependency Check](https://owasp.org/www-project-dependency-check/)
- [Snyk](https://snyk.io/)
- [FOSSA](https://fossa.com/)
- [Black Duck](https://www.blackducksoftware.com/)

### Community Resources
- [CycloneDX Community](https://cyclonedx.org/community/)
- [SPDX Community](https://spdx.dev/community/)
- [OpenSSF](https://openssf.org/)

## Support

For validation issues specific to Heimdall:
1. Check the validation logs in `build/validation_results/`
2. Review the SBOM generation logs
3. Consult the Heimdall documentation
4. Open an issue on the Heimdall repository

For general SBOM validation questions:
1. Consult the respective standard specifications
2. Use the official validation tools
3. Engage with the SBOM community 