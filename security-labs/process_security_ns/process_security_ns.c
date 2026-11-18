/*
probe in C che confronta i namespace del processo corrente 
con quelli di un ambiente di riferimento, 
identificando quali sono condivisi e quali isolati.

La probe esamina otto categorie.
Mount — mnt
PID — pid
Network — net
IPC — ipc
UTS — uts
User — user
Cgroup — cgroup
Time — time

Linux espone i namespace attraverso il filesystem virtuale /proc.
Per il processo corrente puoi trovare: /proc/self/ns/

La funzione stat() permette di ottenere informazioni su un percorso, tra cui:
- st_dev: identificativo del dispositivo;
- st_ino: numero di inode;
- altri metadati.

La funzione principale sarà:
stat(path, &st);
*/


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

typedef enum {
    UNKNOWN = 0,
    SHARED,
    ISOLATED,
    UNAVAILABLE,
    ERROR
} NamespaceState;

typedef struct {
    char name[7];
    dev_t st_dev;
    ino_t st_ino;
    NamespaceState state;
    dev_t baseline_dev;
    ino_t baseline_ino;
} NamespaceInfo;

NamespaceInfo namespaces[] = {
    {"mnt", 0, 0, UNAVAILABLE, 0, 0},
    {"pid", 0, 0, UNAVAILABLE, 0, 0},
    {"net", 0, 0, UNAVAILABLE, 0, 0},
    {"ipc", 0, 0, UNAVAILABLE, 0, 0},
    {"uts", 0, 0, UNAVAILABLE, 0, 0},
    {"user", 0, 0, UNAVAILABLE, 0, 0},
    {"cgroup", 0, 0, UNAVAILABLE, 0, 0},
    {"time", 0, 0, UNAVAILABLE, 0, 0}
};

static void collect_namespaces(NamespaceInfo *ns_info, size_t count) {
    char path[256];
    struct stat st;

    for (size_t i = 0; i < count; i++) {

        snprintf(path, sizeof(path),
                 "/proc/self/ns/%s", ns_info[i].name);

        if (stat(path, &st) == 0) {

            ns_info[i].st_dev = st.st_dev;
            ns_info[i].st_ino = st.st_ino;
            ns_info[i].state = UNKNOWN;

        } else {

            int err = errno;

            if (err == ENOENT || err == EACCES || err == ENOTDIR) {
                ns_info[i].state = UNAVAILABLE;
            } else {
                ns_info[i].state = ERROR;
            }

            fprintf(stderr, "Namespace %s: %s\n", ns_info[i].name, strerror(err));
        }
    }
}

static int save_baseline(const char *filename, NamespaceInfo *ns_info, size_t count)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        if (fprintf(fp, "%s %ju %ju\n",
                    ns_info[i].name,
                    (uintmax_t)ns_info[i].st_dev,
                    (uintmax_t)ns_info[i].st_ino) < 0) {
            perror("fprintf");
            fclose(fp);
            return -1;
        }
    }

    if (fclose(fp) == EOF) {
        perror("fclose");
        return -1;
    }
    return 0;
}


static int load_baseline(const char *filename, NamespaceInfo *ns_info, size_t count)
{
    char line[128];
    size_t i = 0;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    while (i < count) {
        char name[7];
        uintmax_t dev, ino;
        char extra;

        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "Error reading baseline: missing line %zu\n", i + 1);
            fclose(fp);
            return -1;
        }

        /*
        %6s limita il nome a 6 caratteri (+ terminatore), coerente
        con la dimensione di name[7] e con i nomi dei namespace (mnt, pid, net, ipc, uts, user, cgroup, time).
         */
        int parsed = sscanf(line, "%6s %ju %ju %c", name, &dev, &ino, &extra);
        if (parsed != 3) {
            fprintf(stderr, "Invalid baseline format at line %zu\n", i + 1);
            fclose(fp);
            return -1;
        }

        if (strcmp(name, ns_info[i].name) != 0) {
            fprintf(stderr, "Namespace name mismatch: expected %s, got %s\n",
                    ns_info[i].name, name);
            fclose(fp);
            return -1;
        }

        ns_info[i].baseline_dev = (dev_t)dev;
        ns_info[i].baseline_ino = (ino_t)ino;
        i++;
    }

    /* Verifica che non ci siano altre righe con contenuto. */
    while (fgets(line, sizeof(line), fp) != NULL) {
        int only_whitespace = 1;
        for (size_t j = 0; line[j] != '\0'; j++) {
            if (!isspace((unsigned char)line[j])) {
                only_whitespace = 0;
                break;
            }
        }
        if (!only_whitespace) {
            fprintf(stderr, "Unexpected extra data in baseline file\n");
            fclose(fp);
            return -1;
        }
    }

    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return -1;
    }

    if (fclose(fp) == EOF) {
        perror("fclose");
        return -1;
    }
    return 0;
}

static void compare_namespaces(NamespaceInfo *ns_info, size_t count){
    /* Per ogni elemento dell'array:
    Se state è UNAVAILABLE oppure ERROR, lascia invariato lo stato e passa al successivo.
    Confronta st_dev con baseline_dev e st_ino con baseline_ino.
    Se entrambi coincidono, assegna SHARED.
    Se almeno uno è diverso, assegna ISOLATED.
 */
    for(size_t i = 0; i < count; i++){
        if(ns_info[i].state == UNAVAILABLE || ns_info[i].state == ERROR){
            continue;
        }
        if(ns_info[i].st_dev == ns_info[i].baseline_dev && ns_info[i].st_ino == ns_info[i].baseline_ino)
            ns_info[i].state = SHARED;
        else
            ns_info[i].state = ISOLATED;
    }
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s --capture <filename>\n"
                "       %s --compare <filename>\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    size_t count = sizeof(namespaces) / sizeof(namespaces[0]);

    if (strcmp(argv[1], "--capture") == 0) {

        collect_namespaces(namespaces, count);

        printf("Namespace Information:\n");

        for (size_t i = 0; i < count; i++) {
            printf("%s: ", namespaces[i].name);

            switch (namespaces[i].state) {
                case UNKNOWN:
                    printf("Unknown (dev: %lu, ino: %lu)\n",
                           (unsigned long)namespaces[i].st_dev,
                           (unsigned long)namespaces[i].st_ino);
                    break;
                case SHARED:
                    printf("Shared\n");
                    break;
                case ISOLATED:
                    printf("Isolated\n");
                    break;
                case UNAVAILABLE:
                    printf("Unavailable\n");
                    break;
                case ERROR:
                    printf("Error retrieving information\n");
                    break;
            }
        }

        if (save_baseline(argv[2], namespaces, count) != 0) {
            fprintf(stderr, "Failed to save baseline to %s\n", argv[2]);
            return EXIT_FAILURE;
        }

        printf("Baseline saved to %s\n", argv[2]);

    } else if (strcmp(argv[1], "--compare") == 0) {

        collect_namespaces(namespaces, count);

        if (load_baseline(argv[2], namespaces, count) != 0) {
            fprintf(stderr, "Failed to load baseline from %s\n", argv[2]);
            return EXIT_FAILURE;
        }

        compare_namespaces(namespaces, count);

        printf("Namespace comparison:\n");

        for (size_t i = 0; i < count; i++) {
            printf("%s: ", namespaces[i].name);

            switch (namespaces[i].state) {
                case SHARED:
                    printf("Shared\n");
                    break;
                case ISOLATED:
                    printf("Isolated\n");
                    break;
                case UNAVAILABLE:
                    printf("Unavailable\n");
                    break;
                case ERROR:
                    printf("Error retrieving information\n");
                    break;
                case UNKNOWN:
                    printf("Unknown\n");
                    break;
            }
        }

    } else {
        fprintf(stderr, "Unknown option: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

