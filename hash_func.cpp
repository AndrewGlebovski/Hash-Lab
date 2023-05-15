#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <assert.h>


const size_t BUFFER_SIZE = 109;
const size_t ITERATIONS = 1000000;


#define KEY_TYPE 0


#if (KEY_TYPE == 0)
typedef int okey_t;
const int MAX_INT = INT_MAX;
#elif (KEY_TYPE == 1)
typedef float okey_t;
const float MAX_FLOAT = 1.0e+9f;
#elif (KEY_TYPE == 2)
typedef char *okey_t;
const size_t MAX_LENGTH = 1009;
#endif

typedef unsigned long long hash_t;

typedef hash_t (*hash_func_t)(okey_t key);

typedef struct {
    hash_func_t func = nullptr;
    const char *name = nullptr;
} HashFuncInfo;


void test_func(FILE *file, okey_t *arr, HashFuncInfo *info);

okey_t key_generator();


hash_t hash_mod(int key) {
    return key;
}


hash_t hash_bin(int key) {
    return key;
}


hash_t hash_mul(int key) {
    double num = key;
    num *= 0.6180339887;
    num = num - (int) num;
    return (1 << 12) * num;
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
        {&hash_mod, "mod.txt"},
        {&hash_bin, "bin.txt"},
        {&hash_mul, "mul.txt"},
    };
#elif (KEY_TYPE == 1)
    HashFuncInfo func_list[] = {
        {&hash_int, "int.txt"},
        {&hash_bin, "bin.txt"},
    };
#elif (KEY_TYPE == 2)
    HashFuncInfo func_list[] = {
        {&hash_len,     "len.txt"},
        {&hash_charsum, "charsum.txt"},
        {&poly_hash,    "poly_hash.txt"},
        {&crc32,        "crc32.txt"},
    };
#endif
    
    okey_t *arr = (okey_t *) calloc(ITERATIONS, sizeof(okey_t));
    assert(arr && "Can't allocate buffer!\n");

    for (size_t i = 0; i < ITERATIONS; i++) arr[i] = key_generator();

    for (size_t func = 0; func < sizeof(func_list) / sizeof(HashFuncInfo); func++) {
        FILE *file = fopen(func_list[func].name, "w");
        assert(file && "Can't open file!\n");

        test_func(file, arr, func_list + func);

        fclose(file);
    }

#if (KEY_TYPE == 2)
    for (size_t i = 0; i < ITERATIONS; i++) free(arr[i]);
#endif

    free(arr);

    printf("Hash-Function!\n");

    return 0;
}


void test_func(FILE *file, okey_t *arr, HashFuncInfo *info) {
    hash_t *buffer = (hash_t *) calloc(BUFFER_SIZE, sizeof(hash_t));
    assert(buffer && "Can't allocate buffer!\n");

    for (size_t i = 0; i < ITERATIONS; i++)
        (buffer[info -> func(arr[i]) % BUFFER_SIZE])++;

    for (size_t i = 0; i < BUFFER_SIZE; i++)
        fprintf(file, "%llu\n", buffer[i]);
}


okey_t key_generator() {
#if (KEY_TYPE == 0)
    return rand();
#elif (KEY_TYPE == 1)
    return ((float) rand() / (float) RAND_MAX) * MAX_FLOAT;
#elif (KEY_TYPE == 2)
    size_t len = rand() % (MAX_LENGTH - 1) + 1;
    char *key = (char *) calloc(len + 1, sizeof(char));
    for (size_t i = 0; i < len; i++) key[i] = rand() % 26 + 'a';
    key[len] = '\0';
    return key;
#endif
}
