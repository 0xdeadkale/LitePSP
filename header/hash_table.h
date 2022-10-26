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
    size_t key;         // Hash generated from substring arg.
    char *substring;    // Substring to compare
    file_i *file_hits;  // Linked list of files
    size_t file_count;  // Number of file hits from substring
    int status;         // Used for assigning work
    pthread_t assigned;
} substring_i;

typedef struct database
{
    size_t size;                   // Total substrings
    char *root_dir;                // Root dir from argv[1]
    substring_i **all_substrings;  // Substring nodes
    size_t total_count;            // Total count of hits 
} database_i;

database_i *create_hash(size_t size);

int insert_node(database_i *hashtable, size_t key, substring_i *data);

substring_i *search_node(database_i *hashtable, size_t key);

int delete_node(database_i *hashtable, size_t key);

void cleanup(database_i *hashtable, bool on_exit);

size_t hash(const void *var);

enum thread_status
{
    WORK_FREE = 0,
    WORK_IN_PROGRESS = 1,
    WORK_COMPLETE = 2
};

enum constraints
{
    PATH_SIZE = 255,
    PVAL = 167 // For hashing algo.
};

#endif /* HASH_H */

/*** end of file ***/