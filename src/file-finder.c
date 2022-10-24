/** @file parser.c
 *
 * @brief The parser driver file for CrowdStike's Sensor SDE Coding Project.
 */

#include "header/file-finder.h"

int main(int argc, char *argv[])
{

    int status = 0;
    size_t substring_hash = 0;

    database_i *database = NULL;

    if (argc <= 2)
    {
        puts("[-] Invalid number of arguments");
        puts("Usage: ./file-finder <dir> <substring1>[<substring2> [<substring3>]...]");

        status = -1;
        goto EXIT;
    }

    printf("Argc count: %d\n", argc);

    for (int i = 2; i < argc; i++)
    {
        printf("Search patterns: %s\n", argv[i]);
    }

    /* Create hashtable for key: substring, and value: file-hits pairs.*/
    database = create_hash(argc - 2);
    if (database == NULL)
    {
        status = -1;
        goto EXIT;
    }

    // TODO Transverse root directory

    database->root_dir = strndup(argv[1], 255);
    printf("Dir input: %s\n", database->root_dir);

    // int dir_size = strlen(argv[1]);

    substring_i node = {};

    substring_i *test = NULL;

    for (int i = 2; i < argc; i++)
    {
        node.substring = strndup(argv[i], PATH_SIZE);
        substring_hash = hash(argv[i]);
        printf("argv str: %s\n", node.substring);

        insert_node(database, substring_hash, &node);

        test = search_node(database, substring_hash);
        printf("Test node substring: %s\n", test->substring);
        free(node.substring);
    }

    read_dir(database);

    // delete_node(args, hash(argv[3]));
    /*test = search_node(database, hash(argv[4]));
    if (test != NULL)
    {
        while (test)
        {
            printf("Test node number post: %ld\n", test->count);
            printf("Test node dir: %s\n", test->file_hits->file_dir);
            printf("Test node name: %s\n", test->file_hits->file_name);

            test = test->next_substring;
        }
    }

    printf("File name found and pulled: %s\n", database->all_substrings[2]->file_hits->file_name);
    printf("File dir found and pulled: %s\n", database->all_substrings[2]->file_hits->file_dir);*/

EXIT:
    if (database != NULL && database->root_dir != NULL)
    {
        free(database->root_dir);
        database->root_dir = NULL;
    }
    cleanup(database);

    puts("\nExiting...");
    return (status);
}