#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const size_t MIN_ARRAY_SIZE = 10000;
const size_t MAX_ARRAY_SIZE = 1000000;
const size_t STEP = 10000;
const size_t TEST_COUNT = 5;


const size_t TABLE_INIT_SIZE = 109;


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
    struct Node *next;
} Node;


typedef struct {
    size_t count;
    size_t size;
    Node **buckets ;
} HashTable;


int hashtable_constructor(HashTable *table, size_t size);

int hashtable_insert(HashTable *table, okey_t new_key, data_t new_data);

int hashtable_find(HashTable *table, okey_t key, data_t *data);

int hashtable_remove(HashTable *table, okey_t key);

int hashtable_destructor(HashTable *table);

int hashtable_dump(HashTable *table, FILE *stream);

int hashtable_rehash(HashTable *table);




int main() {
    srand(time(NULL));

    int data = 0;

    FILE *file = fopen("result.txt", "w");

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




Node *create_node(okey_t key, data_t data);

void free_node(Node *node);

inline Node *get_list(HashTable *table, okey_t key);

void free_list(Node *node);

inline hash_t get_hash(HashTable *table, okey_t key);

Node *find_node(Node *begin, okey_t key, Node **prev);




int hashtable_constructor(HashTable *table, size_t size) {
    ASSERT(table, 1, "Can't construct null table!\n");

    table -> buckets = (Node **) calloc(size, sizeof(Node *));
    ASSERT(table -> buckets, 1, "Can't allocate buckets for table!\n");
    
    table -> count = 0;
    table -> size = size;

    return 0;
}


int hashtable_insert(HashTable *table, okey_t new_key, data_t new_data) {
    ASSERT(table, 1, "Can't insert in null table!\n");

    hash_t hash = get_hash(table, new_key);

    if (!table -> buckets[hash]) {
        table -> buckets[hash] = create_node(new_key, new_data);
        ASSERT(table -> buckets[hash], 1, "Failed to alloc new node!\n");

        table -> count++;
    }
    else {
        Node *node = find_node(table -> buckets[hash], new_key, NULL);

        if (node) {
            node -> data = new_data;
            return 0;
        }
        else {
            node = table -> buckets[hash];

            table -> buckets[hash] = create_node(new_key, new_data);
            ASSERT(table -> buckets[hash], 1, "Failed to alloc new node!\n");

            table -> buckets[hash] -> next = node;

            table -> count++;
        }
    }

    return hashtable_rehash(table);
}


int hashtable_find(HashTable *table, okey_t key, data_t *data) {
    ASSERT(table, 1, "Can't search in null table!\n");
    ASSERT(data, 1, "Can't put search result in null data!\n");

    Node *node = find_node(get_list(table, key), key, NULL);

    if (node) {
        *data = node -> data;
        return 0;
    }
    
    *data = 0;
    return -1;
}


int hashtable_remove(HashTable *table, okey_t key) {
    ASSERT(table, 1, "Can't remove in null table!\n");

    Node *node = NULL, *prev = NULL;
    node = find_node(get_list(table, key), key, &prev);

    if (node) {
        if (prev) prev -> next = node -> next;
        else table -> buckets[get_hash(table, key)] = node -> next;

        free_node(node);
        table -> count--;
    }

    return 0;
}


int hashtable_destructor(HashTable *table) {
    ASSERT(table, 1, "Can't destruct null table!\n");

    for (size_t i = 0; i < table -> size; i++) free_list(table -> buckets[i]);

    free(table -> buckets);
    table -> buckets = NULL;

    table -> size = 0;
    table -> count = 0;

    return 0;
}


Node *create_node(okey_t key, data_t data) {
    Node *new_node = (Node *) calloc(1, sizeof(Node));

    if (new_node) {
        new_node -> key = key;
        new_node -> data = data;
        new_node -> next = NULL;
    }

    return new_node;
}


void free_node(Node *node) {
    if (!node) return;

    free(node);
}


inline hash_t get_hash(HashTable *table, okey_t key) {
    return (hash_t) key % (hash_t) table -> size;
}


inline Node *get_list(HashTable *table, okey_t key) {
    return table -> buckets[get_hash(table, key)];
}


void free_list(Node *node) {
    if (node) {
        free_list(node -> next);
        free_node(node);
    }
}


Node *find_node(Node *begin, okey_t key, Node **prev) {
    if (prev) *prev = NULL;

    if (!begin) return NULL;
    
    for (; begin && (begin -> key != key); begin = begin -> next) {
        if (prev) *prev = begin;
    }

    return begin;
}


int hashtable_dump(HashTable *table, FILE *stream) {
    fprintf(stream, "Hash Table [%p]\n", table);
    fprintf(stream, "Buffer size: %lu\n", table -> size);
    fprintf(stream, "Buffer:\n");

    for (size_t i = 0; i < table -> size; i++) {
        fprintf(stream, "  %5lu: ", i);

        for (Node *node = table -> buckets[i]; node; node = node -> next) {
            fprintf(stream, "(%i, %i)", node -> key, node -> data);

            if (node -> next) fprintf(stream, " -> ");
        }

        fputc('\n', stream);
    }

    fputc('\n', stream);
    return 0;
}


int hashtable_rehash(HashTable *table) {
    if (table -> count <= table -> size * 4) return 0;

    size_t new_size = table -> size * 2;

    Node **new_buckets = (Node **) calloc(new_size, sizeof(Node *));
    ASSERT(new_buckets, 1, "Couldn't allocate buffer for new buckets!\n");

    Node **old_buckets = table -> buckets;
    size_t old_size = table -> size;

    table -> buckets = new_buckets;
    table -> size = new_size;
    table -> count = 0;

    for (size_t i = 0; i < old_size; i++) {
        Node *node = old_buckets[i];

        while (node) {
            hashtable_insert(table, node -> key, node -> data);

            Node *prev = node;

            node = node -> next;

            free_node(prev);
        }
    }

    free(old_buckets);

    return 0;
}
