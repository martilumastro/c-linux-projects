# Landlock Filesystem Restrictions

A C program that demonstrates Linux Landlock by creating and applying a filesystem access-control ruleset to restrict the current process.

The program creates two directories, grants filesystem write-related permissions to one of them, and then verifies that file creation is allowed in the permitted directory but denied in the restricted directory.

## Concepts

- Linux Landlock
- Linux security modules (LSM)
- Filesystem access control
- Landlock ABI version
- Landlock rulesets
- `landlock_create_ruleset()`
- `landlock_add_rule()`
- `landlock_restrict_self()`
- `LANDLOCK_ACCESS_FS_*`
- `LANDLOCK_RULE_PATH_BENEATH`
- `PR_SET_NO_NEW_PRIVS`
- File descriptors
- `open()`, `close()`, `unlink()`, and `mkdir()`
- Linux system calls

## Description

The program demonstrates how Landlock can restrict filesystem operations for a process.

It performs the following steps:

1. **Check Landlock ABI:** queries the highest Landlock ABI version supported by the running kernel.
2. **Create a ruleset:** defines the filesystem access rights that the ruleset will handle, including file writing, file removal, directory removal, directory creation, and regular file creation.
3. **Create test directories:** creates `/tmp/landlock_allowed` and `/tmp/landlock_restricted`.
4. **Add a path rule:** grants the specified filesystem permissions to `/tmp/landlock_allowed` using `LANDLOCK_RULE_PATH_BENEATH`.
5. **Apply the ruleset:** enables `PR_SET_NO_NEW_PRIVS` and applies the restrictions to the current process using `landlock_restrict_self()`.
6. **Test filesystem access:** attempts to create a file in both directories.

Expected results:

- Creating `/tmp/landlock_allowed/test.txt` succeeds.
- Creating `/tmp/landlock_restricted/test.txt` is denied.

The ruleset follows a default-deny approach for the filesystem access rights it handles: access is granted only where an appropriate rule permits it.

## Compilation

Compile the program using:

```bash
gcc -Wall -Wextra -Wpedantic -std=c11 landlock_test.c -o landlock_test
```

No additional userspace library is required because the program invokes Landlock system calls directly using `syscall()`.

## Cleanup

Before running the program, remove directories left by previous executions:

```bash
rm -rf /tmp/landlock_allowed /tmp/landlock_restricted
```

This ensures that the test directories are recreated from a clean state.

## Execution

Run the program using:

```bash
./landlock_test
```

Expected behavior:

- The Landlock ABI version is displayed.
- A ruleset is created successfully.
- Both test directories are created.
- The ruleset is applied successfully.
- File creation in the allowed directory succeeds.
- File creation in the restricted directory is denied.

## Notes

- Landlock support depends on the running Linux kernel and its configuration.
- The program requires a kernel that supports the Landlock system calls and the access rights used by the ruleset.
- `PR_SET_NO_NEW_PRIVS` is enabled before applying the ruleset.
- Landlock restrictions apply to the current process and are inherited by subsequently created child processes.
- The ruleset is irreversible for the current process: closing the ruleset file descriptor does not remove the restrictions.
- The program tests file creation, not every filesystem operation covered by the ruleset.
- The test directories are created before the restrictions are applied.