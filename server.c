//
// Created by rozwad on 11.01.17.
//

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
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

bool user_match(struct user *user, char to_symbol, char *to) {
    switch (to_symbol) {
        case '*':
            return true;
        case '@':
            return strcmp(user->username, to) == 0;
        default:
            return false;
    }
}

void send_message(struct message msg) {
    size_t msg_size = sizeof(msg) - sizeof(msg.mtype);

    struct user *user;
    for (user = connected_users; user != NULL; user = user->next) {
        if (user_match(user, msg.to_symbol, msg.to)) {
            if (msgsnd(user->q, &msg, msg_size, 0) == -1) {
                perror("Failed to send a message");
            }
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

void send_user_list(char username[]) {
    char users_list[MAX_MESSAGE_LENGTH] = "Connected users: ";

    struct user *user;
    for (user = connected_users; user != NULL; user = user->next) {
        strcat(users_list, user->username);
        if (user->next != NULL) {
            strcat(users_list, ", ");
        }
    }

    send_server_message('@', username, users_list);
}

void add_user(char username[], key_t key) {
    struct user *user, *prev = NULL;
    for (user = connected_users; user != NULL; user = user->next) {
        if (strcmp(user->username, username) == 0) {
            printf("This username is taken!\n");
            return;
        }
        prev = user;
    }

    int user_q = msgget(key, 0);
    if (user_q == -1) {
        perror("Failed to open a user's queue");
        return;
    }

    struct user *new_user = malloc(sizeof(struct user));
    strncpy(new_user->username, username, MAX_NAME_LENGTH);
    new_user->q = user_q;
    new_user->next = NULL;
    *(prev == NULL ? &connected_users : &prev->next) = new_user;

    send_server_message('@', username, "Welcome to the server!");
}

void remove_user(char username[]) {
    struct user *user, *prev = NULL;
    for (user = connected_users; user != NULL; user = user->next) {
        if (strcmp(user->username, username) == 0) {
            if (prev == NULL) {
                connected_users = user->next;
            } else {
                prev->next = user->next;
            }
            printf("Disconnecting %s\n", user->username);
            free(user);
        }
        prev = user;
    }
}

void process_command(struct command cmd) {
    printf("{\n  type: %ld\n  data: %s\n  user: %s\n}\n", cmd.mtype, cmd.data, cmd.username);

    if (cmd.data[0] == '@' || cmd.data[0] == '#' || cmd.data[0] == '*') {
        struct message msg;
        msg.mtype = 1;
        strncpy(msg.from, cmd.username, MAX_NAME_LENGTH);
        msg.to_symbol = cmd.data[0];
        if (msg.to_symbol == '*') {
            strcpy(msg.to, "");
            strncpy(msg.message, cmd.data + 2, MAX_MESSAGE_LENGTH);
        } else {
            sscanf(cmd.data + 1, "%s", msg.to);
            strncpy(msg.message, cmd.data + strlen(msg.to) + 2, MAX_MESSAGE_LENGTH);
        }

        send_message(msg);
    } else {
        char action[32];
        sscanf(cmd.data, "%s", action);
        char *params = cmd.data + strlen(action) + 1;

        if (cmd.mtype == 1) {
            if (strcmp(action, "users") == 0) {
                send_user_list(cmd.username);
            }
        } else if (cmd.mtype == 2) {
            if (strcmp(action, "login") == 0) {
                key_t key;
                sscanf(params, "%u", &key);
                add_user(cmd.username, key);
            } else if (strcmp(action, "logout") == 0) {
                remove_user(cmd.username);
            }
        }
    }
}

void read_commands() {
    struct command cmd;
    size_t cmd_size = sizeof(cmd) - sizeof(cmd.mtype);

    while (1) {
        if (msgrcv(q, &cmd, cmd_size, 0, 0) == -1) {
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
    signal(SIGINT, handle);
    signal(SIGTERM, handle);

    q = open_queue();
    read_commands();
}