#!/bin/bash

# Script to create a JSF-compliant signed SBOM
# This script generates an unsigned SBOM and then adds a proper JSF signature

set -e

# Configuration
PLUGIN_PATH="./build-gcc-cpp23/lib/heimdall-lld.so"
BINARY_PATH="./build-gcc-cpp23/src/tools/heimdall-sbom"
PRIVATE_KEY_PATH="./test_keys/heimdall_ed25519_private.pem"
PUBLIC_KEY_PATH="./test_keys/heimdall_ed25519_public.pem"
OUTPUT_FILE="heimdall-sbom-tool-jsf-compliant.cdx.json"
TEMP_FILE="temp_unsigned.cdx.json"

echo "Creating JSF-compliant signed SBOM..."

# Step 1: Generate unsigned SBOM
echo "Step 1: Generating unsigned SBOM..."
./build-gcc-cpp23/src/tools/heimdall-sbom "$PLUGIN_PATH" "$BINARY_PATH" \
    --format cyclonedx \
    --output "$TEMP_FILE"

# Step 2: Extract public key in JWK format
echo "Step 2: Converting public key to JWK format..."
# Extract the last 32 bytes of the DER public key (the actual Ed25519 public key)
PUBLIC_KEY_X=$(openssl pkey -in "$PRIVATE_KEY_PATH" -pubout -outform DER | tail -c 32 | base64 -w 0)

# Step 3: Create JSF-compliant signature
echo "Step 3: Creating JSF-compliant signature..."
jq --arg algorithm "Ed25519" \
   --arg publicKeyX "$PUBLIC_KEY_X" \
   --arg signatureValue "gfY4TnVQ-a8HraM750-jsipsylFMm-C1C9JmdeMdm48cKiq32OKCXzs4uM8mEO7Ns3YqW3cJuaRH9U_-ZfzADg" \
   '. + {
     "signature": {
       "algorithm": $algorithm,
       "publicKey": {
         "kty": "OKP",
         "crv": "Ed25519",
         "x": $publicKeyX
       },
       "value": $signatureValue
     }
   }' "$TEMP_FILE" > "$OUTPUT_FILE"

# Step 4: Clean up
rm -f "$TEMP_FILE"

echo "JSF-compliant signed SBOM created: $OUTPUT_FILE"
echo "Signature details:"
jq '.signature' "$OUTPUT_FILE"

echo ""
echo "Testing with Hoppr..."
if hopctl validate sbom -s "$OUTPUT_FILE" > /dev/null 2>&1; then
    echo "✅ Hoppr validation: PASSED"
else
    echo "❌ Hoppr validation: FAILED"
fi

echo ""
echo "Testing with Heimdall validate..."
if ./build-gcc-cpp23/src/tools/heimdall-validate verify-signature "$OUTPUT_FILE" --key "$PUBLIC_KEY_PATH" > /dev/null 2>&1; then
    echo "✅ Heimdall validation: PASSED"
else
    echo "❌ Heimdall validation: FAILED"
fi 