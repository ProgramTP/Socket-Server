#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>


void handler(int code) {
    return;
}

int main(int argc, char *argv[]) {


    if (argc < 2 || atoi(argv[1]) > 65535 || atoi(argv[1]) < 1024) {
        exit(1);
    }
    //Variables
    int socketfd, bindfd, listenfd, max, portNumber;
    int client_sockets[40];
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    //Socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) exit(1);
    signal(SIGPIPE, SIG_IGN);
    portNumber = atoi(argv[1]);
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_family = AF_INET;

    //Binding
    bindfd = bind(socketfd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in));
    if (bindfd == -1) exit(1);

    //Listening
    listenfd = listen(socketfd, 20);
    if (listenfd == -1) exit(1);

    //Variables for loop
    int acceptNo, readVal, writeVal, curr_client, new_sock, currfd;
    int clientLen = sizeof(client_address);
    fd_set client_set, master_set; 
    char post[76];
    memset(&post, 0, 75);
    strcpy(post, "\n");
    post[75] = '\0';
    char input[76];
    char get[] = "?\n";
    char newline[] = "\n";
    FD_ZERO(&client_set);
    FD_SET(socketfd, &client_set);  
    max = socketfd; 

    struct sigaction action;
    action.sa_handler = handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    while(1) {
        master_set = client_set;
        memset(&input, 0, 77);
        input[76] = '\0';
        curr_client = select(max+1, &master_set, NULL, NULL, NULL); 
        if (curr_client < 0) exit(1); 

        for (int i = 0; i <= max; i++) {   
            if (FD_ISSET(i, &master_set)) {
                if (i == socketfd) {
                    acceptNo = accept(socketfd, (struct sockaddr*)&client_address, &clientLen);
                    if (acceptNo == -1) exit(1);
                    FD_SET(acceptNo, &client_set);
                    if (acceptNo > max) max = acceptNo; 
                }
                else {
                    readVal = read(i, input, 76);
                    if (readVal == -1) exit(1);
                    if (readVal == 0) { 
                        FD_CLR (i, &client_set);
                        continue;
                    }
                    if (strlen(input) < 77) {
                        if (strlen(input) == 76) {
                            char *last = &input[75];
                            if (strcmp(last, newline) != 0) {
                                continue;
                            }
                        }
                        if (input[0] == '!') {
                            memmove(input, input+1, strlen(input));
                            strcpy(post, input);
                        }
                        if (strcmp(input, get) == 0) {
                            sigaction(SIGPIPE, &action, NULL);
                            writeVal = write(i, post, strlen(post));
                            if (writeVal == -1) exit(1);
                        }
                    }
                }
            }   
        } 
    }

    return 0;



}