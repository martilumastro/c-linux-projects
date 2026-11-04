#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/landlock.h>

/*
Restituisce la versione ABI di Landlock, oppure un codice negativo
che distingue i diversi motivi di indisponibilità.
   > 0  ABI supportata
   -1   kernel senza Landlock (ENOSYS)
   -2   Landlock disabilitato a runtime (EOPNOTSUPP)
   -3   altro errore (errno stampato)
 */
static long landlock_abi_probe(void)
{
    errno = 0;
    long abi = syscall(SYS_landlock_create_ruleset, NULL, 0,
                       LANDLOCK_CREATE_RULESET_VERSION);
    if (abi >= 0)
        return abi;

    switch (errno) {
    case ENOSYS:
        return -1;
    case EOPNOTSUPP:
        return -2;
    default:
        perror("landlock_create_ruleset");
        return -3;
    }
}

int main(void)
{
    long abi = landlock_abi_probe();

    switch (abi) {
    case -1:
        fprintf(stderr, "Landlock: non supportato dal kernel (ENOSYS)\n");
        return 1;
    case -2:
        fprintf(stderr, "Landlock: disabilitato a runtime (EOPNOTSUPP)\n");
        return 1;
    case -3:
        fprintf(stderr, "Landlock: errore imprevisto\n");
        return 1;
    default:
        printf("landlock_abi=%ld\n", abi);
        return 0;
    }
}