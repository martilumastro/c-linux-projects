# Landlock ABI Version

A C program for Linux that queries the highest Landlock ABI version supported by the running kernel using the `landlock_create_ruleset` system call.

## Concepts

- Linux kernel security mechanisms
- Landlock and application sandboxing
- Application Binary Interface (ABI)
- System calls using `syscall()`
- `LANDLOCK_CREATE_RULESET_VERSION`
- Error handling with `perror()` and `errno`
- Kernel feature detection and compatibility

## Description

The program queries the running Linux kernel to determine the highest supported Landlock ABI version. It invokes `landlock_create_ruleset()` through `syscall()`, passing `NULL` as the ruleset attributes, `0` as their size, and `LANDLOCK_CREATE_RULESET_VERSION` as the flag.

This operation does not create a ruleset or restrict the process. Instead, it retrieves information about the Landlock functionality available in the current kernel.

The program checks the system call's return value to detect errors and uses `perror()` to display a diagnostic message if the operation fails. Otherwise, it prints the supported ABI version.

Knowing the ABI version allows applications to detect available security features and adapt their sandboxing configuration to the running kernel.

## Compatibility and Error Handling

The availability of Landlock depends on the running kernel, not only on the headers installed on the system.

Possible errors include:

- `ENOSYS`: the system call is not available.
- `EOPNOTSUPP`: Landlock is supported by the kernel but disabled.
- `EINVAL`: an invalid flag or incompatible arguments were supplied.

The program uses the `syscall()` return value to detect failure and `perror()` to report the associated error.

## Compilation

```bash
gcc -Wall -Wextra -Wpedantic landlock_abi.c -o landlock_abi
```

## Execution

```bash
./landlock_abi
```

## References

- [Linux Kernel Documentation — Landlock](https://docs.kernel.org/userspace-api/landlock.html)
- [Linux man-pages — landlock_create_ruleset(2)](https://man7.org/linux/man-pages/man2/landlock_create_ruleset.2.html)