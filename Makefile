# Compiler to use - can be replaced by clang for instance
CC := gcc
# Flags to pass to compiler
CFLAGS := -O3 -Wall -Wextra -std=c99 -I.
#CFLAGS := -g -Wall -Wextra -std=c99 -I.

# Number of random text expressions to generate, for random testing
NRAND_TESTS := 1000

PYTHON != if (python --version 2>&1 | grep -q 'Python 3\..*'); then \
            echo 'python';                                          \
          elif command -v python3 >/dev/null 2>&1; then             \
            echo 'python3';                                         \
          else                                                      \
            echo 'Error: no compatible python 3 version found.' >&2;  \
            exit 1;                                                 \
          fi
TEST_BINS = tests/test1 tests/test2 tests/test_compile tests/test_rand tests/test_rand_neg

all: $(TEST_BINS) 

tests/test1: re.c tests/test1.c
	@$(CC) $(CFLAGS) re.c tests/test1.c         -o $@
tests/test2: re.c tests/test2.c
	@$(CC) $(CFLAGS) re.c tests/test2.c         -o $@
tests/test_compile: re.c tests/test_compile.c
	@$(CC) $(CFLAGS) re.c tests/test_compile.c  -o $@
tests/test_rand: re.c tests/test_rand.c
	@$(CC) $(CFLAGS) re.c tests/test_rand.c     -o $@
tests/test_rand_neg: re.c tests/test_rand_neg.c
	@$(CC) $(CFLAGS) re.c tests/test_rand_neg.c -o $@

clean:
	@rm -f $(TEST_BINS)
	@rm -f a.out
	@rm -f *.o

test-pyok: tests/test_rand
	@$(test $(PYTHON))
	@echo
	@echo Testing patterns against $(NRAND_TESTS) random strings matching the Python implementation and comparing:
	@echo
	@$(PYTHON) ./scripts/regex_test.py tests/ok.lst $(NRAND_TESTS)
	@echo

test-pynok: tests/test_rand_neg
	@$(test $(PYTHON))
	@echo
	@echo Testing rejection of patterns against $(NRAND_TESTS) random strings also rejected by the Python implementation:
	@echo
	@$(PYTHON) ./scripts/regex_test_neg.py tests/nok.lst $(NRAND_TESTS)
	@echo

test: all 
	@echo Testing hand-picked regex\'s:
	@./tests/test1
	@echo
	$(MAKE) test-pyok
	@echo
	$(MAKE) test-pynok
	@echo
	@echo Testing handling of invalid regex patterns
	@./tests/test_compile
	@echo
	@./tests/test2
	@echo

CBMC := cbmc

# unwindset: loop max MAX_REGEXP_OBJECTS patterns
verify:
	$(CBMC) -DCPROVER --unwindset 8 --unwind 16 --depth 16 --bounds-check --pointer-check --memory-leak-check --div-by-zero-check --signed-overflow-check --unsigned-overflow-check --pointer-overflow-check --conversion-check --undefined-shift-check --enum-range-check $(CBMC_ARGS) re.c
