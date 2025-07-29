#!/bin/bash

# Generate RSA private key (2048-bit, recommended for RS256)
openssl genrsa -out private_rsa.key 2048

# Generate RSA private key (4096-bit, recommended for RS384/RS512)
openssl genrsa -out private_rsa_4096.key 4096

# Extract public key from private key
openssl rsa -in private_rsa.key -pubout -out public_rsa.key
