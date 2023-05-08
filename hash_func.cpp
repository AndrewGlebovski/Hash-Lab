#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>


const size_t BUFFER_SIZE = 1000;
const size_t ITERATIONS = 1000000;


#define KEY_TYPE 2


#if (KEY_TYPE == 0)
typedef int okey_t;
const int MAX_INT = INT_MAX;
#elif (KEY_TYPE == 1)
typedef float okey_t;
const float MAX_FLOAT = 50000.0f;
#elif (KEY_TYPE == 2)
typedef char *okey_t;
const size_t MAX_LENGTH = 250;
#endif

typedef unsigned long long hash_t;

typedef hash_t (*hash_func_t)(okey_t key);

typedef struct {
    hash_func_t func = nullptr;
    const char *name = nullptr;
} HashFuncInfo;


void test_func(FILE *csv_file, HashFuncInfo *info);

okey_t key_generator();


hash_t hash_mod(int key) {
    return key;
}


hash_t hash_bin(int key) {
    return key;
}


hash_t hash_mul(int key) {
    float f = ((float) rand() / (float) RAND_MAX) * key;
    f = f - (int) f;
    return BUFFER_SIZE * f;
}


hash_t hash_int(float key) {
    return (int) key;
}


hash_t hash_bin(float key) {
    union Float2Int {
        float f;
        unsigned int i;
    };
    Float2Int f2l = {key};
    return f2l.i;
}


hash_t hash_len(char *key) {
    hash_t len = 0;
    for (; *key; key++) len++;
    return len;
}


hash_t hash_charsum(char *key) {
    hash_t sum = 0;
    for (; *key; key++) sum += *key;
    return sum;
}


hash_t poly_hash(char *key) {
    hash_t sum = 0;
    for (; *key; key++) sum = sum * 7 + *key; // 7 is a magic number!
    return sum;
}


hash_t crc32(char *key) {
    hash_t sum1 = 1, sum2 = 0;
    for (; *key; key++) {
        sum1 = (sum1 + *key) % (1ULL << 32ULL);
        sum2 = (sum1 + sum2) % (1ULL << 32ULL);
    }
    return (sum2 << 32) + sum1;
}


int main() {
    srand(time(NULL));

#if (KEY_TYPE == 0)
    HashFuncInfo func_list[] = {
        {&hash_mod, "mod"},
        {&hash_bin, "bin"},
        {&hash_mul, "mul"},
    };
#elif (KEY_TYPE == 1)
    HashFuncInfo func_list[] = {
        {&hash_int, "int"},
        {&hash_bin, "bin"},
    };
#elif (KEY_TYPE == 2)
    HashFuncInfo func_list[] = {
        {&hash_len,     "len"},
        {&hash_charsum, "charsum"},
        {&poly_hash,    "poly hash"},
        {&crc32,        "crc32"},
    };
#endif

    FILE *csv_file = fopen("result.csv", "a");
    
    for (size_t i = 0; i < BUFFER_SIZE; i++)
        fprintf(csv_file, ", %lu", i);

    fputc('\n', csv_file);

    for (size_t func = 0; func < sizeof(func_list) / sizeof(HashFuncInfo); func++)
        test_func(csv_file, func_list + func);

    fclose(csv_file);

    printf("Hash-Function!\n");

    return 0;
}


void test_func(FILE *csv_file, HashFuncInfo *info) {
    hash_t *buffer = (hash_t *) calloc(BUFFER_SIZE, sizeof(hash_t));

    for (size_t i = 0; i < ITERATIONS; i++) {
        okey_t key = key_generator();
        (buffer[info -> func(key) % BUFFER_SIZE])++;

#if (KEY_TYPE == 2)
        free(key);
#endif
    }

    fprintf(csv_file, "%s", info -> name);

    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        fprintf(csv_file, ",  %llu", buffer[i]);
    }

    fputc('\n', csv_file);
}


okey_t key_generator() {
#if (KEY_TYPE == 0)
    return rand();
#elif (KEY_TYPE == 1)
    return ((float) rand() / (float) RAND_MAX) * MAX_FLOAT;
#elif (KEY_TYPE == 2)
    size_t len = rand() + 5;
    if (len > MAX_LENGTH) len = MAX_LENGTH;
    char *key = (char *) calloc(len + 1, sizeof(char));
    for (size_t i = 0; i < len; i++) key[i] = rand() % 256;
    key[len] = '\0';
    return key;
#endif
}
