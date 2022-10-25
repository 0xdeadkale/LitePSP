/** @file parser.h
 *
 * @brief The private header file for parser.c.
 */

#ifndef PARSER_H
#define PARSER_H

#include "includes.h"
#include "hash_table.h"
#include "file_util.h"

volatile __sig_atomic_t exit_flag = false;
volatile __sig_atomic_t dump_flag = false;

pthread_mutex_t database_lock = PTHREAD_MUTEX_INITIALIZER;

void create_sig_handler(int signum, void (*func)(int));

#endif /* PARSER_H */

/*** end of file ***/