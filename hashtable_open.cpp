#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const size_t MIN_ARRAY_SIZE = 10000;
const size_t MAX_ARRAY_SIZE = 1000000;
const size_t STEP = 10000;
const size_t TEST_COUNT = 2;


const size_t TABLE_INIT_SIZE = 128;


#define ASSERT(condition, exitcode, ...)            \
do {                                                \
    if (!(condition)) {                             \
        printf(__VA_ARGS__);                        \
        return exitcode;                            \
    }                                               \
} while (0)


typedef unsigned long long hash_t;
typedef int data_t;
typedef int okey_t;


typedef struct Node {
    okey_t key;
    data_t data;
    char status;
} Node;


typedef struct {
    size_t count;
    size_t size;
    Node *buckets ;
} HashTable;


int hashtable_constructor(HashTable *table, size_t size);

int hashtable_insert(HashTable *table, okey_t new_key, data_t new_data);

int hashtable_find(HashTable *table, okey_t key, data_t *data);

int hashtable_remove(HashTable *table, okey_t key);

int hashtable_destructor(HashTable *table);

int hashtable_rehash(HashTable *table);




int main() {
    srand(time(NULL));

    int data = 0;

    FILE *file = fopen("result2.txt", "w");

    for (size_t size = MIN_ARRAY_SIZE; size <= MAX_ARRAY_SIZE; size += STEP) {
        double result_time = 0.0;

        for (size_t test = 0; test < TEST_COUNT; test++) {
            HashTable table = {0, 0, NULL};
            hashtable_constructor(&table, TABLE_INIT_SIZE);

            clock_t t1 = clock();

            for (size_t i = 0; i < size; i++) {
                char op = rand() % 3;
                int arg = rand();

                switch (op) {
                    case 0: hashtable_insert(&table, arg, arg); break;
                    case 1: hashtable_find(&table, arg, &data); break;
                    case 2: hashtable_remove(&table, arg); break;
                }
            }

            clock_t t2 = clock();

            result_time += 1000.0 * (t2 - t1) / CLOCKS_PER_SEC;

            hashtable_destructor(&table);
        }

        // printf("Size: %lu, Time: %lf\n", size, result_time / TEST_COUNT);
        fprintf(file, "%lf\n", result_time / TEST_COUNT);
    }

    fclose(file);

    return 0;
}




inline hash_t hash1(HashTable *table, okey_t key);

inline hash_t hash2(HashTable *table, okey_t key);

Node *find_node(HashTable *table, okey_t key);


int hashtable_constructor(HashTable *table, size_t size) {
    ASSERT(table, 1, "Can't construct null table!\n");

    table -> buckets = (Node *) calloc(size, sizeof(Node));
    ASSERT(table -> buckets, 1, "Can't allocate buckets for table!\n");
    
    table -> count = 0;
    table -> size = size;

    return 0;
}


int hashtable_insert(HashTable *table, okey_t new_key, data_t new_data) {
    ASSERT(table, 1, "Can't insert in null table!\n");

    hash_t i = hash1(table, new_key);
    hash_t k = hash2(table, new_key);
    
    for (; table -> buckets[i].status == 1; i = (i + k) % table -> size) {
        if (table -> buckets[i].key == new_key) {
            table -> buckets[i].data = new_data;
            return 0;
        }
    }

    table -> buckets[i].key = new_key;
    table -> buckets[i].data = new_data;
    table -> buckets[i].status = 1;

    table -> count++;

    return hashtable_rehash(table);
}


int hashtable_find(HashTable *table, okey_t key, data_t *data) {
    ASSERT(table, 1, "Can't search in null table!\n");
    ASSERT(data, 1, "Can't put search result in null data!\n");

    Node *node = find_node(table, key);

    if (node) {
        *data = node -> data;
        return 0;
    }
    
    *data = 0;
    return -1;
}


int hashtable_remove(HashTable *table, okey_t key) {
    ASSERT(table, 1, "Can't remove in null table!\n");

    Node *node = find_node(table, key);

    if (node) {
        node -> key = 0;
        node -> data = 0;
        node -> status = -1;

        table -> count--;
    }

    return 0;
}


int hashtable_destructor(HashTable *table) {
    ASSERT(table, 1, "Can't destruct null table!\n");

    free(table -> buckets);
    table -> buckets = NULL;

    table -> size = 0;
    table -> count = 0;

    return 0;
}


int hashtable_rehash(HashTable *table) {
    if (table -> count * 2 < table -> size) return 0;

    size_t new_size = table -> size * 2;

    Node *new_buckets = (Node *) calloc(new_size, sizeof(Node));
    ASSERT(new_buckets, 1, "Couldn't allocate buffer for new buckets!\n");

    Node *old_buckets = table -> buckets;
    size_t old_size = table -> size;

    table -> buckets = new_buckets;
    table -> size = new_size;
    table -> count = 0;

    for (size_t i = 0; i < old_size; i++) {
        if (old_buckets[i].status == 1)
            hashtable_insert(table, old_buckets[i].key, old_buckets[i].data);
    }

    free(old_buckets);

    return 0;
}


inline hash_t hash1(HashTable *table, okey_t key) {
    return key % (okey_t) (table -> size - 1);
}


inline hash_t hash2(HashTable *table, okey_t key) {
    hash_t hash = key % (okey_t) (table -> size - 3) + 1;
    return hash % 2 ? hash : hash + 1;
}


Node *find_node(HashTable *table, okey_t key) {
    hash_t i = hash1(table, key);
    hash_t k = hash2(table, key);

    for (; table -> buckets[i].status; i = (i + k) % table -> size) {
        if (table -> buckets[i].status == 1 && table -> buckets[i].key == key) 
            return table -> buckets + i;
    }

    return NULL;
}
