#!/usr/bin/env python

"""
This python program verifies `scripts/regex_compile.py` generates the same compiled regex as re.c.
The pattern is given via sys.argv and the compiled hex pattern is passed to
the binary 'tests/test_compile' to check if the compiled hex pattern is
the same as the one compiled in the C code.
The exit code of the testing program determines test success.

This script is called by the Makefile when doing 'make test'.
"""

import binascii
import subprocess
import sys

from regex_compile import MiniRegexCompiler, to_buffer
from utils import get_executable_name

prog = get_executable_name("./tests/test_compile")

if len(sys.argv) < 2:
    print(f"\nusage: {sys.argv[0]} pattern\n\n")
    sys.exit(-1)

pattern = sys.argv[1]

print(f"  pattern '{pattern}': ", end='')

compiler = MiniRegexCompiler()
segments = compiler.compile(pattern)
hex_pattern = ''
if segments:
    compiled_pattern = to_buffer(segments)
    hex_pattern = binascii.hexlify(compiled_pattern)

ret = subprocess.call([prog, pattern, hex_pattern], shell=False)
if ret != 0:
    print("Compiled pattern:", compiled_pattern)
    for segment in segments:
        print(segment)
    print(f"    FAIL: {pattern}")
    print(hex_pattern)
    sys.exit(1)
else:
    print("SUCCEED")
