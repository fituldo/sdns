#include <stdio.h>

#include "sdns_server.h"

#define DNS_UDP_LISTEN_PORT     "53"

int
main (int argc, char *argv[])
{
    int rc;

    /* bind and listen on specific UDP port */
    rc = sdns_serve (DNS_UDP_LISTEN_PORT);

    return rc;
}
