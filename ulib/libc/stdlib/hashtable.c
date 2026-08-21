
/*
 *  hashtable.c - hash tables
 *
 *  Implementation of hash tables.
 * 
 *  Refactor:
 *      [ ] Naming convention has to change (example ht_create is not ok).
 *      [ ] The value has to be done with dynamic memory (it is now 128 characters fixed).
 */

#include "types.h"
#include "user.h"

#include "hashtable.h"

/* 
 * Hash table entry (slot may be filled or empty).
 */
typedef struct {
    const char *key;            // key is NULL if this slot is empty
    char value [128];           // Holds the value of the key
} ht_entry;

/*
 * Hash table structure: create with ht_create, free with ht_destroy.
 */
struct ht {
    ht_entry *entries;          // hash slots
    int capacity;               // size of _entries array
    int length;                 // number of items in hash table
};

#define INITIAL_CAPACITY 16     // must not be zero

void
bzero (void * s, int n)
{
    char * c = s;

    int i;

    for (i = 0; i < n; ++i)
        c[i] = '\0';
}

void *
calloc (int nelem, int elsize)
{
    void * ptr;

    if (nelem == 0 || elsize == 0)
        nelem = elsize = 1;
  
    ptr = malloc (nelem * elsize);
    if (ptr) 
        bzero (ptr, nelem * elsize);
  
    return ptr;
}

ht * 
ht_create (void) 
{
    // Allocate space for hash table struct.
    ht * table = malloc(sizeof(ht));
    if (table == NULL) {
        return NULL;
    }

    table->length = 0;
    table->capacity = INITIAL_CAPACITY;

    // Allocate (zero'd) space for entry buckets.
    table->entries = calloc(table->capacity, sizeof(ht_entry));

    if (table->entries == NULL) {
         free(table); // error, free table before we return!
         return NULL;
    }
    
    return table;
}

void 
ht_destroy(ht* table) {
    
    // First free allocated keys because they reserved memory.
    for (int i = 0; i < table->capacity; i++) {
        if ((void*)table->entries[i].key != NULL) {
            //printf (2, "%s\n", (void*)table->entries[i].key);
            free((void*)table->entries[i].key);
        }
    }

    // Then free entries array and table itself.
    free(table->entries);
    free(table);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function

uint64 
hash_key(const char * key) 
{
    uint64 hash = FNV_OFFSET;
    for (const char * p = key; *p; p++) {
        hash ^= (uint64)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

char * 
ht_get (ht *table, const char *key)
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64 hash = hash_key (key);

    int index = (int)(hash & (uint64)(table->capacity - 1));

    // Loop till we find an empty entry.
    while (table->entries[index].key != NULL) {

        if (strcmp(key, table->entries[index].key) == 0) {
            // Found key, return value.
            return (char *) table->entries[index].value;
        }

        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= table->capacity) {
            // At end of entries array, wrap around.
            index = 0;
        }
    }

    return NULL;
}

char *
strdup (char *src)
{
    char *str;
    char *p;
    int len = 0;

    while (src[len])
        len++;
    str = malloc(len + 1);
    p = str;
    while (*src)
        *p++ = *src++;
    *p = '\0';
    return str;
}

// Internal function to set an entry (without expanding table).
const char * 
ht_set_entry (ht_entry *entries, int capacity, const char *key, char *value, int *plength) 
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64 hash = hash_key(key);

    int index = (int)(hash & (uint64)(capacity - 1));

    // Loop till we find an empty entry.
    while (entries[index].key != NULL) {

        if (strcmp(key, entries[index].key) == 0) {

//            printf (2, "Found it : %s %s at index %d \n", key, value, index);

            // Found key (it already exists), update value.
            strcpy (entries[index].value, value);

//            printf (2, "Changed Value = %s\n", entries[index].value);

            return entries[index].key;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= capacity) {
            // At end of entries array, wrap around.
            index = 0;
        }
    }

    // Didn't find key, allocate+copy if needed, then insert it.
    if (plength != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }

//printf (2, "Current 3 = %s\n", value);

    entries[index].key = (char *) key;

//printf (2, "Current 4 = %s\n", value);

    strcpy (entries[index].value, value);

//printf (2, "Current 5 = %s\n", value);

//  printf (2, "Current = %s Changed Value = %s\n", value, entries[index].value);

    return key;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
bool ht_expand(ht* table) {
    // Allocate new entries array.

//printf (2, "ht_expand\n");

    int new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;  // overflow (capacity would be too big)
    }
    ht_entry * new_entries = calloc(new_capacity, sizeof(ht_entry));
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (int i = 0; i < table->capacity; i++) {
        ht_entry entry = table->entries[i];
        if (entry.key != NULL) {
            ht_set_entry(new_entries, new_capacity, entry.key,
                         entry.value, NULL);
        }
    }

    // Free old entries array and update this table's details.
    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

const char * 
ht_set (ht *table, const char *key, char *value) {

//printf (2, "%s\n", value);

    //assert(value != NULL);
    if (value == NULL) {
        return NULL;
    }

    // If length will exceed half of current capacity, expand it.
    if (table->length >= table->capacity / 2) {
        if (!ht_expand(table)) {
            return NULL;
        }
    }

    // Set entry and update length.
    return ht_set_entry(table->entries, table->capacity, key, value,
                        &table->length);
}

int 
ht_length(ht * table) 
{
    return table->length;
}

hti 
ht_iterator (ht * table) 
{
    hti it;
    it._table = table;
    it._index = 0;
    return it;
}

bool 
ht_next(hti *it) 
{
    // Loop till we've hit end of entries array.
    ht *table = it->_table;
    while (it->_index < table->capacity) {
        int i = it->_index;
        it->_index++;
        if (table->entries[i].key != NULL) {
            // Found next non-empty item, update iterator key and value.
            ht_entry entry = table->entries[i];
            it->key = entry.key;
            strcpy (it->value, table->entries[i].value);
            return true;
        }
    }
    return false;
}
