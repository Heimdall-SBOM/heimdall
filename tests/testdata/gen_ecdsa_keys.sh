#!/usr/bin

# Generate ECDSA private key (P-256 curve for ES256)
openssl ecparam -genkey -name prime256v1 -out private_ecdsa.key

# Generate ECDSA private key (P-384 curve for ES384)
openssl ecparam -genkey -name secp384r1 -out private_ecdsa_384.key

# Generate ECDSA private key (P-521 curve for ES512)
openssl ecparam -genkey -name secp521r1 -out private_ecdsa_512.key

# Extract public key from private key
openssl ec -in private_ecdsa.key -pubout -out public_ecdsa.key
