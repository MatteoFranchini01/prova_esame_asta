#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define N_MAX_DEFAULT 20;

int on = 0;
sigset_t sigmask, zeromask;

void game_func (int msgsock, int random_num, int n_max);

void catcher(int signo) {
    printf("Segnale ricevuto\n");
    on = 1;
}

int main(int argc, char* argv[]) {
    int sock, msgsock;
    struct sockaddr_in server, client;
    int lenght;
    char line[256];
    int n_max, random_num, client_num;

    // GESTIONE DEI SEGNALI AFFIDABILI

    struct sigaction sig;
    sig.sa_handler = catcher;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_RESTART;

    sigaction(SIGUSR1, &sig, NULL);
    sigaction(SIGUSR2, &sig, NULL);

    if (argc == 2) {
        n_max = atoi(argv[1]);
    }
    else {
        n_max = N_MAX_DEFAULT;
    }

    random_num = rand() % n_max + 1;
    printf("SERVER: numero random: %d\n", random_num);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("Creation socket\n");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(1520);

    if (bind(sock, (struct sockaddr *)&server, sizeof server) < 0) {
        perror("Binding error\n");
        exit(2);
    }

    lenght = sizeof server;

    if (getsockname(sock, (struct sockaddr *)&server, &lenght) < 0) {
        perror("Getting sock name\n");
        exit(3);
    }

    listen(sock, 2);

    do {
        msgsock = accept(sock, (struct sockaddr *)&client, (int *)&lenght);

        if (msgsock == -1) {
            perror("accept");
            exit(4);
        }
        else {
            if (fork() == 0) {
                printf("PID: %d\n", getpid());
                close(sock);
                game_func(msgsock, random_num, n_max);
                close(msgsock);
                exit(0);
            }
            else {
                close(msgsock);
            }
        }

    } while (1);
}

void game_func (int msgsock, int random_num, int n_max) {
    char line[256];
    int client_num;

    while(1) {
        sprintf(line, "N_MAX: %d\n", n_max);
        write(msgsock, line, strlen(line) + 1);
        sprintf(line, "Inserire il numero vincente\n");
        write(msgsock, line, strlen(line) + 1);
        read(msgsock, line, sizeof(line));
        sscanf(line, "%d", &client_num);
        printf("SERVER: numero scelto: %d\n", client_num);

        if (client_num == random_num) {
            sprintf(line, "HAI VINTO!\n");
            write(msgsock, line, strlen(line) + 1);
            random_num = rand() % n_max + 1;
            game_func(msgsock, random_num, n_max);
        }
        else {
            sprintf(line, "RITENTA!\n");
            write(msgsock, line, strlen(line) + 1);
        }
    }
}