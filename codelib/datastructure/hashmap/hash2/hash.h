#ifndef __GG_HASH_H
#define __GG_HASH_H

#ifdef __cplusplus
extern "C" {
#endif



#define  HASHTABLE_SIZE_MIN   100

typedef struct listnode *List;
typedef struct listnode *Possion;
typedef struct hashtable *HashTable;

typedef void (*free_element_func)(void *);
typedef unsigned int (*hash_func)(const char *, unsigned int);

HashTable hashtable_create(int tablesize, hash_func hash, free_element_func free_element);
Possion   hashtable_find(HashTable table, const char *key);
int       hashtable_insert(HashTable table, const char *key, void *element);
int       hashtable_delete(HashTable table, const char *key);
void      hashtable_destroy(HashTable table);

enum status {Empty, Legitimate, Delete};

struct listnode {
    void *element;
    char *key;
    enum status st;
};

struct hashtable {
    unsigned int tablesize;
    List *list;
    free_element_func free_element;
    hash_func hash;
};




#ifdef __cplusplus
}
#endif

#endif

