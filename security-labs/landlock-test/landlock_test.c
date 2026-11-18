#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/landlock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static int get_landlock_abi(void){
    int val = syscall(SYS_landlock_create_ruleset, NULL, 0, LANDLOCK_CREATE_RULESET_VERSION);
    if(val < 0){
        fprintf(stderr, "landlock_create_ruleset(): %s\n", strerror(errno));
        return -1;
    }
    return val;
}

static int create_ruleset(void){
    struct landlock_ruleset_attr ruleset_attr = {0};
    ruleset_attr.handled_access_fs = LANDLOCK_ACCESS_FS_WRITE_FILE | LANDLOCK_ACCESS_FS_REMOVE_FILE | LANDLOCK_ACCESS_FS_REMOVE_DIR | LANDLOCK_ACCESS_FS_MAKE_DIR | LANDLOCK_ACCESS_FS_MAKE_REG;

    int fd = syscall(SYS_landlock_create_ruleset, &ruleset_attr, sizeof(ruleset_attr), 0);
    if(fd < 0){
        fprintf(stderr, "landlock_create_ruleset(): %s\n", strerror(errno));
        return -1;
    }
    return fd;
}


static int add_path_rule(int ruleset_fd, const char *path){
    int dir_fd = open(path, O_PATH | O_CLOEXEC);
    if(dir_fd < 0){
        perror("open");
        return -1;
    }
    struct landlock_path_beneath_attr path_attr = {0};
    path_attr.allowed_access = LANDLOCK_ACCESS_FS_WRITE_FILE | LANDLOCK_ACCESS_FS_REMOVE_FILE | LANDLOCK_ACCESS_FS_REMOVE_DIR | LANDLOCK_ACCESS_FS_MAKE_DIR | LANDLOCK_ACCESS_FS_MAKE_REG;
    path_attr.parent_fd = dir_fd;
    int ret = syscall(SYS_landlock_add_rule, ruleset_fd, LANDLOCK_RULE_PATH_BENEATH, &path_attr, 0);
    if(ret < 0){
        fprintf(stderr, "landlock_add_rule(): %s\n", strerror(errno));
        close(dir_fd);
        return -1;
    }
    if(close(dir_fd) < 0){
        perror("close");
        return -1;
    }
    return 0;
}


static int apply_ruleset(int ruleset_fd){
    if(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0){
        fprintf(stderr, "prctl(PR_SET_NO_NEW_PRIVS): %s\n", strerror(errno));
        return -1;
    }
    if(syscall(SYS_landlock_restrict_self, ruleset_fd, 0) < 0){
        fprintf(stderr, "landlock_restrict_self(): %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static int test_access(const char *path, int should_succeed){
    /*
     * Rimuovi un eventuale file residuo da un'esecuzione precedente.
     * Se il file non esiste, unlink fallisce con ENOENT: è un caso
     * normale, non un errore, quindi ignoriamo il valore di ritorno.
     * Questo rende il test idempotente: può essere rieseguito senza
     * dover pulire manualmente /tmp tra un'esecuzione e l'altra.
     */
    unlink(path);

    int fd = open(path, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, S_IRUSR | S_IWUSR);
    if(fd < 0){
        int saved_errno = errno;

        if(!should_succeed && saved_errno == EACCES){
            printf("Access correctly denied: %s\n", path);
            return 0;
        }

        fprintf(stderr, "Unexpected open() failure: %s\n", strerror(saved_errno));
        return -1;
    }

    if(close(fd) <0){
        perror("close");
        return -1;
    }

    if(should_succeed){
        printf("Successfully created file: %s\n", path);
    } else {
        fprintf(stderr, "Unexpectedly created file: %s\n", path);
        if(unlink(path) < 0){
            perror("unlink");
        }
        return -1;
    }
    
    if (unlink(path) < 0) {
        fprintf(stderr, "Warning: failed to remove %s: %s\n", path, strerror(errno));
    }

    return 0;
}

int main(void){

    const char *landlock_allowed = "/tmp/landlock_allowed";
    const char *landlock_restricted = "/tmp/landlock_restricted";

    int abi_version = get_landlock_abi();
    if(abi_version < 0)
        return EXIT_FAILURE;
    printf("Landlock ABI version: %d\n", abi_version);

    int fd = create_ruleset();
    if(fd < 0)
        return EXIT_FAILURE;
    printf("Landlock ruleset created with fd: %d\n", fd);


     if (mkdir(landlock_allowed, S_IRWXU) < 0 && errno != EEXIST) {
        perror("mkdir() failed");
        return EXIT_FAILURE;
    }
    printf("Directory created successfully: %s\n", landlock_allowed);

    if(mkdir(landlock_restricted, S_IRWXU) < 0 && errno != EEXIST){
        perror("mkdir() failed");
        return EXIT_FAILURE;
    }
    printf("Directory created successfully: %s\n", landlock_restricted);

    if(add_path_rule(fd, landlock_allowed) < 0)
        return EXIT_FAILURE;

    int app = apply_ruleset(fd);
    if(app < 0)
        return EXIT_FAILURE;
    printf("Landlock ruleset applied successfully\n");

    if(test_access("/tmp/landlock_allowed/test.txt", 1) < 0)
        return EXIT_FAILURE;
    if(test_access("/tmp/landlock_restricted/test.txt", 0) < 0)
        return EXIT_FAILURE;

    if(close(fd) < 0){
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}