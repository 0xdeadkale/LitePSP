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

#include <errno.h>        // For error handling.

#include <unistd.h>       // For a lot of functions.
#include <limits.h>       // Defines many macro's (i.e. PATH_MAX)

#include <fts.h>          // For dir walking.
#include <pthread.h>      // For multithreading.
#include <fcntl.h>        // For changing stdin to non-blocking.
#include <poll.h>         // For polling stdin

/*
#include <getopt.h>       // For cmd-line parsing.
#include <signal.h>       // For signal handling.
#include <sys/signalfd.h> // For signalfd.
#include <errno.h>        // For error handling.
#include <limits.h>       // Defines many macro's (i.e. PATH_MAX)
#include <dirent.h>       // Used for dirent and DIR operations.
#include <unistd.h>       // For a lot of functions.
#include <fcntl.h>        // For function control.
#include <ctype.h>        // For isdigit().
#include <sys/stat.h>     // For stat().
#include <dirent.h>       // For dirent().

#include <pthread.h>      // For multithreading.
#include <sys/epoll.h>    // For epoll() in server loop.
#include <sys/resource.h> // For max number of threads allowed: RLIMIT_NPROC.
#include <arpa/inet.h>    // For inet_ntoa().
#include <netinet/in.h> // For system allowed ports macro (IPPORT_USERRESERVED).
#include <stdint.h>     // Typedef'ed integers (uint*_t). For portability.
#include <sys/socket.h> // Socket specfic functionality. For generating the TCP server.
#include <netdb.h> // Defines getaddrinfo/network macro's (i.e. AI_Passive).
*/


#endif /* INCL_H */

/*** end of file ***/
