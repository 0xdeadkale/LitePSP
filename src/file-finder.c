/** @file file-finder.c
 *
 * @brief The driver file for CrowdStike's Sensor SDE Coding Project.
 */

#include "header/file-finder.h"

static void handler(int signum);

/**
 * @brief The main function of the program. Calls 
 * all needed functions that contain the business logic.
 *
 * @param argc The number of strings pointed by argv.
 * @param argv The array of arguments.
 * 
 * @return The status of how the program exits.
 */
int main(int argc, char *argv[])
{

    int status = 0; // Error handling
    size_t substring_hash = 0;
    
    /* For initial substring insertions into hashtable */
    database_i *database = NULL;
    substring_i node = {};

    puts("+++++++++++++++++++++++++++++++++++++++++");
    puts("+                                       +");
    puts("+ Crowdstrike Sensor SDE Coding project +");
    puts("+                                       +");
    puts("+++++++++++++++++++++++++++++++++++++++++\n");

    /* If argc is not at least a <root dir> and <substring1>, exit */
    if (argc <= 2 || argc > _POSIX_ARG_MAX)
    {
        puts("[-] Invalid number of arguments");
        puts("Usage: ./file-finder <dir> <substring1>[<substring2> [<substring3>]...]");

        status = -1;
        goto EXIT;
    }

    /* Create hashtable container for storage of key (substring), and value (file-hit) pairs.*/
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

    /* Insert argv substring nodes into hashtable */
    for (int i = 2; i < argc; i++)
    {
        node.substring = strndup(argv[i], PATH_SIZE);  // Loop through each argv
        substring_hash = hash(argv[i]);

        insert_node(database, substring_hash, &node);

        free(node.substring);
    }

    /* For CTRL-C handling. All other signals will be blocked */
    create_sig_handler(SIGINT, handler);

    puts("++++++++++++++++++++++++++++++++++++++++");
    printf("[+] Starting search at '%s'\n", database->root_dir);
    puts("++++++++++++++++++++++++++++++++++++++++\n");

    /* Business logic */
    status = thread_dispatcher(database);  // --> In file_util.c
    if (status != 0) {
        puts("Threads broke");
        goto EXIT;
    }

    /* If worker threads are done, exit gracefully */
    while(exit_flag == true || jobs == 2) {
        exit_flag = true;
        if (jobs == 0)
            break;
        else
            sleep(2);  // Wait for dumper and input threads
    }

    print_stats(database);

EXIT:
    /* Clean up remnants of the hashtable */
    if (database != NULL && database->root_dir != NULL)
    {
        free(database->root_dir);
        database->root_dir = NULL;
    }
    cleanup(database, true);

    if (status != 0)
        puts("\n[-] Exiting...");
    else
        puts("\n[+] Exiting...");

    return status;
}

void print_stats(database_i *database) 
{
    /* If exiting, print out file hit statistics */
    if (exit_flag == true) {

        size_t total_hits = 0;

        puts("\n+++++++++++++++++++++++++++++++++++++");
        puts("+                                   +");
        puts("+       [~] Search complete         +");
        puts("+                                   +");
        puts("+++++++++++++++++++++++++++++++++++++");
        for (int i = 0; i < ((database_i *)database)->size; i++) {
            printf("[~] Substring %s file hits: %ld\n", 
                  ((database_i *)database)->all_substrings[i]->substring, 
                  ((database_i *)database)->all_substrings[i]->total_file_count);

            total_hits += ((database_i *)database)->all_substrings[i]->total_file_count;
        }
        printf("[~] Total file hits: %ld\n", total_hits);
        puts("+++++++++++++++++++++++++++++++++++++\n");
    }
    /*********************************************/
}

/**
 * @brief This function generates a sigaction struct
 * used for signal handling.
 *
 * @param signum The signal to process in the sig handler.
 * @param func Pointer to the signal handler.
 */
void create_sig_handler(int signum, void (*func)(int))
{
    struct sigaction sa = { .sa_handler = func };
    if (sigaction(signum, &sa, NULL) == -1)
    {
        perror("Sigaction:");
    }
}

/**
 * @brief This function listens to any signals and
 * processes them.
 *
 * @param signum The signal to process.
 */
static void handler(int signum)
{
    (void)signum;

    switch (signum)
    {
        // Add other signals here.
        case SIGINT:
            puts("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
            puts("[~] CTRL-C detected, getting ready to exit gracefully [~]");
            puts("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            
            exit_flag = true;
            break;
        default:
            puts("\nHow did this happen!?");
            break;
    }

}
