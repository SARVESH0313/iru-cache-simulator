#include <stdio.h>
#include <stdlib.h>

#define TABLE_SIZE 101

typedef struct Node {
    int key;
    int value;
    struct Node *prev;
    struct Node *next;
    struct Node *hash_next;
} Node;

typedef struct {
    Node *buckets[TABLE_SIZE];
} HashTable;

typedef struct {
    int capacity;
    int size;
    unsigned long hits;
    unsigned long misses;
    unsigned long evictions;
    Node *most_recent;
    Node *least_recent;
    HashTable table;
} LRUCache;

static unsigned int hash_key(int key) {
    return (unsigned int)key % TABLE_SIZE;
}

static Node *hash_find(HashTable *table, int key) {
    Node *current = table->buckets[hash_key(key)];
    while (current != NULL) {
        if (current->key == key) {
            return current;
        }
        current = current->hash_next;
    }
    return NULL;
}

static void hash_insert(HashTable *table, Node *node) {
    unsigned int index = hash_key(node->key);
    node->hash_next = table->buckets[index];
    table->buckets[index] = node;
}

static void hash_remove(HashTable *table, int key) {
    unsigned int index = hash_key(key);
    Node *current = table->buckets[index];
    Node *previous = NULL;

    while (current != NULL) {
        if (current->key == key) {
            if (previous == NULL) {
                table->buckets[index] = current->hash_next;
            } else {
                previous->hash_next = current->hash_next;
            }
            current->hash_next = NULL;
            return;
        }
        previous = current;
        current = current->hash_next;
    }
}

static void detach_node(LRUCache *cache, Node *node) {
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        cache->most_recent = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        cache->least_recent = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

static void move_to_front(LRUCache *cache, Node *node) {
    if (cache->most_recent == node) {
        return;
    }

    if (node->prev != NULL || node->next != NULL ||
        cache->least_recent == node) {
        detach_node(cache, node);
    }

    node->prev = NULL;
    node->next = cache->most_recent;

    if (cache->most_recent != NULL) {
        cache->most_recent->prev = node;
    } else {
        cache->least_recent = node;
    }

    cache->most_recent = node;
}

static int cache_init(LRUCache *cache, int capacity) {
    int i;

    if (capacity < 1) {
        return 0;
    }

    cache->capacity = capacity;
    cache->size = 0;
    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    cache->most_recent = NULL;
    cache->least_recent = NULL;

    for (i = 0; i < TABLE_SIZE; i++) {
        cache->table.buckets[i] = NULL;
    }

    return 1;
}

/* Access an existing key or insert a new key/value pair. */
static void cache_put(LRUCache *cache, int key, int value) {
    Node *node = hash_find(&cache->table, key);

    if (node != NULL) {
        cache->hits++;
        node->value = value;
        move_to_front(cache, node);
        printf("Cache HIT: key %d updated and moved to most-recent position.\n",
               key);
        return;
    }

    cache->misses++;

    if (cache->size == cache->capacity) {
        Node *victim = cache->least_recent;
        printf("Cache MISS: cache is full; evicting key %d.\n", victim->key);

        hash_remove(&cache->table, victim->key);
        detach_node(cache, victim);
        free(victim);
        cache->size--;
        cache->evictions++;
    } else {
        printf("Cache MISS: inserting new key %d.\n", key);
    }

    node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return;
    }

    node->key = key;
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    node->hash_next = NULL;

    move_to_front(cache, node);
    hash_insert(&cache->table, node);
    cache->size++;
}

static void cache_get(LRUCache *cache, int key) {
    Node *node = hash_find(&cache->table, key);

    if (node == NULL) {
        cache->misses++;
        printf("Cache MISS: key %d is not present.\n", key);
        return;
    }

    cache->hits++;
    move_to_front(cache, node);
    printf("Cache HIT: key %d has value %d and is now most recently used.\n",
           key, node->value);
}

static void cache_display(const LRUCache *cache) {
    const Node *current = cache->most_recent;

    if (current == NULL) {
        printf("Cache is empty.\n");
        return;
    }

    printf("Cache (most recent -> least recent):\n");
    while (current != NULL) {
        printf("[key=%d, value=%d]", current->key, current->value);
        if (current->next != NULL) {
            printf(" <-> ");
        }
        current = current->next;
    }
    printf("\n");
}

static void cache_stats(const LRUCache *cache) {
    printf("\n--- Cache Statistics ---\n");
    printf("Capacity : %d\n", cache->capacity);
    printf("Items    : %d\n", cache->size);
    printf("Hits     : %lu\n", cache->hits);
    printf("Misses   : %lu\n", cache->misses);
    printf("Evictions: %lu\n", cache->evictions);
    printf("------------------------\n");
}

static void cache_destroy(LRUCache *cache) {
    Node *current = cache->most_recent;

    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }

    cache->most_recent = NULL;
    cache->least_recent = NULL;
    cache->size = 0;
}

static int read_int(const char *prompt, int *result) {
    char buffer[128];
    char extra;
    long value;

    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    if (sscanf(buffer, "%ld %c", &value, &extra) != 1 ||
        value < -2147483647L - 1L || value > 2147483647L) {
        printf("Invalid input. Please enter an integer.\n");
        return -1;
    }

    *result = (int)value;
    return 1;
}

int main(void) {
    LRUCache cache;
    int capacity;
    int status;
    int choice;

    printf("=================================\n");
    printf("       LRU CACHE SIMULATOR\n");
    printf("=================================\n");

    do {
        status = read_int("Enter cache capacity (1 or more): ", &capacity);
        if (status == 0) {
            return 0;
        }
    } while (status < 0 || capacity < 1);

    if (!cache_init(&cache, capacity)) {
        printf("Could not initialize cache.\n");
        return 1;
    }

    while (1) {
        printf("\n----------- MENU -----------\n");
        printf("1. Insert or update key/value\n");
        printf("2. Access (get) a key\n");
        printf("3. Display cache\n");
        printf("4. Show statistics\n");
        printf("5. Exit\n");
        printf("----------------------------\n");

        status = read_int("Enter choice: ", &choice);
        if (status == 0) {
            break;
        }
        if (status < 0) {
            continue;
        }

        if (choice == 1) {
            int key, value;
            status = read_int("Enter integer key: ", &key);
            if (status == 0) break;
            if (status < 0) continue;

            status = read_int("Enter integer value: ", &value);
            if (status == 0) break;
            if (status < 0) continue;

            cache_put(&cache, key, value);
        } else if (choice == 2) {
            int key;
            status = read_int("Enter key to access: ", &key);
            if (status == 0) break;
            if (status < 0) continue;

            cache_get(&cache, key);
        } else if (choice == 3) {
            cache_display(&cache);
        } else if (choice == 4) {
            cache_stats(&cache);
        } else if (choice == 5) {
            break;
        } else {
            printf("Invalid choice. Select 1-5.\n");
        }
    }

    cache_destroy(&cache);
    printf("Cache memory released. Goodbye!\n");
    return 0;
}
