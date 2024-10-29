#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "sdns_server.h"
#include "dns_proto.h"
#include "log.h"

/* current buffer size for received DNS messages */
#define BUF_SIZE 1024

/* Create linux socket and bind it to specific address and port */
static int
create_and_bind (char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int rc, sfd;
    int yes = 1;

    memset (&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* ipv4 support only */
    hints.ai_socktype = SOCK_DGRAM; /* use udp datagram */
    hints.ai_flags = AI_PASSIVE; /* All interfaces */

    rc = getaddrinfo (NULL, port, &hints, &result);
    if (rc < 0) {
        slog ("getaddrinfo: %s", gai_strerror (rc));
        return -1;
    }

    /* try to find any usable address */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        }

        rc = setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (rc == -1) {
            slog ("setsockopt");
            return -1;
        }

        rc = bind (sfd, rp->ai_addr, rp->ai_addrlen);
        if (rc == 0) {
            /* manged to bind successfully */
            break;
        }
        close (sfd);
    }

    if (rp == NULL) {
        slog ("Could not bind!");
        return -1;
    }
    freeaddrinfo (result);
    return sfd;
}

/* helper macro for checking if it's possible to advance buffer pointer;
 * if not log error and return */
#define buffer_advance_try(b, adv)                                  \
    do{                                                             \
        int _adv = (int)adv;                                        \
        if (buffer_has_more_data(b, _adv)) {                        \
            buffer_advance(b, _adv);                                \
        } else {                                                    \
            slog("cannot advance buffer by %d bytes! curr:%ld",     \
                _adv, buffer_curr_data_get (b) - buffer_data (b));  \
            rc = -1;                                                \
            goto cleanup;                                           \
        }                                                           \
    } while(0)

/* Main function that parses the request and creates a reply with
 * fixed IP4 address.
 * The same buffer is used for DNS reply message.
 */
int
sdns_dispatch_msg (char *msg, ssize_t *msg_len, size_t max_buffer_size)
{
    sdns_buffer_t _b = {0}, *b = &_b;
    sdns_buffer_init (b, msg, msg_len[0], max_buffer_size);
    dns_header_t *h = buffer_curr_data (b);
    int *offsets = 0; /* array of offsets of domain names */
    int n_offsets = 0;
    int rc = 0;

    sdns_print (h, dns_header_tostr);

    /* make sure this is a DNS request */
    if (dns_hdr_qr (h) != 0) {
        slog ("unexpected query bit");
        return -1;
    }

    /* make a reply */
    dns_hdr_qr (h) = 1;

    /* no support for recursive service for now */
    if (dns_hdr_rd (h)) {
        dns_hdr_ra (h) = 0;
    }

    /* update current position */
    buffer_advance_try (b, sizeof (dns_header_t));

    /* remember offsets for each question from in order to use them
     * in answer section */
    uint16_t qdcount = ntohs (dns_hdr_qdcount (h));
    offsets = calloc (qdcount, sizeof (int));
    if (!offsets) {
        slog ("allocation error");
        return -1;
    }

    for (int i = 0; i < qdcount; i++) {
        offsets[n_offsets++] =
            (int)(buffer_curr_data_get (b) - buffer_data (b));

        /* skip to the next question */
        dns_qname_t *qh = buffer_curr_data (b);
        while (qh->len) {
            /* +1 for the length byte */
            buffer_advance_try (b, qh->len + 1);
            qh = buffer_curr_data (b);
        }
        buffer_advance_try (b, 1); /* consume 0 byte */
    }

    /* skip qtype and code fields */
    buffer_advance_try (b, sizeof (dns_qtype_code_t));

    /* ignore all other RR entries and write answer section directly */

    /* update actual length of dns reply message */
    msg_len[0] = buffer_curr_data_get (b) - buffer_data (b);

    /* update number of answers */
    dns_hdr_ancount (h) = dns_hdr_qdcount (h);

    /* clear sections we are not interested in */
    dns_hdr_nscount (h) = 0;
    dns_hdr_arcount (h) = 0;

    /* prepare a template answer section for each answer */
    char ipv4[4] = { 6, 6, 6, 6 }; /* same address for each A record */
    dns_rr_t rr = { 0 };
    rr.type = htons (1);    /* A record */
    rr.cls = htons (1);     /* IPv4 address */
    rr.ttl = htonl (300);   /* 5 mins */
    rr.rdlen = htons (sizeof (ipv4)); /* ipv4 address length */
    int rr_len = sizeof (dns_rr_t) + sizeof (ipv4);

    for (int i = 0; i < n_offsets; i++) {
        /* use offsets for names */
        rr.name = htons(3 << 14 | offsets[i]);

        rc = sdns_buffer_put (b, &rr, rr_len);
        rc += sdns_buffer_put (b, ipv4, sizeof (ipv4));
        if (rc) {
            slog ("buffer too small for reply msg");
            goto cleanup;
        }

        msg_len[0] += rr_len;
    }

cleanup:

    if (offsets)
        free (offsets);

    return rc;
}

int
sdns_serve (char *port)
{
    char host[NI_MAXHOST], service[NI_MAXSERV];
    struct sockaddr_storage  peer_addr;
    socklen_t peer_addrlen;
    int sfd, rc;
    char *buf[BUF_SIZE];
    ssize_t nread;

    sfd = create_and_bind (port);
    if (sfd == -1) {
        return -1;
    }

    slog ("Spoofing DNS server listening on :%s", port);

    /* main dispatch loop */
    for (;;) {
        /* blocking read from UDP socket */
        peer_addrlen = sizeof(peer_addr);
        nread = recvfrom (sfd, buf, BUF_SIZE, 0,
                (struct sockaddr *) &peer_addr, &peer_addrlen);

        if (nread == -1)
            continue;

        rc = getnameinfo ((struct sockaddr *) &peer_addr, peer_addrlen,
                host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
		if (rc != 0) {
            slog ("getnameinfo: %s", gai_strerror(rc));
        }

        /* process the request */
        rc = sdns_dispatch_msg ((char *)buf, &nread, BUF_SIZE);

        if (rc < 0) {
            slog ("error on parsing dns message");
            continue;
        }

        /* dns request was processed successfuly, reply is ready
         * in the same buffer */
        if (sendto (sfd, buf, nread, 0, (struct sockaddr *) &peer_addr,
                    peer_addrlen) != nread) {
            slog ("Error sending response");
        }
    }
}
