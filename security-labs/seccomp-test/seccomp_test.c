#define _GNU_SOURCE

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <seccomp.h>
#include <sys/prctl.h>

static int test_mkdir(const char *path, int *out_errno)
{
    if (mkdir(path, S_IRWXU) < 0) {
        *out_errno = errno;
        perror("mkdir() failed");
        return -1;
    }
    printf("Directory created successfully: %s\n", path);
    return 0;
}

/*Per questo step:
1. Dichiarare una variabile scmp_filter_ctx chiamata ctx.
2. Inizializzarla con seccomp_init(SCMP_ACT_ALLOW).
3. Controllare che il contesto non sia NULL.
4. Se è NULL, stampare un messaggio di errore e restituire -1.
*/
static int install_seccomp_filter(void){
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if(ctx == NULL){
        fprintf(stderr, "seccomp_init(): %s\n", strerror(errno));
        return -1;
    }
    /*
    1. Chiamare seccomp_rule_add()
    Deve aggiungere una regola con questi parametri:
    Parametro	Valore
    Contesto	ctx
    Azione	SCMP_ACT_ERRNO(EPERM)
    Syscall	SCMP_SYS(mkdir)
    Numero argomenti	0
    */
    int rc = seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(mkdir), 0);
    if(rc < 0){
        fprintf(stderr, "seccomp_rule_add(): %s\n", strerror(-rc));
        seccomp_release(ctx);
        return -1;
    }
    printf("Seccomp rule added successfully\n");

    if(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0){
        fprintf(stderr, "prctl(PR_SET_NO_NEW_PRIVS): %s\n", strerror(errno));
        seccomp_release(ctx);
        return -1;
    }
    printf("PR_SET_NO_NEW_PRIVS set successfully\n");

    /*. 
    seccomp_load(ctx): applica il filtro al processo.
    seccomp_release(ctx) --> Dopo un caricamento riuscito, libera il contesto userspace.
    Attenzione: non rimuove il filtro dal kernel. Il filtro rimane attivo.
    */
    int val = seccomp_load(ctx);
    if(val < 0){
        fprintf(stderr, "seccomp_load(): %s\n", strerror(-val));
        seccomp_release(ctx);
        return -1;
    }
    printf("Seccomp filter loaded successfully\n");
    seccomp_release(ctx);
    return 0;
}

/*Ti propongo due percorsi di test:
"/tmp/seccomp_before"
"/tmp/seccomp_after"


In questo modo possiamo distinguere chiaramente i due tentativi.

Prima consegna: implementa main() con:
- una chiamata a test_mkdir() sul primo percorso;
- una chiamata a install_seccomp_filter();
- una chiamata a test_mkdir() sul secondo percorso.
*/

int main(void){
    
    const char *path_before = "/tmp/seccomp_before";
    const char *path_after = "/tmp/seccomp_after";
    int err = 0;
    printf("Testing mkdir() before installing seccomp filter...\n");
    if(test_mkdir(path_before, &err) < 0){
        fprintf(stderr, "Failed to create directory before seccomp filter: %s\n",
            strerror(err));
        return EXIT_FAILURE;
    }

    printf("Installing seccomp filter...\n");
    if(install_seccomp_filter() < 0){
        fprintf(stderr, "Failed to install seccomp filter\n");
        return EXIT_FAILURE;
    }

    printf("Testing mkdir() after installing seccomp filter...\n");
    if(test_mkdir(path_after, &err) < 0){
        if(errno == EPERM){
            printf("Seccomp filter is working correctly: mkdir() was blocked as expected.\n");
        } else {
            fprintf(stderr, "Unexpected result: mkdir() failed with error: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "mkdir() succeeded after seccomp filter was installed, which is unexpected.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
