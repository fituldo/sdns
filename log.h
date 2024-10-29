#ifndef _SDNS_LOG_
#define _SDNS_LOG_

#include <stdio.h>

#define slog(fmt, args...) fprintf(stderr, fmt "\n", ##args)

#endif
