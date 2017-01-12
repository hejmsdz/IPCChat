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
#include "users.h"
#include "rooms.h"
#include "server_funcs.h"
#include "commands.h"

struct user* connected_users = NULL;
int num_users = 0;
struct room* available_rooms = NULL;
int num_rooms = 0;

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

void process_command(struct command cmd) {
    printf("{\n  type: %ld\n  data: %s\n  user: %s\n}\n", cmd.mtype, cmd.data, cmd.username);

    if (cmd.data[0] == '@' || cmd.data[0] == '#' || cmd.data[0] == '*') {
        cmd_message(cmd.data, cmd.username);
    } else {
        char action[32];
        sscanf(cmd.data, "%s", action);
        char *params = cmd.data + strlen(action) + 1;

        if (cmd.mtype == 1) {
            if (strcmp(action, "users") == 0) {
                cmd_users(cmd.username);
            } else if (strcmp(action, "rooms") == 0) {
                cmd_rooms(cmd.username);
            } else if (strcmp(action, "members") == 0) {
                cmd_members(params, cmd.username);
            } else if (strcmp(action, "join") == 0) {
                cmd_join(params, cmd.username);
            } else if (strcmp(action, "leave") == 0) {
                cmd_leave(params, cmd.username);
            } else if (strcmp(action, "help") == 0) {
                cmd_help(cmd.username);
            }
        } else if (cmd.mtype == 2) {
            if (strcmp(action, "login") == 0) {
                key_t key;
                sscanf(params, "%u", &key);
                cmd_login(cmd.username, key);
            } else if (strcmp(action, "logout") == 0) {
                cmd_logout(cmd.username);
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

    struct room *room;
    struct room_member *member;
    while (available_rooms != NULL) {
        room = available_rooms;

        while(room->members != NULL) {
            member = room->members;
            room->members->next = member;
            free(member);
        }

        available_rooms = room->next;
        free(room);
    }

    printf("Goodbye!\n");
}

void handle(int sig) {
    broadcast(server_message(3, '*', "", "Server received a signal and will terminate."));

    exit(0);
}

int main() {
    atexit(cleanup);
    signal(SIGINT, handle);
    signal(SIGTERM, handle);

    q = open_queue();
    read_commands();
}