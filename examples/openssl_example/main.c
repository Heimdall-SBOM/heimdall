#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string_to_hash>\n", argv[0]);
        return 1;
    }

    const char *input = argv[1];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    // Initialize SHA256 context
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sizeof hash);
    SHA256_Final(hash, &sha256);

    printf("SHA256 hash of '%s':\n", input);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    return 0;
} 
