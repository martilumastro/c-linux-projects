# Linux Namespace Analyzer

A C implementation of a Linux namespace analyzer that identifies shared and isolated namespaces by comparing the current process namespace identifiers against a previously captured reference environment.

## Concepts

- Linux namespaces
- Process isolation and containerization
- `/proc` virtual filesystem
- Namespace identification through `stat()`
- Device IDs (`st_dev`) and inode numbers (`st_ino`)
- Baseline capture and comparison
- File handling in C
- Error handling with `errno` and `strerror()`
- Command-line argument parsing
- Data validation and parsing with `sscanf()`

## Description

The program analyzes eight Linux namespace categories:

- Mount (`mnt`)
- Process ID (`pid`)
- Network (`net`)
- Interprocess Communication (`ipc`)
- UTS (`uts`)
- User (`user`)
- Cgroup (`cgroup`)
- Time (`time`)

It uses the `/proc/self/ns/` directory to retrieve namespace references for the current process. The `stat()` system call obtains the device ID and inode number associated with each namespace.

The application supports two operating modes:

**Capture Mode (`--capture`)**

Collects the namespace identifiers of the current process and saves them to a baseline file. This file represents the reference environment used for subsequent comparisons.

**Compare Mode (`--compare`)**

Collects the current process namespace identifiers, loads a previously saved baseline, and compares the device and inode numbers to determine which namespaces are shared and which are isolated relative to the reference.

The program also handles unavailable namespaces, filesystem access errors, invalid baseline files, and unexpected command-line arguments.

## Compilation

```bash
gcc -Wall -Wextra -Wpedantic namespace_analyzer.c -o namespace_analyzer
```

## Execution

**Capture a baseline:**

```bash
./namespace_analyzer --capture baseline.txt
```

**Compare against the captured baseline:**

```bash
./namespace_analyzer --compare baseline.txt
```