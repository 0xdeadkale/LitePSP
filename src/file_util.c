/** @file file_util.c
 *
 * @brief Provides the ability to interface with files.
 */

#include "header/includes.h"
#include "header/file_util.h"
#include "header/hash_table.h"

int read_dir(substrings_i *args, char *path) 
{
    int status = 0;
    size_t check_snprintf = 0;

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
        size_t name_len = 0;
        
        /* Store name into file_path buffer. */
        check_snprintf = snprintf(file_path, sizeof(file_path), "%s%s", directory, dir_ptr->d_name);
        if (check_snprintf <= 0)
        {
            puts("[-] LS failed: snprintf error[3]!?");
            status = -1;
            goto END;
        }

        stat(file_path, &dir_data);
        if (S_ISDIR(dir_data.st_mode))
        {
            ;
            // 
        }
        else
        {
            if (strstr(args->file_name_hits, args->file_name_hits[4]) != NULL) {
                args->file_name_hits[2]->file->file_dir = strndup(directory, 255);
                args->file_name_hits[2]->file->file_name = strndup(dir_ptr->d_name, 255);
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