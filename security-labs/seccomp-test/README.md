# Seccomp Syscall Filtering

A C program that demonstrates Linux Seccomp filtering by selectively blocking the `mkdir()` system call.

The program first creates a directory without restrictions, installs a Seccomp filter that denies subsequent `mkdir()` calls, and then attempts to create another directory to verify that the restriction is enforced.

## Concepts

- Linux Seccomp
- System call filtering
- libseccomp
- Seccomp-BPF
- `seccomp_init()`
- `seccomp_rule_add()`
- `seccomp_load()`
- `seccomp_release()`
- `PR_SET_NO_NEW_PRIVS`
- `SCMP_ACT_ALLOW`
- `SCMP_ACT_ERRNO`
- `EPERM`
- Linux process security

## Description

The program performs three main steps:

1. **Test before filtering:** attempts to create `/tmp/seccomp_before` using `mkdir()`. The operation should succeed.
2. **Install the Seccomp filter:** initializes a filter with `SCMP_ACT_ALLOW`, allowing system calls by default, and adds a rule that returns `EPERM` whenever the `mkdir()` system call is invoked.
3. **Test after filtering:** attempts to create `/tmp/seccomp_after`. The operation should fail because the Seccomp filter blocks `mkdir()`.

Before loading the filter, the program enables `PR_SET_NO_NEW_PRIVS` using `prctl()`. This prevents the process from gaining additional privileges through `execve()` and satisfies the unprivileged Seccomp filter installation requirement.

After successfully loading the filter, `seccomp_release()` frees the userspace filter context. The filter itself remains active in the kernel.

## Compilation

The program requires the libseccomp development library.

On Debian/Ubuntu-based systems, install it with:

```bash
sudo apt update
sudo apt install libseccomp-dev
```

Compile the program using:

```bash
gcc -Wall -Wextra -Wpedantic seccomp_test.c -o seccomp_test -lseccomp
```

## Cleanup

Before running the program, remove directories that may have been created during previous executions:

```bash
rmdir /tmp/seccomp_before /tmp/seccomp_after 2>/dev/null
```

This ensures that the test starts with a clean environment. The command suppresses error messages if the directories do not exist.

## Execution

Run the program with:

```bash
./seccomp_test
```

Expected behavior:

- The first directory creation succeeds.
- The Seccomp filter is successfully installed.
- The second directory creation fails with `EPERM` (`Operation not permitted`).

This demonstrates how Seccomp can restrict a process's system call interface at runtime.

## Notes

- The filter uses a default-allow policy: system calls are allowed unless explicitly restricted by a rule.
- The `mkdir()` rule returns `EPERM` instead of terminating the process.
- Seccomp filters are irreversible for the process: `seccomp_release()` does not remove an already loaded filter.
- The filter is inherited by child processes created after installation.
- The program requires a Linux environment with Seccomp support and libseccomp installed.