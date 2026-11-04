#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Utility: lettura di una riga da un file di /proc o /sys */

/*
Cerca in `path` una riga che inizia con `prefix` e ne restituisce
il valore (trim del newline) in `out`. Ritorna 0 se trovata, -1 se
il file non esiste o il prefisso non c'è.
 */
static int read_kv(const char *path, const char *prefix,
                   char *out, size_t outlen)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        return -1;

    char line[512];
    size_t plen = strlen(prefix);
    int found = -1;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, prefix, plen) != 0)
            continue;
        const char *val = line + plen;
        while (*val == ' ' || *val == '\t')
            val++;
        size_t vlen = strcspn(val, "\n");
        if (vlen >= outlen)
            vlen = outlen - 1;
        memcpy(out, val, vlen);
        out[vlen] = '\0';
        found = 0;
        break;
    }
    fclose(fp);
    return found;
}

/* Capabilities: bitmask -> nomi */
/* Tabella dei nomi delle capability (Linux 5.x/6.x). Indice = bit. */
static const char *const cap_names[] = {
    "CAP_CHOWN", "CAP_DAC_OVERRIDE", "CAP_DAC_READ_SEARCH",
    "CAP_FOWNER", "CAP_FSETID", "CAP_KILL", "CAP_SETGID", "CAP_SETUID",
    "CAP_SETPCAP", "CAP_LINUX_IMMUTABLE", "CAP_NET_BIND_SERVICE",
    "CAP_NET_BROADCAST", "CAP_NET_ADMIN", "CAP_NET_RAW",
    "CAP_IPC_LOCK", "CAP_IPC_OWNER", "CAP_SYS_MODULE", "CAP_SYS_RAWIO",
    "CAP_SYS_CHROOT", "CAP_SYS_PTRACE", "CAP_SYS_PACCT",
    "CAP_SYS_ADMIN", "CAP_SYS_BOOT", "CAP_SYS_NICE", "CAP_SYS_RESOURCE",
    "CAP_SYS_TIME", "CAP_SYS_TTY_CONFIG", "CAP_MKNOD", "CAP_LEASE",
    "CAP_AUDIT_WRITE", "CAP_AUDIT_CONTROL", "CAP_SETFCAP",
    "CAP_MAC_OVERRIDE", "CAP_MAC_ADMIN", "CAP_SYSLOG",
    "CAP_WAKE_ALARM", "CAP_BLOCK_SUSPEND", "CAP_AUDIT_READ",
    "CAP_PERFMON", "CAP_BPF", "CAP_CHECKPOINT_RESTORE",
};

#define CAP_NAMES_LEN (sizeof(cap_names) / sizeof(cap_names[0]))

/*
Stampa i nomi delle capability presenti nella maschera hex `hexmask`.
Esempio: "0000000000000000" -> "none"
         "00000000a80425fb" -> "CAP_CHOWN,CAP_DAC_OVERRIDE,..."
*/
static void print_caps(const char *label, const char *hexmask)
{
    errno = 0;
    unsigned long long mask = strtoull(hexmask, NULL, 16);
    if (errno != 0) {
        printf("%s=parse_error\n", label);
        return;
    }

    printf("%s=0x%llx", label, mask);
    if (mask == 0) {
        printf(" (none)\n");
        return;
    }
    printf(" (");

    int first = 1;
    for (size_t i = 0; i < CAP_NAMES_LEN; i++) {
        if (mask & (1ULL << i)) {
            if (!first)
                printf(",");
            printf("%s", cap_names[i]);
            first = 0;
        }
    }
    printf(")\n");
}

/* SELinux: etichetta + modalità */
static void print_selinux(void)
{
    char label[256];
    if (read_kv("/proc/self/attr/current", "", label, sizeof(label)) == 0) {
        printf("selinux_label=%s\n", label);
    } else {
        printf("selinux_label=unavailable\n");
    }

    char enforce[8];
    if (read_kv("/sys/fs/selinux/enforce", "", enforce, sizeof(enforce)) == 0) {
        printf("selinux_mode=%s\n",
               enforce[0] == '1' ? "enforcing" : "permissive");
    } else {
        printf("selinux_mode=disabled\n");
    }
}

int main(void)
{
    char buf[256];

    /* /proc/self/status: i campi che ci interessano */
    if (read_kv("/proc/self/status", "NoNewPrivs:", buf, sizeof(buf)) == 0)
        printf("no_new_privs=%s\n", buf);
    else
        printf("no_new_privs=unknown\n");

    if (read_kv("/proc/self/status", "Seccomp:", buf, sizeof(buf)) == 0) {
        /* 0=disabled, 1=strict, 2=filter */
        printf("seccomp_mode=%s\n", buf);
    } else {
        printf("seccomp_mode=unknown\n");
    }

    /* CapEff: capabilities effettive. CapBnd: bounding set. */
    if (read_kv("/proc/self/status", "CapEff:", buf, sizeof(buf)) == 0)
        print_caps("capeff", buf);
    else
        printf("capeff=unknown\n");

    if (read_kv("/proc/self/status", "CapBnd:", buf, sizeof(buf)) == 0)
        print_caps("capbnd", buf);
    else
        printf("capbnd=unknown\n");

    /* SELinux */
    print_selinux();

    return 0;
}