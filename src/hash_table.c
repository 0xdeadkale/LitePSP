/** @file hash_table.c
 *
 * @brief A library for hash table releated code. Other functionalities of
 * typical a hashtable have been stripped for this project 
 * (search, delete node, etc.).
 */

#include "header/includes.h"
#include "header/hash_table.h"

database_i *create_hash(size_t size)
{
    if (size < 1)
    {
        puts("Need a real bucket size\n");
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

int insert_node(database_i *hashtable, size_t key, substring_i *data)
{
    int status = 0;
    size_t index = 0;

    file_i *file_tmp = NULL;

    substring_i *node = NULL;
    file_i *file = NULL;

    if (hashtable == NULL)
    {
        puts("Hashtable does not exist!");
        status = -1;
        goto END;
    }

    index = key % hashtable->size;

    if (data->file_hits != NULL)
    {
        // printf("insert node data: %s\n", data->file_hits->file_name);

        file = calloc(1, sizeof(file_i));
        if (file == NULL)
        {
            puts("File calloc failed!\n");
            status = -1;
            goto END;
        }

        file->file_dir = strndup(data->file_hits->file_dir, 255);
        file->file_name = strndup(data->file_hits->file_name, 255);
        file->next_hit = NULL;
        // add_file = true;

        // node->file_hits = file;
    }
    else
    {
        node = calloc(1, sizeof(substring_i));
        if (NULL == node)
        {
            puts("Node calloc failed!\n");
            status = -1;
            goto END;
        }

        node->key = key;
        node->count = 0;
        node->substring = strndup(data->substring, 255);
        node->next_substring = NULL;
        node->file_hits = NULL;
        node->status = 0;
        
        hashtable->all_substrings[index] = node;
    }

    if (file)
    {
        hashtable->all_substrings[index]->count = 1;

        if (hashtable->all_substrings[index]->file_hits == NULL)
            hashtable->all_substrings[index]->file_hits = file;
        else 
        {
            file_tmp = hashtable->all_substrings[index]->file_hits;
            while (file_tmp->next_hit != NULL)
            {
                hashtable->all_substrings[index]->count++;
                file_tmp = file_tmp->next_hit;
            }

            hashtable->all_substrings[index]->count++;
            file_tmp->next_hit = file;  
        }
    }
    

END:
    return status;
}

void cleanup(database_i *hashtable, bool on_exit)
{
    substring_i *current_node = NULL;
    file_i *next_file = NULL;

    if (NULL == hashtable)
    {
        return;
    }

    /* Loops through and cleans up everything. */ 
    for (size_t i = 0; i < hashtable->size; i++)
    {
        current_node = hashtable->all_substrings[i];
        
        if (current_node == NULL)
        {
            printf("Delete node target: %ld\n", current_node->key);
            puts("Delete Node invalid");
            continue;
        }

        do
        {
            if (next_file != NULL)
            {
               
                current_node->file_hits = next_file;
                next_file = NULL;
            }

            if (current_node->file_hits != NULL && current_node->file_hits->next_hit != NULL)
            {
                next_file = current_node->file_hits->next_hit;
            }

            if (current_node->file_hits == NULL)
            { 
                if (on_exit == true && current_node->substring != NULL) {
                    free(current_node->substring);
                    current_node->substring = NULL;
                }
            }
            else
            {
                
                if (next_file == NULL && current_node->substring != NULL) {
                    free(current_node->substring);
                    current_node->substring = NULL;
                }
                    
                free(current_node->file_hits->file_dir);
                current_node->file_hits->file_dir = NULL;

                free(current_node->file_hits->file_name);
                current_node->file_hits->file_name = NULL;

                free(current_node->file_hits);
                current_node->file_hits = NULL;

                current_node->count--;
            }
      
            if (next_file == NULL) {
        
                if (on_exit == true) {
                    free(current_node);
                    current_node = NULL;
                }
                else {
                    break;
                }
                
            }

        } while (current_node != NULL || next_file != NULL);
    }

    if (on_exit == true) {
        free(hashtable->all_substrings);
        hashtable->all_substrings = NULL;

        free(hashtable->root_dir);
        hashtable->root_dir = NULL;

        free(hashtable);
        hashtable = NULL;
    }
    
}

/**
 * @brief This function generates a hash for inserting nodes
 * into a hash table.
 * https://cp-algorithms.com/string/string-hashing.html
 *
 * @param var Seed number for algoithm.
 *
 * @return Hashed number.
 */
size_t hash(const void *var)
{
    size_t sum = 0;
    size_t p = 1;

    for (const char *str = (const char *)var; *str; str++)
    {
        sum = ((*str - 'a' + 1) * p + sum);
        p = (p * PVAL);
    }
    return sum;
}

/*** end of file ***/