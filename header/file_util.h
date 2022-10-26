/** @file file_Lib.h
 *
 * @brief The header file for file_lib.c.
 */

#ifndef FILELIB_H
#define FILELIB_H

#include "header/includes.h"
#include "header/file-finder.h"
#include "header/hash_table.h"

int thread_dispatcher(database_i *database);

void *user_input(void *varg_p);

void *read_dir(void *varg_p);

void *dumper(void *database);

#endif /* FILELIB_H */

/*** end of file ***/