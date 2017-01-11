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
    char username[MAX_NAME_LENGTH];
    int q;
    struct user *next;
};

struct room_member {
    struct user *member;
    struct room_member *next;
};

struct room {
    char name[MAX_NAME_LENGTH];
    struct room_member *members;
    struct room *next;
};

struct user *connected_users = NULL;
struct room *available_rooms = NULL;
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

struct user* find_user(char username[], struct user **prev) {
    if (prev != NULL) {
        *prev = NULL;
    }

    struct user *user;
    for (user = connected_users; user != NULL; user = user->next) {
        if (strcmp(user->username, username) == 0) {
            return user;
        }
        if (prev != NULL) {
            *prev = user;
        }
    }
    return NULL;
}

struct room* find_room(char name[], struct room **prev) {
    if (prev != NULL) {
        *prev = NULL;
    }

    struct room *room;
    for (room = available_rooms; room != NULL; room = room->next) {
        if (strcmp(room->name, name) == 0) {
            return room;
        }
        if (prev != NULL) {
            *prev = room;
        }
    }
    return NULL;
}

void send_message(struct message msg) {
    size_t msg_size = sizeof(msg) - sizeof(msg.mtype);

    if (msg.to_symbol == '#') {
        struct room *room;
        room = find_room(msg.to, NULL);

        struct room_member *member;
        for (member = room->members; member != NULL; member = member->next) {
            if (msgsnd(member->member->q, &msg, msg_size, 0) == -1) {
                perror("Failed to send a message");
            }
        }
    } else {
        struct user *user;
        for (user = connected_users; user != NULL; user = user->next) {
            if (user_match(user, msg.to_symbol, msg.to)) {
                if (msgsnd(user->q, &msg, msg_size, 0) == -1) {
                    perror("Failed to send a message");
                }
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

void send_room_list(char username[]) {
    char rooms_list[MAX_MESSAGE_LENGTH] = "Available rooms: ";

    if (available_rooms == NULL) {
        strcpy(rooms_list, "There aren't any active rooms");
    } else {
        struct room *room;
        for (room = available_rooms; room != NULL; room = room->next) {
            strcat(rooms_list, room->name);
            if (room->next != NULL) {
                strcat(rooms_list, ", ");
            }
        }
    }

    send_server_message('@', username, rooms_list);
}

void send_member_list(char room_name[], char username[]) {
    struct room *room;
    char members_list[MAX_MESSAGE_LENGTH];

    room = find_room(room_name, NULL);
    if (room == NULL) {
        strcpy(members_list, "Such room doesn't exist");
    } else {
        sprintf(members_list, "Users connected to %s: ", room_name);

        struct room_member *member;
        for (member = room->members; member != NULL; member = member->next) {
            strcat(members_list, member->member->username);
            if (member->next != NULL) {
                strcat(members_list, ", ");
            }
        }
    }

    send_server_message('@', username, members_list);
}

void join_room(char room_name[], char username[]) {
    struct user *user;
    struct room *room, *prev;

    user = find_user(username, NULL);
    room = find_room(room_name, &prev);

    if (room != NULL) {
        struct room_member *member = malloc(sizeof(struct room_member));
        member->member = user;
        member->next = room->members;
        room->members = member;

        send_server_message('@', username, "You joined a room");
    } else {
        struct room *new_room = malloc(sizeof(struct room));
        strncpy(new_room->name, room_name, MAX_NAME_LENGTH);
        new_room->members = malloc(sizeof(struct room_member));
        new_room->members->member = user;

        *(prev == NULL ? &available_rooms : &prev->next) = new_room;

        send_server_message('@', username, "You created and joined a room");
    }
}


void leave_room(char room_name[], char username[]) {
    struct user *user;
    struct room *room, *prev_room = NULL;
    struct room_member *member, *prev_member = NULL;

    user = find_user(username, NULL);
    room = find_room(room_name, &prev_room);

    if (user == NULL || room == NULL) {
        return;
    }

    for (member = room->members; member != NULL; member = member->next) {
        if (member->member == user) {
            *(prev_member == NULL ? &room->members : &prev_member->next) = member->next;
            free(member);
            break;
        }
        prev_member = member;
    }

    if (room->members == NULL) {
        *(prev_room == NULL ? &available_rooms : &prev_room->next) = room->next;
        free(room);
        send_server_message('@', username, "You left and removed a room");
    } else {
        send_server_message('@', username, "You left a room");
    }
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
            } else if (strcmp(action, "rooms") == 0) {
                send_room_list(cmd.username);
            } else if (strcmp(action, "members") == 0) {
                char room_name[MAX_NAME_LENGTH];
                sscanf(params, "%s", room_name);
                send_member_list(room_name, cmd.username);
            } else if (strcmp(action, "join") == 0) {
                char room_name[MAX_NAME_LENGTH];
                sscanf(params, "%s", room_name);
                join_room(room_name, cmd.username);
            } else if (strcmp(action, "leave") == 0) {
                char room_name[MAX_NAME_LENGTH];
                sscanf(params, "%s", room_name);
                leave_room(room_name, cmd.username);
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