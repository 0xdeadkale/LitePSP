/** @file file_Lib.h
 *
 * @brief The header file for file_lib.c.
 */

#ifndef FILELIB_H
#define FILELIB_H

#include "header/includes.h"
#include "header/hash_table.h"

int read_dir(database_i *database, char *path);

/**
 * @brief This function obtains the attributes of a file and prints to console.
 *
 * @param name This is the name of the file to be scanned.
 * @param buffer Used to return the attributes obtained from the file.
 *
 * @return Status code on success, GENERIC_FAIL error code on failure.
 */
int get_attributes(char *name, struct stat *buffer);

#endif /* FILELIB_H */

/*** end of file ***/