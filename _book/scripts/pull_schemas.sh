#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Assume the repository root is one level up from scripts/ (heimdall)
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

# Check if we're in the heimdall repository by looking for a specific marker (e.g., .git directory)
if [[ ! -d "$REPO_ROOT/.git" ]]; then
    echo "Error: This script must be run from within the heimdall repository."
    echo "The repository root (heimdall) could not be found at $REPO_ROOT."
    exit 1
fi

# Define the schema directory relative to the repository root
SCHEMA_DIR="$REPO_ROOT/schema"

# Grab CycloneDX specs
curl https://cyclonedx.org/schema/bom-1.6.schema.json -o "$SCHEMA_DIR/cyclonedx-bom-1.6.schema.json"
curl https://cyclonedx.org/schema/bom-1.5.schema.json -o "$SCHEMA_DIR/cyclonedx-bom-1.5.schema.json"
curl https://cyclonedx.org/schema/bom-1.4.schema.json -o "$SCHEMA_DIR/cyclonedx-bom-1.4.schema.json"

# Grab SPDX specs
curl https://spdx.github.io/spdx-spec/v3.0.1/rdf/schema.json -o "$SCHEMA_DIR/spdx-bom-3.0.1.schema.json"
curl https://spdx.github.io/spdx-spec/v3.0.0/rdf/schema.json -o "$SCHEMA_DIR/spdx-bom-3.0.0.schema.json"
curl https://raw.githubusercontent.com/spdx/spdx-spec/support/2.3/schemas/spdx-schema.json -o "$SCHEMA_DIR/spdx-bom-2.3.schema.json"