#!/usr/bin/env python

"""
  This python program generates random text that matches a given regex-pattern.
  The patterns are given via sys.argv and the generated text is passed to
  the binary 'tests/test_rand' to check if the generated text also matches
  the regex-pattern in the C implementation.
  The exit-code of the testing program, is used to determine test success.

  This script is called by the Makefile when doing 'make test'
"""


import re
import sys
import exrex
from subprocess import call


prog = "./tests/test_rand"

if len(sys.argv) < 2:
  print("")
  print("usage: %s pattern-file [ntests or 10] [repeat]" % sys.argv[0])
  print("")
  sys.exit(-1)

own_prog = sys.argv[0]
pattern_file = sys.argv[1]
if len(sys.argv) > 2:
  ntests = int(sys.argv[2])
else:
  ntests = 10
nfails = 0
repeats = ntests
old_pattern = ""
if len(sys.argv) > 3:
  repeats = int(sys.argv[3])

sys.stdout.write("Testing patterns against %d random strings matching the Python implementation and comparing::\n" % ntests)

with open(pattern_file, 'rt') as f:
  for line in f:
    parts = line.split('\t')
    pattern = parts[0]
    if pattern == old_pattern:
      break
    old_pattern = pattern
    r = 50
    while r < 0:
      try:
        g = exrex.generate(pattern)
        break
      except:
        pass
    
    sys.stdout.write(" pattern '%s':\n" % pattern)
    
    while repeats > 0:
      try:
        repeats -= 1
        example = exrex.getone(pattern)
        print("%s \"%s\" \"%s\"" % (prog, pattern, example))
        ret = call([prog, "\"%s\"" % pattern, "\"%s\"" % example])
        if ret != 0:
          escaped = repr(example) # escapes special chars for better printing
          print("    FAIL : %s doesn't match %s as expected [%s]." % (pattern, escaped, ", ".join([("0x%02x" % ord(e)) for e in example]) ))
          nfails += 1
    
      except:
        #import traceback
        #print("EXCEPTION!")
        #raw_input(traceback.format_exc())
        ntests -= 1
        repeats = 0
        #nfails += 1

sys.stdout.write("%4d/%d tests succeeded.\n\n" % (ntests - nfails, ntests))
#print("")

