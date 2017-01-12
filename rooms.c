//
// Created by rozwad on 12.01.17.
//

#include <stdio.h>
#include <string.h>
#include "rooms.h"

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

int join_room(char room_name[], char username[]) {
    struct user *user;
    struct room *room, *prev;
    struct room_member *member, *prev_member;

    user = find_user(username, NULL);
    room = find_room(room_name, &prev);

    if (room != NULL) {
        prev_member = room->members;
        for (member = room->members; member != NULL; member = member->next) {
            if (member->member == user) {
                return -2;
            }
            prev_member = member;
        }

        struct room_member *new_member = malloc(sizeof(struct room_member));
        new_member->member = user;
        new_member->next = member;
        prev_member->next = new_member; // this list always has at least one element

        return 1;
    } else {
        if (num_rooms >= MAX_GROUPS) {
            return -1;
        }

        struct room *new_room = malloc(sizeof(struct room));
        strncpy(new_room->name, room_name, MAX_NAME_LENGTH);
        new_room->members = malloc(sizeof(struct room_member));
        new_room->members->member = user;

        *(prev == NULL ? &available_rooms : &prev->next) = new_room;
        num_rooms++;

        return 2;
    }
}

int leave_room(char room_name[], char username[]) {
    struct user *user;
    struct room *room, *prev_room = NULL;
    struct room_member *member, *prev_member = NULL;

    user = find_user(username, NULL);
    room = find_room(room_name, &prev_room);

    if (user == NULL || room == NULL) {
        return -1;
    }

    for (member = room->members; member != NULL; member = member->next) {
        if (member->member == user) {
            *(prev_member == NULL ? &room->members : &prev_member->next) = member->next;
            free(member);
            break;
        }
        prev_member = member;
    }

    if (member == NULL) {
        return -2;
    }

    if (room->members == NULL) {
        *(prev_room == NULL ? &available_rooms : &prev_room->next) = room->next;
        free(room);
        num_rooms--;
        return 2;
    } else {
        return 1;
    }
}

void list_rooms(char *list) {
    if (num_rooms == 0) {
        strcpy(list, "There aren't any open rooms");
        return;
    }

    strcat(list, "Open rooms: ");

    struct room *room;
    for (room = available_rooms; room != NULL; room = room->next) {
        strcat(list, room->name);
        if (room->next != NULL) {
            strcat(list, ", ");
        }
    }
}

void list_members(char room_name[], char *list) {
    struct room *room;

    room = find_room(room_name, NULL);
    if (room == NULL) {
        strcpy(list, "Such room doesn't exist");
    } else {
        sprintf(list, "Users connected to %s: ", room_name);

        struct room_member *member;
        for (member = room->members; member != NULL; member = member->next) {
            strcat(list, member->member->username);
            if (member->next != NULL) {
                strcat(list, ", ");
            }
        }
    }
}