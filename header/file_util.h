/** @file file_Lib.h
 *
 * @brief The header file for file_lib.c.
 */

#ifndef FILELIB_H
#define FILELIB_H

#include "header/includes.h"
#include "header/file-finder.h"
#include "header/hash_table.h"

int thread_dispatcher(database_i *);

int assign_job(database_i *);

void *read_dir(void *);

void *user_input(void *);

void *dumper(void *);

#endif /* FILELIB_H */

/*** end of file ***/