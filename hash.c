#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>

#define NUM_THREADS 2
#define SHA512_LENGTH 64
#define SHA256_LENGTH 32

/*
Generates an SHA512 hash using string "word"
INPUTS: string "word"
OUTPUTS: generated SHA512 hash
*/
void *sha512_hash_generator(void* word){
    pthread_t self_id = pthread_self();
    char* converted_word = (char*) word; //Casts word to char for correct usage

    //Creates and assigns SHA512 to completed hash variable
    unsigned char* SHA512_completed_hash = malloc(sizeof(unsigned char[SHA512_LENGTH]));
    SHA512(converted_word, strlen(converted_word), SHA512_completed_hash);

    printf("Thread 1: tid %ld\n", self_id);
    pthread_exit((void*) SHA512_completed_hash);
}

/*
Generates an SHA256 hash using string "word"
INPUTS: string "word"
OUTPUTS: generated SHA256 hash
*/
void *SHA256_hash_generator(void* word){
    pthread_t self_id = pthread_self();
    char* converted_word = (char*) word; //Casts word to char for correct usage

    //Creates and assigns SHA256 to completed hash variable
    unsigned char* SHA256_completed_hash = malloc(sizeof(unsigned char[SHA256_LENGTH]));
    SHA256(converted_word, strlen(converted_word), SHA256_completed_hash);

    printf("Thread 2: tid %ld\n", self_id);
    pthread_exit((void*) SHA256_completed_hash);
}

int main (int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS];

    //Stores "word"
    char* word = argv[1];

    void* SHA512_ret;
    void* SHA256_ret;

    pthread_create(&threads[0], NULL, sha512_hash_generator, (void *) word);
    pthread_create(&threads[1], NULL, SHA256_hash_generator, (void *) word);

    //SHA512 hash thread joining
    pthread_join(threads[0], (void*)&SHA512_ret);
    unsigned char* SHA512_hash = (unsigned char*) SHA512_ret; //Cast correctly for use
    printf("SHA512 of \"%s\" calculated by thread %lu is %i bytes long:\n", word, threads[0], SHA512_LENGTH);
    for (int i = 0; i < SHA512_LENGTH; i++) {
        printf("%02x", SHA512_hash[i]); //Prints each byte as hex
    }
    printf("\n");
    free(SHA512_hash);

    //SHA256 hash thread joining
    pthread_join(threads[1], (void*)&SHA256_ret);
    unsigned char* SHA256_hash = (unsigned char*) SHA256_ret; //Cast correctly for use
    printf("SHA256 of \"%s\" calculated by thread %lu is %i bytes long:\n", word, threads[1], SHA256_LENGTH);
    for (int i = 0; i < SHA256_LENGTH; i++) {
        printf("%02x", SHA256_hash[i]); //Prints each byte as hex
    }
    printf("\n");
    free(SHA256_hash);

    return 0;
}
