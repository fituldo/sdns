
BIN_OUT=sdns

SRC=main.c          \
	sdns_server.c   \
	dns_proto.c     \

TEST_SRC=test/test_sdns.c

all:
	@echo
	@echo "Targets:"
	@echo "  build   - build DNS server (with debug symbols)"
	@echo "  check   - run unit tests"
	@echo "  cscope  - generate cscope tags"
	@echo

build_sdns:
	gcc -g -DSDNS_DEBUG=0 -Wunused -Wall $(SRC) -lc -o $(BIN_OUT)

build_tests:
	gcc -g -I.. $(TEST_SRC) sdns_server.c -o test/test_sdns

build: build_sdns build_tests

run:
	@./$(BIN_OUT)

check:
	@./test/test_sdns

cscope:
	find . -name '*.[ch]' > cscope.files
	cscope -b -q
