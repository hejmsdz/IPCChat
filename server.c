//
// Created by rozwad on 11.01.17.
//

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <signal.h>
#include "ipcchat.h"

struct user {
    char username[32];
    int q;
    struct user *next;
};

struct user *connected_users = NULL;
int q;

int open_queue() {
    key_t key;
    int queue;

    key = ftok("server", 127205);
    if (key == -1) {
        perror("Failed to get a queue key");
        exit(-1);
    }

    queue = msgget(key, IPC_CREAT | 0777);
    if (queue == -1) {
        perror("Failed to open a server queue");
        exit(-1);
    }

    printf("Listening on queue %u\n", key);
    return queue;
}

void send_message(struct message msg) {
    size_t msg_size = sizeof(msg) - sizeof(msg.mtype);

    struct user *user;
    for (user = connected_users; user != NULL; user = user->next) {
        if (msgsnd(user->q, &msg, msg_size, 0) == -1) {
            perror("Failed to send a message");
        }
    }
}

void send_server_message(char to_symbol, char *to, char *message) {
    struct message msg;
    msg.mtype = 2;
    strncpy(msg.from, "server", MAX_NAME_LENGTH);
    msg.to_symbol = to_symbol;
    strncpy(msg.to, to, MAX_NAME_LENGTH);
    strncpy(msg.message, message, MAX_MESSAGE_LENGTH);
    send_message(msg);
}

void add_user(char username[], key_t key) {
    int user_q = msgget(key, 0);
    if (user_q == -1) {
        perror("Failed to open a user's queue");
        return;
    }

    struct user *new_user = malloc(sizeof(struct user));
    strncpy(new_user->username, username, MAX_NAME_LENGTH);
    new_user->q = user_q;
    new_user->next = connected_users;
    connected_users = new_user;

    send_server_message('@', username, "Welcome to the server!");
}

void process_command(struct command cmd) {
    printf("{\n  type: %ld\n  data: %s\n  user: %s\n}\n", cmd.mtype, cmd.data, cmd.username);

    char action[32];
    if (cmd.mtype == 2) {
        key_t key;
        sscanf(cmd.data, "%s %u", action, &key);
        add_user(cmd.username, key);
    }
}

void read_commands() {
    struct command cmd;
    size_t size = sizeof(cmd) - sizeof(cmd.mtype);

    while (1) {
        if (msgrcv(q, &cmd, size, 0, 0) == -1) {
            perror("Failed to read a command");
            exit(-1);
        }
        process_command(cmd);
    }
}

void cleanup() {
    msgctl(q, IPC_RMID, NULL);

    struct user *user;
    while (connected_users != NULL) {
        user = connected_users;
        connected_users = user->next;
        free(user);
    }

    printf("Goodbye!\n");
}

void handle(int sig) {
    send_server_message('*', "", "Server received a signal and will terminate.");

    exit(0);
}

int main() {
    atexit(cleanup);
    signal(SIGTERM, handle);

    q = open_queue();
    read_commands();
}