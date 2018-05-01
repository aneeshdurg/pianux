# Pianux

Welcome to pianux - a piano filesystem object for linux! To generate pdfs of all documentation (including this file) run `make docs`.

## Dependencies

+ libfuse (Version 29)
+ libao
+ aneeshdurg/generic-list (provided)
+ aneeshdurg/algebraic-c (provided)

## Piano

Pianux generates audio by launching an instance of the `piano` executable which can be generated with `make piano`. The piano can be launched independantly of pianux for testing or otherwise. The syntax bindings to use it with pianux and directly are the same. To learn the piano syntax see [syntax.pdf](syntax.pdf).

## Installation

To install, clone this git repo into your home directory and run `make install`. Read the onscreen instructions that follow.

This will mount the filesystem and define an environment variable `PIANUX_PATH`, a function `piano` that will echo arguments to the pianux filesystem, and a function `piano_kill` to stop all active pianos. To install to a different location, edit the contents of `.pianux_bashrc`.

## Using in an external program

Using pianux is easy! Just open a file descriptor pointing to `$PIANUX_PATH` and you can start writing commands to the audio interface. Since all piano input is given using the piano syntax, this interface is language agnostic! For example in `python`:

```python
import os
piano = os.environ['PIANUX_PATH']
f = open(piano, 'w')
f.write('abcdefg')
f.flush()
f.close()
```

would be equivalent to the following `C`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
int main(){
  int fd = open(getenv("PIANUX_PATH"), O_WRONLY);
  write(fd, "abcdefg", 7); 
  close(fd);
  return 0;
}
```

or even the following `bash`:

```bash
echo abcdefg > $PIANUX_PATH
```

Pianux can be easily incorporated into any language that supports file IO!

## Building and running

To build run `make` and then launch the program with `./pianux [mountpt]` for more information about mount points, see fuse's documentation.

To run with logging enabled, you can either set the environment variable `LOGFILE` and have all logs dumped to a file specifed in the variable or use `make rundebug`. e.g:

```bash
LOGFILE=log ./pianux mount
```

will use the file log (or create it if it doesn't exist) to write logging output.

The target `rundebug` in the makefile will create or use a named pipe named `pipe` as the logfile instead and starts a new backgrounded process of `cat` to print out logs as they are generated, thus allowing the log to seem like standard output.

