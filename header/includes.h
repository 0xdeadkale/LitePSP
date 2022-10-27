/** @file includes.h
 *
 * @brief Organizes all included libraries in one place.
 */

#ifndef INCL_H
#define INCL_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <signal.h>       // For signal handling.
#include <errno.h>        // For error handling.

#include <fts.h>          // For dir walking.
#include <poll.h>         // For polling stdin
#include <pthread.h>      // For multithreading.
#include <fcntl.h>        // For changing stdin to non-blocking.

#include <unistd.h>       // For a lot of functions.
#include <limits.h>       // Defines many macro's (i.e. PATH_MAX)

#endif /* INCL_H */

/*** end of file ***/
