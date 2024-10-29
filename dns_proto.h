#ifndef _DNS_PROTO_
#define _DNS_PROTO_

#include <stdlib.h>
#include <inttypes.h>

#if (__BYTE_ORDER__)==( __ORDER_LITTLE_ENDIAN__)
#define SDNS_BIG_ENDIAN 0
#define SDNS_LITTLE_ENDIAN 1
#else
#define SDNS_BIG_ENDIAN 1
#define SDNS_LITTLE_ENDIAN 0
#endif

/*
	DNS header format definition as per RFC 1035:

      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
typedef struct dns_header_s_ {
	uint16_t id;
#if SDNS_BIG_ENDIAN
	uint8_t qr: 1;      /* query:0, response:1 */
	uint8_t opcode: 4;  /* opcode */
	uint8_t aa: 1;      /* autoritative answer */
	uint8_t tc: 1;      /* truncation */
	uint8_t rd: 1;      /* decursion desired */
#else
	uint8_t rd: 1;
	uint8_t tc: 1;
	uint8_t aa: 1;
	uint8_t opcode: 4;
	uint8_t qr: 1;
#endif

#if SDNS_BIG_ENDIAN
	uint8_t ra: 1;      /* recursion available */
	uint8_t z: 3;       /* reserved */
	uint8_t rcode: 4;   /* response code */
#else
	uint8_t rcode: 4;
	uint8_t z: 3;
	uint8_t ra: 1;
#endif

	uint16_t qdcount;   /* number of questions */
	uint16_t ancount;   /* number o answers */
	uint16_t nscount;   /* number of name server resource */
	uint16_t arcount;   /* additionl resource records */
} __attribute__ ((packed)) dns_header_t;

#define dns_hdr_id(h) ((h)->id)
#define dns_hdr_qr(h) ((h)->qr)
#define dns_hdr_rd(h) ((h)->rd)
#define dns_hdr_ra(h) ((h)->ra)
#define dns_hdr_qdcount(h) ((h)->qdcount)
#define dns_hdr_ancount(h) ((h)->ancount)
#define dns_hdr_nscount(h) ((h)->nscount)
#define dns_hdr_arcount(h) ((h)->arcount)

char *dns_header_tostr (dns_header_t *h);

/*
    DNS question section as per RFC 1035

      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
typedef struct dns_qname_s_ {
    uint8_t len;
    char dname[0];
} __attribute__ ((packed)) dns_qname_t;

typedef struct dns_qtype_code_s_ {
    uint16_t type;
    uint16_t code;
} __attribute__ ((packed)) dns_qtype_code_t;

typedef struct dns_rr_s_ {
    uint16_t name;
    uint16_t type;
    uint16_t cls;
    uint32_t ttl;
    uint16_t rdlen;
    uint8_t rdata[0];
} __attribute__ ((packed)) dns_rr_t;

/* helper macro for debugging purposes */
#if SDNS_DEBUG == 0
#define sdns_print(b,f)
#else
#define sdns_print(b,f) \
do { \
    char *str = (f)(b); \
    slog ("%s", str); \
    free (str); \
}while(0)
#endif

#endif
