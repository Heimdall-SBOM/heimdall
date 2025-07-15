# Heimdall DWARF-Rich CycloneDX Example

This example demonstrates a realistic C++ Task Manager application designed to generate a significant amount of DWARF debug information for Heimdall's CycloneDX SBOM extraction.

## Features
- Multiple classes (Task, Project, User, TaskManager)
- Inheritance, virtual methods, templates
- Namespaces, enums, typedefs, STL containers
- Multiple source and header files
- Exception handling, standard algorithms

## Build Instructions

```bash
cd examples/heimdall-dwarf-rich-cyclonedx-example
mkdir -p build && cd build
cmake ..
make -j
```

## Generate CycloneDX SBOM with DWARF Info

1. Ensure Heimdall is built (see main repo README).
2. Run the build script:
   ```bash
   ./build_and_generate_sbom.sh
   ```
   This will build with debug info and generate `taskmgr.cyclonedx.json`.

3. Inspect the SBOM for DWARF info (source files, functions, etc.):
   ```bash
   grep -i debug taskmgr.cyclonedx.json
   grep -i source taskmgr.cyclonedx.json
   ```

## Example Output (CycloneDX SBOM)

```json
{
  "containsDebugInfo": true,
  "sourceFiles": [
    "Task.cpp",
    "Project.cpp",
    "User.cpp",
    "TaskManager.cpp",
    "utils.cpp",
    "main.cpp"
  ],
  ...
}
```

## Run the Example

```bash
./taskmgr
```

---

This example is designed to maximize DWARF debug info for SBOM demonstration.

