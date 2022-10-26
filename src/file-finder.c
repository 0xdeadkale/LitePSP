/** @file file-finder.c
 *
 * @brief The driver file for CrowdStike's Sensor SDE Coding Project.
 */

#include "header/file-finder.h"


static void handler(int signum);

int main(int argc, char *argv[])
{

    int status = 0; // Error handling

    size_t substring_hash = 0;
    database_i *database = NULL;
    substring_i node = {};

    if (argc <= 2)
    {
        puts("[-] Invalid number of arguments");
        puts("Usage: ./file-finder <dir> <substring1>[<substring2> [<substring3>]...]");

        status = -1;
        goto EXIT;
    }

    /* Create hashtable for key: substring, and value: file-hits pairs.*/
    database = create_hash(argc - 2);
    if (database == NULL)
    {
        status = -1;
        goto EXIT;
    }
    database->root_dir = strndup(argv[1], PATH_SIZE);

    /* Inserts substrings into hashtable as nodes. Each node has a linked-list
     * of file 'hits' from when the directory walk is scanning for files.
     * Accessing the hashtable node is O(1)/constant time, while linking the file
     * hits together is O(n) time, except for the first file hit.
     */

    /* Alternative design:
     * Could utilize a red-black tree with it's O(logn) search, or a bst.
     */

    /* Insert substring nodes into hashtable */
    for (int i = 2; i < argc; i++)
    {
        node.substring = strndup(argv[i], PATH_SIZE);  // Loop through each argv
        substring_hash = hash(argv[i]);

        insert_node(database, substring_hash, &node);

        free(node.substring);
    }

    create_sig_handler(SIGINT, handler); // For CTRL+C handling.

    /* Business logic */
    status = thread_dispatcher(database);  // In file_util.c
    if (status != 0) {
        puts("Threads broke");
        goto EXIT;
    }

    
    
    while(exit_flag == true || jobs == 2) {
        exit_flag = true;
        if (jobs == 0)
            break;
        else
            sleep(2);
     }
        
    
    // dumper((void *)database);

EXIT:
    if (database != NULL && database->root_dir != NULL)
    {
        free(database->root_dir);
        database->root_dir = NULL;
    }
    cleanup(database, true);

    puts("\nExiting...");
    return status;
}

void create_sig_handler(int signum, void (*p_func)(int))
{
    struct sigaction sa = { .sa_handler = p_func };
    if (sigaction(signum, &sa, NULL) == -1)
    {
        perror("Sigaction:");
    }
}

static void handler(int signum)
{
    (void)signum;

    switch (signum)
    {
        // Add other signals here.
        case SIGINT:
            puts("TRL-C detected, waiting for sleep");
            exit_flag = true;
            break;
        default:
            puts("\nHow did this happen!?");
            break;
    }

}
