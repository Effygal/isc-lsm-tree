#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LEVELS 10
#define MAX_LEVEL_SIZE 100

typedef struct {
    int key;
    int value;
} KeyValue;

typedef struct {
    KeyValue data[MAX_LEVEL_SIZE];
    int size;
} Level;

typedef struct {
    Level levels[MAX_LEVELS];
    int level_count;
} LSMTree;

LSMTree *create_lsm_tree() {
    LSMTree *tree = malloc(sizeof(LSMTree));
    tree->level_count = 0;
    return tree;
}

void insert(LSMTree *tree, int key, int value) {
    int i, j;
    for (i = 0; i < tree->level_count; i++) {
        Level *level = &tree->levels[i];
        for (j = 0; j < level->size; j++) {
            KeyValue *kv = &level->data[j];
            if (kv->key == key) {
                kv->value = value;
                return;
            }
        }
    }

    if (tree->levels[0].size >= MAX_LEVEL_SIZE) {
        // Merge levels
        int level_index = 0;
        while (level_index < tree->level_count && tree->levels[level_index].size >= MAX_LEVEL_SIZE) {
            Level *level1 = &tree->levels[level_index];
            Level *level2 = &tree->levels[level_index + 1];
            int i = 0, j = 0, k = 0;
            Level merged_level;
            while (i < level1->size && j < level2->size && k < MAX_LEVEL_SIZE) {
                if (level1->data[i].key < level2->data[j].key) {
                    merged_level.data[k++] = level1->data[i++];
                } else {
                    merged_level.data[k++] = level2->data[j++];
                }
            }
            while (i < level1->size && k < MAX_LEVEL_SIZE) {
                merged_level.data[k++] = level1->data[i++];
            }
            while (j < level2->size && k < MAX_LEVEL_SIZE) {
                merged_level.data[k++] = level2->data[j++];
            }
            merged_level.size = k;
            tree->levels[level_index] = merged_level;
            level_index++;
        }
        if (level_index == tree->level_count) {
            tree->level_count++;
        }
    }

    // Insert new key-value pair
    Level *level = &tree->levels[0];
    int index = level->size;
    while (index > 0 && level->data[index - 1].key > key) {
        level->data[index] = level->data[index - 1];
        index--;
    }
    level->data[index].key = key;
    level->data[index].value = value;
    level->size++;
}

int get(LSMTree *tree, int key) {
    int i, j;
    for (i = 0; i < tree->level_count; i++) {
        Level *level = &tree->levels[i];
        for (j = 0; j < level->size; j++) {
            KeyValue *kv = &level->data[j];
            if (kv->key == key) {
                return kv->value;
            }
        }
    }
    return -1;
}

void print_lsm_tree(LSMTree *tree)
{
    printf("LSM Tree:\n");
    for (int i = 0; i < tree->level_count; i++)
    {
        Level *level = &tree->levels[i];
        printf("Level %d: [", i);
        for (int j = 0; j < level->size; j++)
        {
            KeyValue *kv = &level->data[j];
            printf("(%d, %d)", kv->key, kv->value);
            if (j < level->size - 1)
            {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

int main()
{
    LSMTree *tree = create_lsm_tree();
    // Insert some key-value pairs
    insert(tree, 10, 100);
    insert(tree, 20, 200);
    insert(tree, 30, 300);
    insert(tree, 15, 150);

    // Print the tree
    print_lsm_tree(tree);

    int value = get(tree, 20);
    // Lookup a key
    printf("Value of key 20: %d\n", value);

    return 0;
}

/*
This implementation defines a `KeyValue` struct to hold the key-value pairs, a `Level` struct to represent each level in the tree, and an `LSMTree` struct to hold all the levels. 
The `insert()` function inserts a key-value pair into the first level of the tree, 
and if that level is full, it merges adjacent levels until there is space for the new key-value pair. 
The `get()` function looks up a key in the tree and returns its corresponding value.

The `print_lsm_tree()` function is used to print the contents of the tree in a human-readable format.

Finally, the `main()` function creates an LSM tree, inserts some key-value pairs, prints the tree, and looks up a key in the tree.
*/