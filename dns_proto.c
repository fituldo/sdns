#include <string.h>
#include <arpa/inet.h>
#include "dns_proto.h"
#include "log.h"

char *
dns_header_tostr (dns_header_t *h)
{
    /* allocate enough memory; debugging purposes only */
    char *s = malloc(256);
    s[0] = 0;
    slog("sizeof(hdr) %ld", sizeof (dns_header_t));
    slog("id 0x%x", ntohs(dns_hdr_id(h)));
    slog(" flags: qr:%d opcode:%d aa:%d tc:%d rd:%d",
            h->qr, h->opcode, h->aa, h->tc, h->rd);
    slog("qd count: %d", ntohs(dns_hdr_qdcount(h)));
    slog("an count: %d", ntohs(dns_hdr_ancount(h)));
    slog("ns count: %d", ntohs(dns_hdr_nscount(h)));
    slog("ar count: %d", ntohs(dns_hdr_arcount(h)));
    return s;
}
