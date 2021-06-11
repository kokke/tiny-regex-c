# Using KLEE for formal verification

Here is a crude demo of formal verification of tiny-regex. This is a hefty plagiat of [@DavidKorczynski](https://twitter.com/davkorcz/) - see https://www.youtube.com/watch?v=z6bsk-lsk1Q or [#44](https://github.com/kokke/tiny-regex-c/issues/44) for more context.

I am using the [KLEE Symbolic Execution Engine](https://klee.github.io/) and their Docker image here on a Debian-based host.

What this does, is mechanically try to prove the abscence of all run-time errors, memory corruption bugs and other problems by symbolic execution. We mark the inputs as being symbolic, so that the tool knows to use that as the "search space". That means KLEE checks all possible inputs of the form we give it.

Steps:

- Get the KLEE Docker image: ` $ sudo docker pull klee/klee `
- Run the KLEE Docker image: ` $ sudo docker run --rm -ti --ulimit='stack=-1:-1' klee/klee `
- NOTE: You should see a command prompt like this: ` klee@cc0c26c5b84c:~$ `
- Fetch `re.h`: ` klee@cc0c26c5b84c:~$ wget https://raw.githubusercontent.com/kokke/tiny-regex-c/master/re.h `
- Fetch `re.c`: ` klee@cc0c26c5b84c:~$ wget https://raw.githubusercontent.com/kokke/tiny-regex-c/master/re.c `
- Run your favorite editor, and insert the code below in the bottom of `re.c`
```C
/*
tiny-regex KLEE test driver
kindly contributed by @DavidKorczynski - see https://github.com/kokke/tiny-regex-c/issues/44
*/

int main(int argc, char* argv[])
{
  /* test input - ten chars used as a regex-pattern input */
  char arr[10];

  /* make input symbolic, to search all paths through the code */
  /* i.e. the input is checked for all possible ten-char combinations */
  klee_make_symbolic(arr, sizeof(arr), "arr"); 

  /* assume proper NULL termination */
  klee_assume(arr[sizeof(arr) - 1] == 0);

  /* verify abscence of run-time errors - go! */
  re_compile(arr);

  return 0;
}
```
- Alternatively, run this command:
` klee@cc0c26c5b84c:~$ echo "int main(int argc,char* argv[]){ char arr[10]; klee_make_symbolic(arr, sizeof(arr), \"arr\"); klee_assume(arr[sizeof(arr)-1] == 0); re_compile(arr); return 0; }" >> re.c `
- Compile and emit LLVM bitcode: ` klee@cc0c26c5b84c:~$ clang -emit-llvm -g -c -O0 -Xclang -disable-O0-optnone re.c ` [(NOTE: flags passed to clang are the ones "recommended" by the KLEE project)](https://klee.github.io/tutorials/testing-function/)
- Run KLEE and wait for 5-10 minutes: ` klee@cc0c26c5b84c:~$ klee --libc=uclibc re.bc `
- A positive result looks like this:
```
klee@cc0c26c5b84c:~$ klee --libc=uclibc re.bc
KLEE: NOTE: Using klee-uclibc : /tmp/klee_build90stp_z3/runtime/lib/klee-uclibc.bca
KLEE: output directory is "/home/klee/klee-out-3"
KLEE: Using STP solver backend
warning: Linking two modules of different target triples: re.bc' is 'x86_64-unknown-linux-gnu' whereas '__uClibc_main.os' is 'x86_64-pc-linux-gnu'

KLEE: WARNING: undefined reference to function: __syscall_rt_sigaction
KLEE: WARNING: undefined reference to function: close
KLEE: WARNING: undefined reference to function: fcntl
KLEE: WARNING: undefined reference to function: fstat
KLEE: WARNING: undefined reference to function: ioctl
KLEE: WARNING: undefined reference to function: lseek64
KLEE: WARNING: undefined reference to function: mkdir
KLEE: WARNING: undefined reference to function: open
KLEE: WARNING: undefined reference to function: open64
KLEE: WARNING: undefined reference to function: read
KLEE: WARNING: undefined reference to function: sigprocmask
KLEE: WARNING: undefined reference to function: stat
KLEE: WARNING: undefined reference to function: write
KLEE: WARNING: undefined reference to function: kill (UNSAFE)!
KLEE: WARNING: executable has module level assembly (ignoring)
KLEE: WARNING ONCE: calling external: ioctl(0, 21505, 94666720729472) at libc/termios/tcgetattr.c:43 12
KLEE: WARNING ONCE: calling __user_main with extra arguments.
KLEE: WARNING ONCE: skipping fork (memory cap exceeded)
KLEE: WARNING: killing 12290 states (over memory cap: 2102MB)
KLEE: WARNING: killing 11467 states (over memory cap: 2101MB)

KLEE: done: total instructions = 104365773
KLEE: done: completed paths = 801298
KLEE: done: generated tests = 801298
klee@cc0c26c5b84c:~$ 
```

Similarly, the code below tests both `re_compile(...)` and `re_match(...)` which should be sufficient for coverage of the core logic.
Depending on your hardware, you should be able to increase the sizes of `pat` and `txt` to increase your confidence in the verification.


```C
/*
tiny-regex KLEE test driver
kindly contributed by @DavidKorczynski - see https://github.com/kokke/tiny-regex-c/issues/44
*/

int main(int argc, char* argv[])
{
  /* test input - a regex-pattern and a text string to search in */
  char pat[7];
  char txt[3];

  /* make input symbolic, to search all paths through the code */
  /* i.e. the input is checked for all possible ten-char combinations */
  klee_make_symbolic(pat, sizeof(pat), "pat"); 
  klee_make_symbolic(txt, sizeof(txt), "txt"); 

  /* assume proper NULL termination */
  klee_assume(pat[sizeof(pat) - 1] == 0);
  klee_assume(txt[sizeof(txt) - 1] == 0);

  /* verify abscence of run-time errors - go! */
  int l;
  re_match(pat, txt, &l);

  return 0;
}
```

My modest hardware (T420/i5-2520M@2.5GHz/8GB) completes a check of a 7-char pattern and a 3-char text string in 20-30 minutes (size includes null-termination), whereas 8/5 takes +8 hours, 8/6 takes 14 hours:

```
klee@780432c1aaae0:~$ clang -emit-llvm -g -c -O0 -Xclang -disable-O0-optnone re.c
klee@780432c1aaae0:~$ time klee --libc=uclibc --optimize re.bc
KLEE: NOTE: Using klee-uclibc : /tmp/klee_build90stp_z3/runtime/lib/klee-uclibc.bca
KLEE: output directory is "/home/klee/klee-out-0"
KLEE: Using STP solver backend
warning: Linking two modules of different target triples: re.bc' is 'x86_64-unknown-linux-gnu' whereas '__uClibc_main.os' is 'x86_64-pc-linux-gnu'

KLEE: WARNING: undefined reference to function: fcntl
KLEE: WARNING: undefined reference to function: fstat
KLEE: WARNING: undefined reference to function: ioctl
KLEE: WARNING: undefined reference to function: open
KLEE: WARNING: undefined reference to function: write
KLEE: WARNING: executable has module level assembly (ignoring)
KLEE: WARNING ONCE: calling external: ioctl(0, 21505, 94248844458320) at libc/termios/tcgetattr:43 12
KLEE: WARNING ONCE: calling __user_main with extra arguments.
KLEE: WARNING ONCE: skipping fork (memory cap exceeded)

KLEE: done: total instructions = 201292178
KLEE: done: completed paths = 910249
KLEE: done: generated tests = 910249

real    29m16.633s
user    19m38.438s
sys     9m34.654s
klee@780432c1aaae0:~$ 
```

