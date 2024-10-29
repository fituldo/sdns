#ifndef _SDNS_SERVER_
#define _SDNS_SERVER_

#include <string.h>

/* buffer strucure used to simplify packet parsing */
typedef struct sdns_buffer_s_ {
    char *data;         /* beginning of dns message */
    char *curr;         /* current position within the message */
    size_t total_len;   /* lenght of dns message */
    size_t rem_len;     /* remaining length from current poostition; used for parsing */
    size_t max_cap;     /* buffer capacity */
} sdns_buffer_t;

#define buffer_data(b) (b)->data

/* can be used as l-value */
#define buffer_curr_data_get(b) (b)->curr
#define buffer_curr_data(b) (void *) ((b)->curr)
#define buffer_rem_len(b) (b)->rem_len
#define buffer_max_cap(b) (b)->max_cap

/* return true if there is enough space to read/advance current buffer pointer */
static inline int
buffer_has_more_data (sdns_buffer_t *b, int len)
{
    return buffer_rem_len (b) >= len;
}

/* return true if there is enough space in the buffer to write len bytes */
static inline int
buffer_has_write_space (sdns_buffer_t *b, int len)
{
    return buffer_max_cap (b) - (buffer_curr_data_get (b) - buffer_data (b)) >= len;
}

#define buffer_advance(b,len) \
do { \
    b->curr += len; \
    buffer_rem_len(b) -= len; \
}while(0)

/* Append len bytes at the current buffer position.
 * If successful it advances current pointer and returns 0,
 * otherwise returns -1
 */
static inline int
sdns_buffer_put (sdns_buffer_t *b, void *data, int len)
{
    if (!buffer_has_write_space (b, len))
        return -1;

    memcpy (buffer_curr_data (b), data, len);
    buffer_advance (b, len);
    return 0;
}

static inline void
sdns_buffer_init (sdns_buffer_t *b, char *data, size_t len, size_t max_buffer_size)
{
    b->data = b->curr = data;
    b->total_len = b->rem_len = len;
    b->max_cap = max_buffer_size;
}

/* main logic of SDNS server */
int sdns_serve (char *port);
int sdns_dispatch_msg (char *msg, ssize_t *msg_len, size_t max_buffer_size);

#endif
