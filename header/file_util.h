/** @file file_Lib.h
 *
 * @brief The header file for file_lib.c.
 */

#ifndef FILELIB_H
#define FILELIB_H

#include "header/includes.h"
#include "header/hash_table.h"

int read_dir(database_i *);

int print_node(database_i *database);

#endif /* FILELIB_H */

/*** end of file ***/