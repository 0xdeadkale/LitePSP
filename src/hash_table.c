/** @file hash_table.c
 *
 * @brief A library for hash table releated code. Other functionalities of
 * typical a hashtable have been stripped for this Crowdstrike project 
 * (search, delete node, etc.).
 */

#include "header/includes.h"
#include "header/hash_table.h"


/**
 * @brief This function creates/allocates the
 * hashtable and 2D array of the substring.
 *
 * @param size The size used to create the hashtable.
 * 
 * @return The hashtable (as a database_i pointer).
 */
database_i *create_hash(size_t size)
{
    if (size < 1)
    {
        puts("Need a real bucket size\n");
        goto END;
    }

    /* Creates hashtable */
    database_i *hashtable = calloc(1, sizeof(database_i));
    if (NULL == hashtable)
    {
        perror("Hashtable Calloc failed");
        goto END;
    }
    hashtable->size = size;

    /* Calloc the 2D array by the number of substring args */
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

/**
 * @brief This function inserts either a substring node or file
 * info to the linked list of a certain substring node.
 *
 * @param hashtable Pointer to the hashtable.
 * @param key Key used to generate the index to reference the file hits.
 * @param data Node that contains data to be inserted into hashtable.
 * 
 * @return Status of the insertion.
 */
int insert_node(database_i *hashtable, size_t key, substring_i *data)
{
    /* Setting up variables */
    int status = 0;
    size_t index = 0;

    substring_i *node = NULL;
    file_i *file = {NULL};

    file_i *file_tmp = {NULL};
    /************************/

    if (hashtable == NULL)
    {
        puts("Hashtable does not exist!");
        status = -1;
        goto END;
    }

    index = key % hashtable->size;  // Generate index for insertion

    /* If there is file data, prepare that data for chaining to existing node */
    if (data->file_hits != NULL)
    {
        file = calloc(1, sizeof(file_i));
        if (file == NULL)
        {
            puts("File calloc failed!\n");
            status = -1;
            goto END;
        }

        /* Pre-populate struct that holds file data */
        file->file_dir = strndup(data->file_hits->file_dir, PATH_MAX);
        file->file_name = strndup(data->file_hits->file_name, FILENAME_MAX);
        file->next_hit = NULL;
    }
    /* If there is no file data, add a blank substring node */
    else
    {
        node = calloc(1, sizeof(substring_i));
        if (node == NULL)
        {
            puts("Node calloc failed!\n");
            status = -1;
            goto END;
        }

        node->key = key;
        node->substring = strndup(data->substring, PATH_SIZE);
        node->count = 0;
        node->file_hits = NULL;
        node->status = 0;
        
        hashtable->all_substrings[index] = node;
    }

    /* Chain the file data to an existing node */
    if (file)
    {
        hashtable->all_substrings[index]->count = 1;

        /* If there is not node in the linked list, place first node */
        if (hashtable->all_substrings[index]->file_hits == NULL)
            hashtable->all_substrings[index]->file_hits = file;
        /* If there is a node in the linked list, transverse to free spot and link */
        else 
        {
            file_tmp = hashtable->all_substrings[index]->file_hits;

            while (file_tmp->next_hit != NULL)
            {
                hashtable->all_substrings[index]->count++;  
                file_tmp = file_tmp->next_hit;
            }

            hashtable->all_substrings[index]->count++;  // Update count when adding node
            file_tmp->next_hit = file;  
        }
    }
    

END:
    if (status == -1) {
        if (file->file_dir != NULL)
            free(file->file_dir);
        if (file->file_name != NULL)
            free(file->file_name);
    }

    return status;
}

/**
 * @brief This function free's all allocated memory
 * in all structs that compose the hashtable.
 *
 * @param hashtable Pointer to the hashtable.
 * @param on_exit Flag to trigger exit free's.
 */
void cleanup(database_i *hashtable, bool on_exit)
{
    substring_i *current_node = NULL;
    file_i *next_file = NULL;

    if (hashtable == NULL)
    {
        puts("Hashtable does not exist!");
        goto END;
    }

    /* Loops through and cleans up everything all nodes and subnodes */ 
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
            /* When next node in chain was detected after 
             * freeing last node, assign as current node
             */
            if (next_file != NULL)
            {
                current_node->file_hits = next_file;
                next_file = NULL;
            }

            /* Setting next node (if there is a next node) 
             * so that we can free current node 
             */
            if (current_node->file_hits != NULL && current_node->file_hits->next_hit != NULL)
            {
                next_file = current_node->file_hits->next_hit;
            }

            /* If no files in linked list, start freeing main node*/
            if (current_node->file_hits == NULL)
            { 
                if (on_exit == true && current_node->substring != NULL) {
                    free(current_node->substring);
                    current_node->substring = NULL;
                }
            }
            /* Free the actual file struct in the linked list*/
            else
            {
                if (on_exit == true && next_file == NULL && current_node->substring != NULL) {
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
      
            /* If no more files to free, free main node */
            if (next_file == NULL) {
        
                if (on_exit == true) {
                    free(current_node);
                    current_node = NULL;
                }
                else
                    break;            
            }

        } while (current_node != NULL || next_file != NULL);
    }

    /* Free hashtable when exiting */
    if (on_exit == true) {
        free(hashtable->all_substrings);
        hashtable->all_substrings = NULL;

        free(hashtable->root_dir);
        hashtable->root_dir = NULL;

        free(hashtable);
        hashtable = NULL;
    }

END:
    
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