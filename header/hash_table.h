/** @file p_hash.h
 *
 * @brief The header file for hash_table.c.
 */

#ifndef HASH_H
#define HASH_H

typedef struct file
{
    char *substring;  // Substring to compare
    char *file_dir;  // File directory
    char *file_name;  // File name
} file_i;

typedef struct hit
{
    size_t key;  // Hash generated from substring arg.
    size_t count;  // Number of file hits from substring
    file_i *file;
    struct hit *next_hit;
} hit_i;

typedef struct substrings
{
    size_t size;  /* Size of hashtable. */
    char *root_dir;
    hit_i **file_name_hits;
} substrings_i;

substrings_i *create_hash(size_t size);

int insert_node(substrings_i *hashtable, size_t key, file_i *data);

hit_i *search_node(substrings_i *hashtable, size_t key);

int delete_node(substrings_i *hashtable, size_t key);

void cleanup(substrings_i *hashtable);

size_t hash(const void *var);

enum constraints
{
    PATH_SIZE = 255,
    PVAL = 167 // For hashing algo.
};

#endif /* HASH_H */

/*** end of file ***/