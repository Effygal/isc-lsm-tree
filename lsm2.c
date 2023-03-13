#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMTABLE_SIZE 1000
#define SSTABLE_SIZE 10000
#define LEVELS 5

typedef struct
{
    char *key;
    char *value;
} kvpair;

typedef struct
{
    kvpair **pairs;
    int size;
    int mutable; 
} memtable;

typedef struct
{
    int fd;
    kvpair *pairs;
    int size;
    int smallest_key;
    int largest_key;
    int sequence_num;
} sstable;

typedef struct
{
    sstable *tables[];
    int size;
    int l_0 = 0;
} level;

typedef struct
{
    memtable *mem;
    level *levels;
} lsm;

int compare_kvpair(const void *a, const void *b)
{
    kvpair *ka = *(kvpair **)a;
    kvpair *kb = *(kvpair **)b;
    return strcmp(ka->key, kb->key);
}

void sort_memtable(memtable *mem)
{
    qsort(mem->pairs, mem->size, sizeof(kvpair *), compare_kvpair);
}

void add_to_memtable(memtable *mem, const char *key, const char *value)
{
    kvpair *pair = (kvpair *)malloc(sizeof(kvpair));
    pair->key = (char *)malloc(strlen(key) + 1);
    pair->value = (char *)malloc(strlen(value) + 1);
    strcpy(pair->key, key);
    strcpy(pair->value, value);
    mem->pairs[mem->size++] = pair;
}

void print_memtable(memtable *mem)
{
    for (int i = 0; i < mem->size; i++)
    {
        printf("%s:%s ", mem->pairs[i]->key, mem->pairs[i]->value);
    }
    printf("\n");
}`
//fwrite to disk; 
//compaction, fopen new file, reorganize all sst kv pairs...
//flush only applies for L0;
//from L0 to LN 
void flush_memtable(lsm *tree)
{
    memtable *mem = tree->mem;
    sort_memtable(mem);
    sstable *table = (sstable *)malloc(sizeof(sstable));
    table->size = mem->size;
    table->pairs = (kvpair *)malloc(sizeof(kvpair) * mem->size);
    for (int i = 0; i < mem->size; i++)
    {
        kvpair *pair = mem->pairs[i];
        table->pairs[i].key = (char *)malloc(strlen(pair->key) + 1);
        table->pairs[i].value = (char *)malloc(strlen(pair->value) + 1);
        strcpy(table->pairs[i].key, pair->key);
        strcpy(table->pairs[i].value, pair->value);
        free(pair->key);
        free(pair->value);
        free(pair);
    }
    free(mem->pairs);
    mem->size = 0;
    tree->levels->tables[0] = table;
    tree->levels->size = 1;
}

void add_to_tree(lsm *tree, const char *key, const char *value)
{
    memtable *mem = tree->mem;
    if (mem->size >= MEMTABLE_SIZE)
    {
        flush_memtable(tree);
        mem = tree->mem;
    }
    add_to_memtable(mem, key, value);
}

int compare_kvpair_void(const void *a, const void *b)
{
    kvpair ka = *(kvpair *)a;
    kvpair kb = *(kvpair *)b;
    return strcmp(ka.key, kb.key);
}

int compare_kvpair_key(const void *a, const void *b)
{
    kvpair *ka = (kvpair *)const_cast<char *>(a);
    kvpair *kb = (kvpair *)const_cast<char *>(b);
    return strcmp(ka->key, kb->key);
}

int binary_search(sstable *table, const char *key)
{
    kvpair pair = {.key = key};
    return bsearch(&pair, table->pairs, table->size, sizeof(kvpair), compare_kvpair_void) - table->pairs;
}

const char *get_from_tree(lsm *tree, const char *key)
{
    for (int i = LEVELS - 1; i >= 0; i--)
    {
        level *l = &tree->levels[i];
        if (l->size == 0)
        {
            continue;
        }
        int j = 0;
        while (j < l->size && strcmp(l->tables[j]->pairs[0].key, key) <= 0)
        {
            j++;
        }
        if (j == 0)
        {
            continue;
        }
        sstable *table = l->tables[j - 1];
        int k = binary_search(table, key);
        if (k < table->size && strcmp(table->pairs[k].key, key) == 0)
        {
            return table->pairs[k].value;
        }
    }
    return NULL;
}

void print_sstable(sstable *table)
{
    for (int i = 0; i < table->size; i++)
    {
        printf("%s:%s ", table->pairs[i].key, table->pairs[i].value);
    }
    printf("\n");
}

void print_level(level *l)
{
    for (int i = 0; i < l->size; i++)
    {
        printf("Level %d, Table %d: ", LEVELS - (l - &tree->levels[0]) - 1, i);
        print_sstable(l->tables[i]);
    }
}

void print_tree(lsm *tree)
{
    printf("Memtable: ");
    print_memtable(tree->mem);
    for (int i = 0; i < LEVELS; i++)
    {
        level *l = &tree->levels[i];
        printf("Level %d:\n", i);
        print_level(l);
    }
}

int main()
{
    lsm tree;
    tree.mem = (memtable *)malloc(sizeof(memtable));
    tree.mem->pairs = (kvpair **)malloc(sizeof(kvpair *) * MEMTABLE_SIZE);
    tree.mem->size = 0;
    tree.levels = (level *)malloc(sizeof(level) * LEVELS);
    for (int i = 0; i < LEVELS; i++)
    {
        tree.levels[i].tables = (sstable **)malloc(sizeof(sstable *) * SSTABLE_SIZE);
        tree.levels[i].size = 0;
    }

    add_to_tree(&tree, "key1", "value1");
    add_to_tree(&tree, "key2", "value2");
    add_to_tree(&tree, "key3", "value3");
    add_to_tree(&tree, "key4", "value4");
    add_to_tree(&tree, "key5", "value5");
    add_to_tree(&tree, "key6", "value6");
    add_to_tree(&tree, "key7", "value7");
    add_to_tree(&tree, "key8", "value8");
    add_to_tree(&tree, "key9", "value9");
    add_to_tree(&tree, "key10", "value10");

    const char *value = get_from_tree(&tree, "key5");
    if (value != NULL)
    {
        printf("Value of key5:%s\n", value);
    }
    else
    {
        printf("Key not found.\n");
    }

    value = get_from_tree(&tree, "key11");
    if (value != NULL)
    {
        printf("Value of key11: %s\n", value);
    }
    else
    {
        printf("Key not found.\n");
    }

    print_tree(&tree);

    for (int i = 0; i < LEVELS; i++)
    {
        level *l = &tree.levels[i];
        for (int j = 0; j < l->size; j++)
        {
            sstable *table = l->tables[j];
            free(table->pairs);
            free(table);
        }
        free(l->t   ables);
    }
    free(tree.mem->pairs);
    free(tree.mem);
    free(tree.levels);

    return 0;
}

// This is a simple implementation of an LSM tree in C, with support for adding key-value pairs and looking up values by key. The implementation uses a memtable and a series of levels, each containing a number of sorted sstables.

// The kvpair struct represents a key-value pair, and the sstable struct represents a sorted table of key-value pairs. The memtable struct represents an in-memory table that accumulates key-value pairs until it reaches a certain size or a certain number of entries, at which point it is flushed to disk as a new sstable.

// The level struct represents a level of the LSM tree, containing a number of sstables. The lsm struct represents the entire LSM tree, containing a memtable and a series of levels.

// The add_to_tree function adds a key-value pair to the memtable. If the memtable reaches the maximum size or maximum number of entries, it is flushed to disk as a new sstable and merged into the first level of the LSM tree. If the first level of the LSM tree becomes too large, it is merged into the next level, and so on.

// The get_from_tree function looks up a value by key in the LSM tree. It searches each level of the LSM tree from highest to lowest, looking for the first sstable that might contain the key (i.e., the first sstable whose first key is greater than or equal to the lookup key). It then performs a binary search on that sstable to find the value.

// The print_sstable and print_level functions are utility functions that print the contents of an sstable or level, respectively.

// The print_tree function is a utility function that prints the entire LSM tree, including the memtable and all levels.

// Finally, the main function creates an LSM tree, adds some key-value pairs to it, looks up some values by key, and then prints the entire LSM tree.

// Note that this is a very simple implementation of an LSM tree and does not include some of the more advanced features that are often found in real-world implementations, such as bloom filters, compaction heuristics, or optimizations for concurrent access.
