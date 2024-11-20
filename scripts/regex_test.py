#!/usr/bin/env python

"""
  This program generates random text that matches a given regex-pattern.
  The pattern is given via sys.argv and the generated text is passed to
  the binary 'tests/test_rand' to check if the generated text also matches
  the regex-pattern in the C implementation.
  The exit-code of the testing program, is used to determine test success.

  This script is called by the Makefile when doing 'make test'
"""


import re
import sys
import exrex
from subprocess import call


def get_one(pattern):
  """Ensure that Python's re-module agrees the example matches the pattern"""
  while True:
    p = exrex.getone(pattern)
    m = re.match(pattern, p)
    if m:
      return p

prog = "./tests/test_rand"

if len(sys.argv) < 2:
  print("")
  print("usage: %s pattern [nrepeat]" % sys.argv[0])
  print("  where [nrepeat] is optional")
  print("")
  sys.exit(-1)

own_prog = sys.argv[0]
pattern = sys.argv[1]
if len(sys.argv) > 2:
  ntests = int(sys.argv[2])
else:
  ntests = 10
nfails = 0
repeats = ntests


try:
  repeats = int(sys.argv[2])
except:
  pass


sys.stdout.write("%-35s" % ("  pattern '%s': " % pattern))


while repeats >= 0:
  try:
    repeats -= 1
    example = get_one(pattern)
    #print("%s %s %s" % (prog, pattern, example))
    ret = call([prog, pattern, example])
    if ret != 0:
      escaped = repr(example) # escapes special chars for better printing
      print("    FAIL : doesn't match %s as expected [%s]." % (escaped, ", ".join([("0x%02x" % ord(e)) for e in example]) ))
      nfails += 1

  except:
    import traceback
    print("EXCEPTION!", traceback.format_exc())
    ntests -= 1
    repeats += 1
    #nfails += 1

sys.stdout.write("%4d/%d tests succeeded \n" % (ntests - nfails, ntests))
#print("")

