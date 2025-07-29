#!/usr/bin

# Generate Ed25519 private key
openssl genpkey -algorithm ed25519 -out private_ed25519.key

# Extract public key from private key
openssl pkey -in private_ed25519.key -pubout -out public_ed25519.key
