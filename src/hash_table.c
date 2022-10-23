/** @file hash_table.c
 *
 * @brief A library for hash table releated code.
 */

#include "header/includes.h"
#include "header/hash_table.h"

database_i *create_hash(size_t size)
{
    if (size < 1)
    {
        printf("Need a real bucket size\n");
        goto END;
    }

    // Creates hashtable.
    //
    database_i *hashtable = calloc(1, sizeof(database_i));
    if (NULL == hashtable)
    {
        perror("Hashtable Calloc failed");
        goto END;
    }
    hashtable->size = size;

    // Calloc the 2D array by the number of substring args.
    //
    hashtable->all_substrings = (substring_i **)calloc(size, sizeof(substring_i *));
    if (NULL == hashtable->all_substrings)
    {
        perror("Array Calloc failed!\n");
        goto END;
    }

    return hashtable;

END:
    if (hashtable != NULL)
    {
        free(hashtable);
    }
    return NULL;
}

int insert_node(database_i *hashtable, size_t key, file_i *metadata)
{
    int status = 0;
    size_t index = 0;

    substring_i *tmp = NULL;
    substring_i *node = NULL;
    file_i *file = NULL;

    // Check if hashtable is NULL.
    //
    if (NULL == hashtable)
    {
        puts("Hashtable does not exist!");
        status = -1;
        goto END;
    }

    node = calloc(1, sizeof(substring_i));
    if (NULL == node)
    {
        printf("Node calloc failed!\n");
        status = -1;
        goto END;
    }

    file = calloc(1, sizeof(file_i));
    if (NULL == file)
    {
        printf("File calloc failed!\n");
        status = -1;
        goto END;
    }

    // Populate key and values.
    //
    node->key = key;
    node->next_substring = NULL;

    if (metadata->file_dir != NULL)
        file->file_dir = strndup(metadata->file_dir, 255);
    if (metadata->file_name != NULL)
        file->file_name = strndup(metadata->file_name, 255);
    // file->substring = strndup(metadata->substring, 255);
    node->file = file;

    index = key % hashtable->size;

    // If there is already a node at the index, we don't need to do anything.
    // A collision helps us since it confirms a substring already exists.
    //
    if (hashtable->all_substrings[index] != NULL)
    {
        tmp = hashtable->all_substrings[index];
        node->count = 1;

        while (tmp->next_substring != NULL)
        {
            node->count++;
            tmp = tmp->next_substring;
        }

        if (tmp->next_substring == NULL)
        {
            node->count++;
            tmp->next_substring = node;
        }
    }

    // Else, there is no node at the index. //
    else
    {
        hashtable->all_substrings[index] = node;
        node->count = 1;
    }

END:
    return status;
}

substring_i *search_node(database_i *hashtable, size_t key)
{
    substring_i *tmp = NULL;
    size_t index = 0;

    if (NULL == hashtable)
    {
        puts("Hashtable does not exist!");
        return NULL;
    }

    index = key % hashtable->size;
    tmp = hashtable->all_substrings[index];
    if (tmp == NULL)
    {
        puts("Node does not exist!");
        return NULL;
    }

    // Searchs for key through linked-list at the index of the hashtable.
    //
    while (tmp != NULL)
    {
        if (tmp->key == key)
        {
            break;
        }
        tmp = tmp->next_substring;
    }

    return tmp;
}

int delete_node(database_i *hashtable, size_t key)
{
    int status = 0;
    size_t index = 0;

    substring_i *current_node = NULL;
    substring_i *next_node = NULL;

    if (NULL == hashtable)
    {
        puts("Hashtable does not exist!");
        status = -1;
        goto END;
    }

    index = key % hashtable->size;
    current_node = hashtable->all_substrings[index];
    if (current_node == NULL)
    {
        puts("Node does not exist!");
        status = -1;
        goto END;
    }

    do
    {
        if (next_node != NULL)
        {
            current_node = next_node;
            next_node = NULL;
        }

        /* Free's file struct's name and dir if memory is allocated for them. */
        free(current_node->file->file_dir);
        current_node->file->file_dir = NULL;

        free(current_node->file->file_name);
        current_node->file->file_name = NULL;

        free(current_node->file);
        current_node->file = NULL;
        /**********************************************************************/

        /* Free's current node*/
        if (current_node->next_substring != NULL)
        {
            next_node = current_node->next_substring;

            free(current_node);
            current_node = NULL;
        }
        /* If not linked list, just free the one node. */
        else
        {
            free(current_node);

            current_node = NULL;
            next_node = NULL;
        }

    } while (current_node != NULL || next_node != NULL);

END:
    return status;
}

void cleanup(database_i *hashtable)
{
    substring_i *current_node = NULL;
    substring_i *next_node = NULL;

    if (NULL == hashtable)
    {
        return;
    }

    // Loops through and cleans up everything.
    //
    for (size_t i = 0; i < hashtable->size; ++i)
    {
        current_node = hashtable->all_substrings[i];
        if (current_node == NULL)
        {
            puts("ok");
        }

        do
        {

            if (next_node != NULL)
            {
                current_node = next_node;
                next_node = NULL;
            }

            /* Free's file struct's name and dir if memory is allocated for them. */
            free(current_node->file->file_dir);
            current_node->file->file_dir = NULL;

            free(current_node->file->file_name);
            current_node->file->file_name = NULL;

            free(current_node->file);
            current_node->file = NULL;
            /**********************************************************************/

            if (current_node->next_substring != NULL)
            {
                next_node = current_node->next_substring;

                free(current_node);
                current_node = NULL;
            }
            else
            {
                free(current_node);

                current_node = NULL;
                next_node = NULL;
            }

        } while (current_node != NULL || next_node != NULL);
    }

    free(hashtable->all_substrings);
    hashtable->all_substrings = NULL;

    free(hashtable->root_dir);
    hashtable->root_dir = NULL;

    free(hashtable);
    hashtable = NULL;
}

/**
 * @brief This function opens a file and returns the file handle.
 * https://cp-algorithms.com/string/string-hashing.html
 *
 * @param p_var Seed number for algoithm.
 *
 * @return Hashed number.
 */
size_t hash(const void *p_var)
{
    size_t sum = 0;
    size_t p = 1;

    for (const char *p_str = (const char *)p_var; *p_str; p_str++)
    {
        sum = ((*p_str - 'a' + 1) * p + sum);
        p = (p * PVAL);
    }
    return sum;
}

/*** end of file ***/