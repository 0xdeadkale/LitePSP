/** @file file_util.c
 *
 * @brief Provides the ability to interface with files.
 */


#include "header/file_util.h"


int read_dir(substrings_i *args, char *path) 
{
    for (int i = 0; i < 3; i++) {
        printf("Cmp str: %s\n", (args->file_name_hits[i])->file->substring);
    }
    int status = 0;
    size_t check_snprintf = 0;

    printf("File dir: %s\n", path);

    DIR *dir_fd = NULL;
    struct dirent *dir_ptr = NULL;
    struct stat dir_data = {0};

    char directory[PATH_SIZE] = {0};
    char file_path[PATH_SIZE] = {0};

    dir_fd = opendir(path);
    if (NULL == dir_fd)
    {
        puts("[-] opendir failed: Could not open directory");
        status = -1;
        goto END;
    }

    while ((dir_ptr = readdir(dir_fd)))
    {
        // size_t name_len = 0;
        
        /* Store name into file_path buffer. */
        check_snprintf = snprintf(file_path, sizeof(file_path), "%s%s", directory, dir_ptr->d_name);
        if (check_snprintf <= 0)
        {
            puts("[-] LS failed: snprintf error[3]!?");
            status = -1;
            goto END;
        }

        printf("File dir: %s\n", file_path);

        stat(file_path, &dir_data);
        if (S_ISDIR(dir_data.st_mode))
        {
            ;
            // 
        }
        else
        {
            printf("Cmp str before strstr: %s\n", (args->file_name_hits[2])->file->substring);
            printf("File name before strstr: %s\n", dir_ptr->d_name);
            printf("File dir before strstr: %s\n", file_path);

            char *found = NULL;

            found = strstr((args->file_name_hits[2])->file->substring, dir_ptr->d_name);
            printf("Substring found here: %s\n", found);

            if (found) {
                puts("substring found in filename");
                args->file_name_hits[2]->file->file_dir = strndup(path, 255);
                args->file_name_hits[2]->file->file_name = strndup(dir_ptr->d_name, 255);
                printf("File name: %s\n", args->file_name_hits[2]->file->file_name);
                printf("File dir: %s\n", args->file_name_hits[2]->file->file_dir);
            }
            //is file
        }
    }

END:
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