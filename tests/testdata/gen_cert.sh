#!/usr/bin

# Generate RSA private key
openssl genrsa -out private_key.pem 2048

# Generate self-signed certificate from the private key
openssl req -new -x509 -key private_key.pem -out certificate.pem -days 365 -subj "/CN=Heimdall Test Certificate/O=Heimdall Project/C=US"

# Extract public key from private key (for verification)
openssl rsa -in private_key.pem -pubout -out public_key.pem

# View certificate details
openssl x509 -in certificate.pem -text -noout | head -20


