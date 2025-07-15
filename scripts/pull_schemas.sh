#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Define the schema directory relative to the script's location
SCHEMA_DIR="$SCRIPT_DIR/schema"

# Grab CycloneDX specs
curl https://cyclonedx.org/schema/bom-1.6.schema.json > "$SCHEMA_DIR/cyclonedx-bom-1.6.schema.json"
curl https://cyclonedx.org/schema/bom-1.5.schema.json > "$SCHEMA_DIR/cyclonedx-bom-1.5.schema.json"
curl https://cyclonedx.org/schema/bom-1.4.schema.json > "$SCHEMA_DIR/cyclonedx-bom-1.4.schema.json"

# Grab SPDX specs
curl https://spdx.github.io/spdx-spec/v3.0.1/rdf/schema.json > "$SCHEMA_DIR/spdx-bom-3.0.1.schema.json"
curl https://spdx.github.io/spdx-spec/v3.0.0/rdf/schema.json > "$SCHEMA_DIR/spdx-bom-3.0.0.schema.json"
curl https://raw.githubusercontent.com/spdx/spdx-spec/support/2.3/schemas/spdx-schema.json > "$SCHEMA_DIR/spdx-bom-2.3.schema.json"
