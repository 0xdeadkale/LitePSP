/** @file parser.c
 *
 * @brief The parser driver file for CrowdStike's Sensor SDE Coding Project.
 */

#include "header/file-finder.h"


int main(int argc, char *argv[]) 
{

    int status = 0;
    size_t substring_hash = 0;

    if (argc <= 2) {
        puts("[-] Invalid number of arguments");
        puts("Usage: ./file-finder <dir> <substring1>[<substring2> [<substring3>]...]");

        status = -1;
        goto EXIT;
    }

    substrings_i *args = create_hash(argc - 2);
    if (args == NULL) {
        status = -1;
        goto EXIT;
    }

    // int dir_size = strlen(argv[1]);

    args->root_dir = strndup(argv[1], 255);
    printf("Dir input: %s\n", args->root_dir);

     
    for (int i = 2; i < argc; i++) {
        printf("Search patterns: %s\n", argv[i]);
    }

    file_i addition_test = {};
    addition_test.file_dir = strndup("hello", 255);
    addition_test.file_name = strndup("hey", 255);

    file_i addition_test2 = {};
    addition_test2.file_dir = strndup("poop", 255);
    addition_test2.file_name = strndup("pee", 255);

    for (int i = 2; i < argc; i++) {
        hit_i *test = NULL;
        substring_hash = hash(argv[i]);
        insert_node(args, substring_hash, &addition_test);
        test = search_node(args, substring_hash);
        printf("Test node number pre: %ld\n", test->count);
        
    }
    hit_i *test = NULL;
    insert_node(args, substring_hash, &addition_test);
    insert_node(args, substring_hash, &addition_test2);
    delete_node(args, substring_hash);
    test = search_node(args, substring_hash);

    while(test) {
        printf("Test node number post: %ld\n", test->count);
        printf("Test node dir: %s\n", test->file->file_dir);
        printf("Test node name: %s\n", test->file->file_name);

        test = test->next_hit;
    }

    free(addition_test.file_dir);
    free(addition_test.file_name);
    free(addition_test2.file_dir);
    free(addition_test2.file_name);

EXIT:
    if (args->root_dir != NULL) {
        free(args->root_dir);
        args->root_dir = NULL;
    }
    cleanup(args);

    puts("\nExiting...");
    return(status);

}