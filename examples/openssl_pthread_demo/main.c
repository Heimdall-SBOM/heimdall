/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>

#define NUM_THREADS 3
#define BUFFER_SIZE 1024

// Thread function that performs OpenSSL operations
void* openssl_worker(void* arg)
{
    int thread_id = *(int*)arg;
    printf("Thread %d: Starting OpenSSL operations\n", thread_id);
    
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // Create SSL context
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        printf("Thread %d: Failed to create SSL context\n", thread_id);
        return NULL;
    }
    
    // Perform some cryptographic operations
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (md_ctx) {
        EVP_DigestInit_ex(md_ctx, EVP_sha256(), NULL);
        
        char data[BUFFER_SIZE];
        snprintf(data, sizeof(data), "Thread %d data for hashing", thread_id);
        
        EVP_DigestUpdate(md_ctx, data, strlen(data));
        
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;
        EVP_DigestFinal_ex(md_ctx, hash, &hash_len);
        
        printf("Thread %d: Generated SHA256 hash (first 16 bytes): ", thread_id);
        for (int i = 0; i < 16 && i < hash_len; i++) {
            printf("%02x", hash[i]);
        }
        printf("\n");
        
        EVP_MD_CTX_free(md_ctx);
    }
    
    // Cleanup
    SSL_CTX_free(ctx);
    EVP_cleanup();
    ERR_free_strings();
    
    printf("Thread %d: OpenSSL operations completed\n", thread_id);
    return NULL;
}

// Thread function that performs pthread operations
void* pthread_worker(void* arg)
{
    int thread_id = *(int*)arg;
    printf("Thread %d: Starting pthread operations\n", thread_id);
    
    // Create a mutex
    pthread_mutex_t mutex;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Thread %d: Failed to initialize mutex\n", thread_id);
        return NULL;
    }
    
    // Lock and unlock the mutex
    pthread_mutex_lock(&mutex);
    printf("Thread %d: Mutex locked\n", thread_id);
    
    // Simulate some work
    for (int i = 0; i < 1000000; i++) {
        // Busy work
    }
    
    pthread_mutex_unlock(&mutex);
    printf("Thread %d: Mutex unlocked\n", thread_id);
    
    // Cleanup
    pthread_mutex_destroy(&mutex);
    
    printf("Thread %d: Pthread operations completed\n", thread_id);
    return NULL;
}

int main()
{
    printf("🔧 Heimdall OpenSSL + Pthreads Demo\n");
    printf("====================================\n");
    printf("This program demonstrates OpenSSL and pthreads usage\n");
    printf("to show how dependencies appear in the SBOM.\n\n");
    
    // Show OpenSSL version
    printf("📋 OpenSSL Version: %s\n", OpenSSL_version(OPENSSL_VERSION));
    printf("📋 OpenSSL Build Date: %s\n", OpenSSL_version(OPENSSL_BUILT_ON));
    printf("📋 OpenSSL Platform: %s\n", OpenSSL_version(OPENSSL_PLATFORM));
    printf("📋 OpenSSL Directory: %s\n", OpenSSL_version(OPENSSL_DIR));
    printf("\n");
    
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int rc;
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        
        if (i % 2 == 0) {
            // Even threads do OpenSSL work
            rc = pthread_create(&threads[i], NULL, openssl_worker, &thread_ids[i]);
        } else {
            // Odd threads do pthread work
            rc = pthread_create(&threads[i], NULL, pthread_worker, &thread_ids[i]);
        }
        
        if (rc) {
            printf("Error: Unable to create thread %d\n", i);
            exit(-1);
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n✅ All threads completed successfully!\n");
    printf("📋 This program uses:\n");
    printf("   - OpenSSL for cryptographic operations\n");
    printf("   - Pthreads for multi-threading\n");
    printf("   - System libraries for I/O and memory management\n");
    printf("\n🔍 Check the generated SBOM to see how these dependencies are documented.\n");
    
    return 0;
} 