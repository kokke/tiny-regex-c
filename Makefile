# Compiler to use - can be replaced by clang for instance
CC := gcc

# Number of random text expressions to generate, for random testing
NRAND_TESTS := 1000

PYTHON := python

# Flags to pass to compiler
CFLAGS := -O3 -Wall -Wextra -std=c99 -I.

all:
	@$(CC) $(CFLAGS) re.c tests/test1.c           -o tests/test1
	@$(CC) $(CFLAGS) re.c tests/test2.c           -o tests/test2
	@$(CC) $(CFLAGS) re.c tests/test_rand.c       -o tests/test_rand
	@$(CC) $(CFLAGS) re.c tests/test_rand_neg.c   -o tests/test_rand_neg
	@$(CC) $(CFLAGS) re.c tests/test_compile.c    -o tests/test_compile
	@$(CC) $(CFLAGS) re.c tests/test_end_anchor.c -o tests/test_end_anchor

clean:
	@rm -f tests/test1 tests/test2 tests/test_rand tests/test_compile tests/test_end_anchor
	@#@$(foreach test_bin,$(TEST_BINS), rm -f $(test_bin) ; )
	@rm -f a.out
	@rm -f *.o


test: all
	@$(test $(PYTHON))
	@echo
	@echo Testing hand-picked regex\'s:
	@./tests/test1
	@echo Testing handling of invalid regex patterns
	@./tests/test_compile
	@echo Compiling patterns in both Python and C and verifying the results are the same:
	@echo
	@$(PYTHON) ./scripts/regex_test_compile.py \\d+\\w?\\D\\d
	@$(PYTHON) ./scripts/regex_test_compile.py \\s+[a-zA-Z0-9?]*
	@$(PYTHON) ./scripts/regex_test_compile.py \\w*\\d?\\w\\?
	@$(PYTHON) ./scripts/regex_test_compile.py [^\\d]+\\\\?\\s
	@$(PYTHON) ./scripts/regex_test_compile.py [^\\w][^-1-4]
	@$(PYTHON) ./scripts/regex_test_compile.py [^\\w]
	@$(PYTHON) ./scripts/regex_test_compile.py [^1-4]
	@$(PYTHON) ./scripts/regex_test_compile.py [^-1-4]
	@$(PYTHON) ./scripts/regex_test_compile.py [^\\d]+\\s?[\\w]*
	@$(PYTHON) ./scripts/regex_test_compile.py a+b*[ac]*.+.*.[\\.].
	@$(PYTHON) ./scripts/regex_test_compile.py a?b[ac*]*.?[\\]+[?]?
	@$(PYTHON) ./scripts/regex_test_compile.py [1-5-]+[-1-2]-[-]
	@$(PYTHON) ./scripts/regex_test_compile.py [-1-3]-[-]+
	@$(PYTHON) ./scripts/regex_test_compile.py [1-5]+[-1-2]-[\\-]
	@$(PYTHON) ./scripts/regex_test_compile.py [-1-2]*
	@$(PYTHON) ./scripts/regex_test_compile.py \\s?[a-fKL098]+-?
	@$(PYTHON) ./scripts/regex_test_compile.py [\\-]*
	@$(PYTHON) ./scripts/regex_test_compile.py [\\\\]+
	@$(PYTHON) ./scripts/regex_test_compile.py [0-9a-fA-F]+
	@$(PYTHON) ./scripts/regex_test_compile.py [1379][2468][abcdef]
	@$(PYTHON) ./scripts/regex_test_compile.py [012345-9]?[0123-789]
	@$(PYTHON) ./scripts/regex_test_compile.py [012345-9]
	@$(PYTHON) ./scripts/regex_test_compile.py [0-56789]
	@$(PYTHON) ./scripts/regex_test_compile.py [abc-zABC-Z]
	@$(PYTHON) ./scripts/regex_test_compile.py [a\d]?1234
	@$(PYTHON) ./scripts/regex_test_compile.py .*123faerdig
	@$(PYTHON) ./scripts/regex_test_compile.py .?\\w+jsj$
	@$(PYTHON) ./scripts/regex_test_compile.py [?to][+to][?ta][*ta]
	@$(PYTHON) ./scripts/regex_test_compile.py \\d+
	@$(PYTHON) ./scripts/regex_test_compile.py [a-z]+
	@$(PYTHON) ./scripts/regex_test_compile.py \\s+[a-zA-Z0-9?]*
	@$(PYTHON) ./scripts/regex_test_compile.py \\w
	@$(PYTHON) ./scripts/regex_test_compile.py \\d
	@$(PYTHON) ./scripts/regex_test_compile.py [\\d]
	@$(PYTHON) ./scripts/regex_test_compile.py [^\\d]
	@$(PYTHON) ./scripts/regex_test_compile.py [^-1-4]
	@$(PYTHON) ./scripts/regex_test_compile.py \\x01[^\\xff][^
	@$(PYTHON) ./scripts/regex_test_compile.py \\x01[^\\xff][\
	@echo
	@echo
	@echo
	@echo Testing patterns against $(NRAND_TESTS) random strings matching the Python implementation and comparing:
	@echo
	@$(PYTHON) ./scripts/regex_test.py \\d+\\w?\\D\\d             $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\s+[a-zA-Z0-9?]*          $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\w*\\d?\\w\\?             $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^\\d]+\\\\?\\s            $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^\\w][^-1-4]              $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^\\w]                     $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^1-4]                     $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^-1-4]                    $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^\\d]+\\s?[\\w]*          $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py a+b*[ac]*.+.*.[\\.].       $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py a?b[ac*]*.?[\\]+[?]?       $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [1-5-]+[-1-2]-[-]          $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [-1-3]-[-]+                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [1-5]+[-1-2]-[\\-]         $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [-1-2]*                    $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\s?[a-fKL098]+-?          $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [\\-]*                     $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [\\\\]+                    $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [0-9a-fA-F]+               $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [1379][2468][abcdef]       $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [012345-9]?[0123-789]      $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [012345-9]                 $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [0-56789]                  $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [abc-zABC-Z]               $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [a\d]?1234                 $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py .*123faerdig               $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py .?\\w+jsj$                 $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [?to][+to][?ta][*ta]       $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\d+                       $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [a-z]+                     $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\s+[a-zA-Z0-9?]*          $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\w                        $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py \\d                        $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [\\d]                      $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^\\d]                     $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test.py [^-1-4]                    $(NRAND_TESTS)
	@echo
	@echo
	@echo
	@echo Testing rejection of patterns against $(NRAND_TESTS) random strings also rejected by the Python implementation:
	@echo
	@$(PYTHON) ./scripts/regex_test_neg.py \\d+                   $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [a-z]+                 $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py \\s+[a-zA-Z0-9?]*      $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py ^\\w                   $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py ^\\d                   $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [\\d]                  $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py ^[^\\d]                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [^\\w]+                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py ^[\\w]+                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py ^[^0-9]                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [a-z].[A-Z]            $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [-1-3]-[-]+            $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [1-5]+[-1-2]-[\\-]     $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [-0-9]+                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [\\-]+                 $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [\\\\]+                $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [0-9a-fA-F]+           $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [1379][2468][abcdef]   $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [012345-9]             $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py [0-56789]              $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py .*123faerdig           $(NRAND_TESTS)
	@$(PYTHON) ./scripts/regex_test_neg.py a^                     $(NRAND_TESTS)
	@echo
	@echo
	@./tests/test2
	@echo
	@echo
	@echo
	@echo
	@./tests/test_end_anchor
	@echo
	@echo

CBMC := cbmc

# unwindset: loop max MAX_REGEXP_OBJECTS patterns
verify:
	$(CBMC) -DCPROVER --unwindset 8 --unwind 16 --depth 16 --bounds-check --pointer-check --memory-leak-check --div-by-zero-check --signed-overflow-check --unsigned-overflow-check --pointer-overflow-check --conversion-check --undefined-shift-check --enum-range-check $(CBMC_ARGS) re.c
