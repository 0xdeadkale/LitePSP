/** @file parser.c
 *
 * @brief The parser driver file for CrowdStike's Sensor SDE Coding Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header/file-finder.h"
#include "header/cll.h"

int main(int argc, char *argv[]) 
{

    int status = 0;

    if (argc <= 2) {
        puts("[-] Invalid number of arguments");
        puts("Usage: ./file-finder <dir> <substring1>[<substring2> [<substring3>]...]");

        status = -1;
        goto EXIT;
    }

    puts("Here");

    args_i *args = calloc(1, sizeof(args_i));
    if (args == NULL) {
        perror("Calloc");
        status = -1;
        goto EXIT;
    }

    puts("Here2");

    int dir_size = strlen(argv[1]);

    args->dir = (char *)calloc(dir_size, sizeof(char));
    if (args->dir == NULL) {
        perror("Calloc");
        status = -1;
        goto EXIT;
    }
    strncpy(args->dir, argv[1], 255);
    printf("Dir input: %s\n", args->dir);

    puts("Here3");

    args->cll = create_cll();
    if (args->cll == NULL) {
        perror("Calloc");
        status = -1;
        goto EXIT;
    }

    puts("Here4");
     
    for (int i = 2; i < argc; i++) {
        printf("Search patterns: %s\n", argv[i]);
    }


EXIT:
    puts("Exiting...");
    return(status);

}