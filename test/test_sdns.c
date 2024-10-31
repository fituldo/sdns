/* unit test for sdns message dispatch function */
#include <stdio.h>
#include <assert.h>
#include "../sdns_server.h"

#define MAX_BUF 1024

char rq[MAX_BUF] = {
        0xd9, 0x1d, /* id */
        0x01, 0x20, /* flags */
        0x00, 0x01, /* qdcount */
        0x00, 0x00, /* ancount */
        0x00, 0x00, /* nscount */
        0x00, 0x01, /* arcount */
        0x07, /* qlen */
        0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, /* example */
        0x04, 0x74, 0x65, 0x63, 0x68, /* tech */

        /* ignored */
        0x00, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x00, 0x29, 0x04,
        0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0a, 0x00, 0x08,
        0x48, 0xbd, 0xfc, 0x23, 0xcc,
        0x90, 0x5c, 0xb1
};


/* simple test with 1 question */
int test_expected_answer ()
{
    size_t msg_len = 51;
    int rc = sdns_dispatch_msg (rq, &msg_len, MAX_BUF);
    assert(rc == 0);
    assert(rq[0] == (char)0xd9); // no id change expected
    assert(rq[2] == (char)0x81); // expect response flag set

    int ip_offset = 41;
    assert(rq[ip_offset] == (char)0x04); // rlen = 4 (ip4 address size)
    assert(rq[ip_offset+1] == (char)0x06); // check ip address
    assert(rq[ip_offset+2] == (char)0x06);
    assert(rq[ip_offset+3] == (char)0x06);
    assert(rq[ip_offset+4] == (char)0x06);

    return 0;
}

#define run_test(tc)                \
    do {                            \
        printf("test "#tc "...  "); \
        assert(tc() == 0);          \
        printf("OK!\n");            \
    }while(0);


int main ()
{
    run_test(test_expected_answer);
    return 0;
}
