# SUID and No New Privileges

A two-part C project demonstrating Linux SUID behavior and the security implications of the `PR_SET_NO_NEW_PRIVS` process attribute.

The project compares the execution of a SUID-enabled helper program before and after enabling `no_new_privs`, showing how Linux prevents privilege escalation through SUID executables.

## Concepts

- Linux user and group IDs
- Real UID and effective UID
- SUID (Set User ID)
- Privilege escalation
- `fork()` and `execve()`
- `execl()`
- `waitpid()`
- `PR_SET_NO_NEW_PRIVS`
- `prctl()`
- Linux process security
- File permissions and special permission bits

## Project Structure

The project consists of two C source files:

- **`suid_test.c`**: the main program. It executes the helper program twice, first without `no_new_privs` and then after enabling it.
- **`suid_helper.c`**: prints the real UID and effective UID of the process, allowing the SUID behavior to be observed.

## Description

The main program performs two execution tests.

### 1. Execution without No New Privileges

The parent process prints its real and effective UIDs, creates a child process using `fork()`, and executes `suid_helper` using `execl()`.

If the helper executable has the SUID bit enabled and is owned by a different user, its effective UID should become the UID of the executable's owner.

### 2. Execution with No New Privileges

The parent process enables `PR_SET_NO_NEW_PRIVS` using `prctl()`.

It then creates another child process and executes the same helper program.

Because `no_new_privs` is inherited by child processes and preserved across `execve()`, the SUID mechanism cannot grant additional privileges. The helper should therefore retain the caller's effective UID.

## Compilation

Compile both source files:

```bash
gcc -Wall -Wextra -Wpedantic suid_test.c -o suid_test
gcc -Wall -Wextra -Wpedantic suid_helper.c -o suid_helper
```

## SUID Configuration

To demonstrate SUID behavior, the helper executable must be owned by a different user, typically root, and have the SUID permission bit enabled.

On a Linux environment where you have administrative privileges, run:

```bash
sudo chown root:root suid_helper
sudo chmod 4755 suid_helper
```

Verify the permissions:

```bash
ls -l suid_helper
```

The output should contain an `s` in the owner's execute permission field, for example:

```text
-rwsr-xr-x
```

Run the main program as a regular, non-root user.

## Execution

Execute the main program:

```bash
./suid_test
```

Expected behavior:

- During the first execution, the helper should run with the effective UID of its owner, provided SUID is supported and effective.
- During the second execution, SUID privilege elevation should be prevented by `no_new_privs`.
- The helper should print its real and effective UIDs for both tests.

## Environment Considerations

SUID behavior depends on the Linux environment, filesystem, mount options, and execution context.

When using WSL, filesystem location is particularly important:

- Directories under `/mnt/c/` are typically stored on a Windows filesystem mounted through DrvFs.
- Linux permissions and special permission bits depend on the DrvFs configuration and whether Linux metadata is enabled.
- Mount options such as `nosuid` can prevent SUID bits from taking effect.
- For more reliable testing, use the Linux filesystem inside the WSL distribution, for example under `~/security-labs/`.

The `findmnt -T .` command can help inspect the filesystem and its mount options.

## Notes

- `no_new_privs` is irreversible for the current process.
- The attribute is inherited by child processes and preserved across program execution.
- SUID does not necessarily take effect when the filesystem is mounted with `nosuid`, when the process is being traced, or when other execution restrictions apply.
- The experiment should be performed in a controlled Linux environment using a non-root account.
- The first test is meaningful only if the SUID bit is actually effective.