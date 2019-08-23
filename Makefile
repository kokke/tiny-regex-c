# Compiler to use - can be replaced by clang for instance
CC := gcc

# Number of random text expressions to generate, for random testing
NRAND_TESTS := 1000

PYTHON != if (python --version 2>&1 | grep -q 'Python 2\..*'); then \
            echo 'python';                                          \
          elif command -v python2 >/dev/null 2>&1; then             \
            echo 'python2';                                         \
          else                                                      \
            echo 'Error: no compatible python version found.' >&2;  \
            exit 1;                                                 \
          fi

# Flags to pass to compiler
CFLAGS := -g -Og -Wall -Wextra -std=c99 -I.

all:
	@$(CC) $(CFLAGS) re.c tests/perf.c -o tests/perf
	@$(CC) $(CFLAGS) re.c tests/fixed.c -o tests/fixed
	@$(CC) $(CFLAGS) re.c tests/print.c -o tests/print
	@$(CC) $(CFLAGS) re.c tests/rand.c -o tests/rand -lpcre2-8
	@$(CC) $(CFLAGS) re.c example.c -o example

clean:
	@rm -f tests/test1 tests/test2 tests/test_rand example


test: all
	@$(test $(PYTHON))
	@echo
	@echo Fixed Tests:
	@tests/fixed
	@echo Random Tests:
	@tests/rand
	@echo Performance Test:
	@tests/perf

