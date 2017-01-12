//
// Created by rozwad on 12.01.17.
//

#include <stdio.h>
#include <string.h>
#include "users.h"
#include "rooms.h"

bool match_user(struct user *user, char to_symbol, char *to) {
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

int add_user(char username[], int user_q) {
    struct user *user, *prev = NULL;

    if (user_q == -1) {
        perror("Failed to open a user's queue");
        return 0;
    }

    if (num_users >= MAX_USERS) {
        return -1;
    }

    user = find_user(username, &prev);
    if (user != NULL) {
        return -2;
    }

    struct user *new_user = malloc(sizeof(struct user));
    strncpy(new_user->username, username, MAX_NAME_LENGTH);
    new_user->q = user_q;
    new_user->next = NULL;
    *(prev == NULL ? &connected_users : &prev->next) = new_user;
    num_users++;

    return 1;
}


int remove_user(char username[]) {
    struct user *user, *prev;
    struct room *room;

    user = find_user(username, &prev);

    for (room = available_rooms; room != NULL; room = room->next) {
        leave_room(room->name, username);
    }

    if (user == NULL) {
        return -1;
    }

    *(prev == NULL ? &connected_users : &prev->next) = user->next;

    free(user);
    return 1;
}

void list_users(char *list) {
    strncpy(list, "Connected users: ", MAX_MESSAGE_LENGTH);

    struct user *user;
    for (user = connected_users; user != NULL; user = user->next) {
        strcat(list, user->username);
        if (user->next != NULL) {
            strcat(list, ", ");
        }
    }
}