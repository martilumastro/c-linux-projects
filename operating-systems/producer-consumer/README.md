# Producer-Consumer Problem

A C implementation of the producer-consumer problem using POSIX threads and synchronization primitives.

## Concepts

- POSIX threads (`pthreads`)
- Mutexes
- Condition variables
- Circular buffers
- Thread synchronization and shared resources

## Description

The program simulates customers waiting in a queue and a shopkeeper serving them. A bounded circular buffer represents the waiting queue, while mutexes and condition variables coordinate access to the shared resource and prevent the queue from exceeding its capacity or becoming empty when a customer is served.

## Compilation

```bash
gcc -Wall -Wextra -Wpedantic -pthread main.c -o producer-consumer
```

## Execution

```bash
./producer-consumer
```