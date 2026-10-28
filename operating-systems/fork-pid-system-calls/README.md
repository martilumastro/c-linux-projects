# PID and System Calls

A C program for Linux that retrieves and compares process IDs using `getpid()` and `syscall(SYS_getpid)`, with process creation through `fork()`.

## Concepts

- Process IDs (PID)
- `getpid()` and `syscall(SYS_getpid)`
- Process creation with `fork()`
- Parent and child processes
- Process synchronization with `wait()`
- Error handling with `perror()`

## Description

The program retrieves and compares the PID of the parent process using both `getpid()` and `syscall(SYS_getpid)`. It then creates a child process with `fork()`, retrieves the child's PID using both methods, and compares the results. The parent waits for the child to terminate and checks its exit status.

## Compilation

```bash
gcc -Wall -Wextra -Wpedantic pid.c -o pid
```

## Execution

```bash
./pid
```