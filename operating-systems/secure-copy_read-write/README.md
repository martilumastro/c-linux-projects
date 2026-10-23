# File Copy

A C program that copies the contents of a source file to a destination file using POSIX system calls.

## Description

The program reads the source and destination file paths from the user, then copies the file contents using `open()`, `read()`, and `write()`. It creates the destination file if it does not exist or overwrites it if it already exists.

## Compilation

```bash
gcc -Wall -Wextra -Wpedantic main.c -o file-copy
```

## Execution

```bash
./file-copy
```