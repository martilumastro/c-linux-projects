#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <linux/landlock.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {
    long abi = syscall(SYS_landlock_create_ruleset, NULL, 0, LANDLOCK_CREATE_RULESET_VERSION);
    if (abi == -1) {
        perror("landlock_create_ruleset");
        return 1;
    }
    printf("Versione ABI Landlock supportata: %ld\n", abi);
    return 0;
}