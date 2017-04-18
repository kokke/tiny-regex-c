# Compiler to use - can be replaced by clang for instance
CC := gcc

# Number of random text expressions to generate, for random testing
NRAND_TESTS := 1000


# Flags to pass to compiler
CFLAGS := -O0 -Wall -Wextra -std=c99 -I.

all:
	@$(CC) $(CFLAGS) re.c tests/test1.c     -o tests/test1
	@$(CC) $(CFLAGS) re.c tests/test2.c     -o tests/test2
	@$(CC) $(CFLAGS) re.c tests/test_rand.c -o tests/test_rand

clean:
	@rm -f tests/test1 tests/test2 tests/test_rand
	@#@$(foreach test_bin,$(TEST_BINS), rm -f $(test_bin) ; )
	@rm -f a.out
	@rm -f *.o


test: all
	@echo
	@echo Testing hand-picked regex\'s:
	@./tests/test1
	@echo Testing patterns against $(NRAND_TESTS) random strings matching the Python implementation and comparing:
	@echo
	@./scripts/regex_test.py \\d+\\w?\\D\\d         $(NRAND_TESTS)
	@./scripts/regex_test.py \\s+[a-zA-Z0-9?]*      $(NRAND_TESTS)
	@./scripts/regex_test.py \\w*\\d?\\w\\?         $(NRAND_TESTS)
	@#./scripts/regex_test.py [^\\d]+\\\\?\\s        $(NRAND_TESTS)
	@#./scripts/regex_test.py [^\\w][^-1-4]          $(NRAND_TESTS)
	@#./scripts/regex_test.py [^\\w]                 $(NRAND_TESTS)
	@#./scripts/regex_test.py [^1-4]                 $(NRAND_TESTS)
	@#./scripts/regex_test.py [^-1-4]                $(NRAND_TESTS)
	@#./scripts/regex_test.py [^\\d]+\\s?[\\w]*      $(NRAND_TESTS)
	@./scripts/regex_test.py a+b*[ac]*.+.*.[\\.].   $(NRAND_TESTS)
	@./scripts/regex_test.py a?b[ac*]*.?[\\]+[?]?   $(NRAND_TESTS)
	@#./scripts/regex_test.py [1-5-]+[-1-2]-[-]     $(NRAND_TESTS)
	@./scripts/regex_test.py [-1-3]-[-]+            $(NRAND_TESTS)
	@./scripts/regex_test.py [1-5]+[-1-2]-[\\-]     $(NRAND_TESTS)
	@./scripts/regex_test.py [-1-2]*                $(NRAND_TESTS)
	@./scripts/regex_test.py -*                     $(NRAND_TESTS)
	@./scripts/regex_test.py [\\-]*                 $(NRAND_TESTS)
	@./scripts/regex_test.py [\\\\]+                $(NRAND_TESTS)
	@#./scripts/regex_test.py [^-1-4]                $(NRAND_TESTS)
	@echo
	@echo
	@./tests/test2
	@echo
	@echo
	
