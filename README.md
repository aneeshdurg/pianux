# Pianux

Welcome to pianux - a piano filesystem object for linux! To generate pdfs of all documentation (including this file) run `make docs`.

## Dependencies

+ libfuse (Version 29)
+ libao
+ aneeshdurg/generic-list (provided)
+ aneeshdurg/algebraic-c (provided)

## Piano

Pianux generates audio by launching an instance of the `piano` executable which can be generated with `make piano`. The piano can be launched independantly of pianux for testing or otherwise. The syntax bindings to use it with pianux and directly are the same. To learn the piano syntax see [syntax.md](syntax.md).

## Installation

To install, clone this git repo into your home directory and run `make install`. Read the onscreen instructions that follow.

This will mount the filesystem and define an environment variable `PIANUX_PATH`, a function `piano` that will echo arguments to the pianux filesystem, and a function `piano_kill` to stop all active pianos. To install to a different location, edit the contents of `.pianux_bashrc`.

## Building and running

To build run `make` and then launch the program with `./pianux [mountpt]` for more information about mount points, see fuse's documentation.

To run with logging enabled, you can either set the environment variable `LOGFILE` and have all logs dumped to a file specifed in the variable or use `make rundebug`. e.g:

```bash
LOGFILE=log ./pianux mount
```

will use the file log (or create it if it doesn't exist) to write logging output.

The target `rundebug` in the makefile will create or use a named pipe named `pipe` as the logfile instead and starts a new backgrounded process of `cat` to print out logs as they are generated, thus allowing the log to seem like standard output.

