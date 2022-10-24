/** @file file_util.c
 *
 * @brief Provides the ability to interface with files.
 */

#include "header/file_util.h"

int read_dir(database_i *database)
{
    puts("*****************");
    printf("database size: %ld\n", database->size);

    for (int i = 0; i < database->size; i++)
    {
        printf("Cmp str: %s\n", (database->all_substrings[i])->substring);
    }

    printf("Root dir: %s\n", database->root_dir);

    /* DEBUG */

    int status = 0;
    size_t check_snprintf = 0;

    DIR *dir_fd = NULL;
    struct dirent *dir_ptr = NULL;
    struct stat dir_data = {0};

    char directory[PATH_SIZE] = {0};
    char file_path[PATH_SIZE] = {0};

    dir_fd = opendir(database->root_dir);
    if (NULL == dir_fd)
    {
        puts("[-] opendir failed: Could not open directory");
        status = -1;
        goto END;
    }

    while ((dir_ptr = readdir(dir_fd)))
    {
        // size_t name_len = 0;

        file_i file_found = {};
        substring_i node = {};
        node.substring = strndup(database->all_substrings[2]->substring, 255);

        /* Store name into file_path buffer. */
        check_snprintf = snprintf(file_path, sizeof(file_path), "%s%s", directory, dir_ptr->d_name);
        if (check_snprintf <= 0)
        {
            puts("[-] snprintf error[3]!?");
            status = -1;
            goto END;
        }

        // printf("File dir: %s\n", file_path);

        stat(file_path, &dir_data);
        if (S_ISDIR(dir_data.st_mode))
        {
            printf("dir name: %s\n", dir_ptr->d_name);
            // Store in some buffer
        }
        else
        {
            printf("Cmp str before strstr: %s\n", (database->all_substrings[2])->substring);
            printf("File name before strstr: %s\n", dir_ptr->d_name);
            printf("File dir before strstr: %s\n", file_path);

            char *found = NULL;

            found = strstr(dir_ptr->d_name, (database->all_substrings[2])->substring);

            if (found)
            {
                printf("Substring found here: %s\n", found);
                file_found.file_dir = strndup(file_path, 255);
                file_found.file_name = strndup(dir_ptr->d_name, 255);
                node.file_hits = &file_found;

                printf("Node File name before insert: %s\n", node.file_hits->file_name);
                printf("Node File dir before insert: %s\n", node.file_hits->file_dir);

                insert_node(database, database->all_substrings[2]->key, &node);

                free(file_found.file_dir);
                free(file_found.file_name);
                // printf("File added in database: %s\n", database->all_substrings[2]->file_hits);
            }
            else
                puts("subsring not found");
            // is file
        }
        free(node.substring);
    }

END:
    closedir(dir_fd);
    puts("*****************");
    return status;
}

int get_attributes(char *name, struct stat *attrs)
{
    int status = 0;

    status = stat(name, attrs);
    if (status != 0)
        perror("File stat");

    return status;

} /* get_attributes() */

/*** end of file ***/