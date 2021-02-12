# Crude demo of formal verification

Here is a crude demo of formal verification of tiny-regex. This is a hefty plagiat of @DavidKorczynski - see https://www.youtube.com/watch?v=z6bsk-lsk1Q or https://github.com/kokke/tiny-regex-c/issues/44 for more context.

I am using the KLEE symbolic execution engine and Docker here on a Debian-based platform.

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
- Compile and emit LLVM bitcode: ` klee@cc0c26c5b84c:~$ clang -c -g -O0 -emit-llvm re.c `
- Run KLEE and wait for 5-10 minutes: ` klee@cc0c26c5b84c:~$ klee --libc=uclibc re.bc `
- A positive result looks like this:
```
klee@cc0c26c5b84c:~$ klee --libc=uclibc re.bc
KLEE: NOTE: Using klee-uclibc : /tmp/klee_build90stp_z3/runtime/lib/klee-uclibc.bca
KLEE: output directory is "/home/kle# Crude demo of formal verification

Here is a crude demo of formal verification of tiny-regex. This is a hefty plagiat of @DavidKorczynski - see https://github.com/kokke/tiny-regex-c/issues/44 for more context.

I am using the KLEE symbolic execution engine and Docker here on a Debian-based platform.

Steps:

- Get the KLEE Docker image: ` $ sudo docker pull klee/klee `
- Run the KLEE Docker image: ` $ sudo $ sudo docker run --rm -ti --ulimit='stack=-1:-1' klee/klee `
- NOTE: You should see a command prompt like this: ` klee@cc0c26c5b84c:~$ `
- Fetch `re.h`: ` klee@cc0c26c5b84c:~$ wget https://raw.githubusercontents.com/kokke/tiny-regex-c/master/re.h . `
- Fetch `re.c`: ` klee@cc0c26c5b84c:~$ wget https://raw.githubusercontents.com/kokke/tiny-regex-c/master/re.h . `
- Run your favorite editor, and insert the code below in the bottom of `re.c`
```C
/*

tiny-regex KLEE test driver
---------------------------
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
- Alternatively, run this command: ` klee@cc0c26c5b84c:~$ echo "int main(int argc,char* argv[]){ char arr[10]; klee_make_symbolic(arr, sizeof(arr), \"arr\"); klee_assume(arr[sizeof(arr)-1] == 0); re_compile(arr); return 0; }" >> re.c `
- Compile and emit LLVM bitcode: ` klee@cc0c26c5b84c:~$ clang -c -g -O0 -emit-llvm re.c `
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


e/klee-out-3"
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


