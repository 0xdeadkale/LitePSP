/** @file p_hash.h
 *
 * @brief The header file for hash_table.c.
 */

#ifndef HASH_H
#define HASH_H

typedef struct file
{
    char *file_dir;  // File directory
    char *file_name; // File name
    struct file *next_hit;
} file_i;

typedef struct substring
{
    size_t key;      // Hash generated from substring arg.
    char *substring; // Substring to compare
    size_t count;    // Number of file hits from substring
    file_i *file_hits;
    int status;
    struct substring *next_substring;
} substring_i;

typedef struct database
{
    size_t size; /* Size of hashtable. */
    char *root_dir;
    substring_i **all_substrings;
} database_i;

database_i *create_hash(size_t size);

int insert_node(database_i *hashtable, size_t key, substring_i *data);

substring_i *search_node(database_i *hashtable, size_t key);

int delete_node(database_i *hashtable, size_t key);

void cleanup(database_i *hashtable, bool on_exit);

size_t hash(const void *var);

enum constraints
{
    PATH_SIZE = 255,
    PVAL = 167 // For hashing algo.
};

#endif /* HASH_H */

/*** end of file ***/