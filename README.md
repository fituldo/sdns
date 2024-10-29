# Simple DNS spoof server

Simple DNS spoofing server running on UDP that returns fixed IP4 address for any A record.

# Building

    $ make
    $ make check
    $ sudo ./sdns


# Example run
	$ dig @127.0.0.1  example.com

	; <<>> DiG 9.18.28-1~deb12u2-Debian <<>> @127.0.0.1 example.com
	; (1 server found)
	;; global options: +cmd
	;; Got answer:
	;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 64654
	;; flags: qr rd ad; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0
	;; WARNING: recursion requested but not available

	;; QUESTION SECTION:
	;example.com.                   IN      A

	;; ANSWER SECTION:
	example.com.            300     IN      A       6.6.6.6

	;; Query time: 0 msec
	;; SERVER: 127.0.0.1#53(127.0.0.1) (UDP)
	;; WHEN: Tue Oct 29 16:44:17 EDT 2024
	;; MSG SIZE  rcvd: 45

