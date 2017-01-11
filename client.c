//
// Created by rozwad on 11.01.17.
//

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "ipcchat.h"

#define COLOR_BOLD    "\x1b[1m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

char username[MAX_NAME_LENGTH];
int in_q, out_q;

key_t create_ftok() {
    key_t key;

    if (access(username, 0) != 0) {
        creat(username, 0600);
    }

    key = ftok(username, 127205);
    if (key == -1) {
        perror("Failed to get input queue key");
        exit(-1);
    }

    return key;
}

int open_input_queue(key_t key) {
    int queue;

    key = create_ftok();

    queue = msgget(key, IPC_CREAT | 0622);
    if (queue == -1) {
        perror("Failed to access input queue");
        exit(-1);
    }

    return queue;
}

int open_output_queue(key_t key) {
    int queue;

    queue = msgget(key, 0);
    if (queue == -1) {
        perror("Failed to access output queue");
        exit(-1);
    }
    return queue;
}

void send_command(struct command cmd) {
    size_t cmd_size = sizeof(cmd) - sizeof(cmd.mtype);
    strcpy(cmd.username, username);
    if (msgsnd(out_q, &cmd, cmd_size, 0) == -1) {
        perror("Failed to send a command");
        exit(-1);
    }
}

void login(key_t in_key) {
    struct command cmd;
    cmd.mtype = 2;
    sprintf(cmd.data, "login %u", in_key);
    send_command(cmd);
}

void read_messages() {
    struct message msg;
    size_t msg_size = sizeof(msg) - sizeof(msg.mtype);

    while (1) {
        if (msgrcv(in_q, &msg, msg_size, 0, 0) == -1) {
            perror("Failed to read a message");
            break;
        }
        printf("\n" COLOR_GREEN "%s" COLOR_RESET, msg.from);
        printf(" to " COLOR_GREEN "%c%s" COLOR_RESET "\n", msg.to_symbol, msg.to);
        printf(COLOR_BOLD "%s" COLOR_RESET "\n\n", msg.message);
    }
}

void cleanup() {
    msgctl(in_q, IPC_RMID, NULL);
}

void handle(int sig) {
    exit(0);
}

int main() {
    atexit(cleanup);
    signal(SIGTERM, handle);

    key_t in_key, out_key;

    printf("Server queue key: ");
    scanf("%u", &out_key);
    printf("Username: ");
    scanf("%s", username);

    in_key = create_ftok();
    in_q = open_input_queue(in_key);
    out_q = open_output_queue(out_key);
    login(in_key);

    read_messages();
}