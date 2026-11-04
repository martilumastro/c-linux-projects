#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

static void print_status_fields(const char *path, const char *const prefixes[], size_t n){
    FILE *fp;
    char line[256];
    size_t i;

    fp = fopen(path, "r");
    if(fp == NULL){
        perror(path);
        return;
    }

    while(fgets(line, sizeof(line), fp) != NULL){
        for (i = 0; i < n; i++) {
                if (strncmp(line, prefixes[i], strlen(prefixes[i])) == 0) {
                    printf("%s", line);
                    break;
                }
        }
    }
    fclose(fp);
}

static void print_selinux_label(const char *path){
    FILE *fp;
    char line[256];
    fp = fopen(path, "r");
    if(fp == NULL){
        perror(path);
        return;
    }

    if(fgets(line, sizeof(line), fp) != NULL){
        line[strcspn(line, "\n")] = '\0';
        printf("%s\n",line);
    }
    fclose(fp);
}

int main(void){
const char *const status_prefixes[] = {"NoNewPrivs:", "Seccomp:", "CapEff:", "CapBnd:"};
size_t n = sizeof(status_prefixes)/sizeof(status_prefixes[0]);

print_status_fields("/proc/self/status", status_prefixes, n);
print_selinux_label("/proc/self/attr/current");
return 0;
}