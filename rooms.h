//
// Created by rozwad on 12.01.17.
//

#include <stdlib.h>
#include "ipcchat.h"
#include "users.h"

#ifndef IPCCHAT_ROOMS_H
#define IPCCHAT_ROOMS_H

struct room_member {
    struct user *member;
    struct room_member *next;
};

struct room {
    char name[MAX_NAME_LENGTH];
    struct room_member *members;
    struct room *next;
};

extern struct room *available_rooms;
extern int num_rooms;

struct room* find_room(char name[], struct room **prev);
int join_room(char room_name[], char username[]);
int leave_room(char room_name[], char username[]);
void list_rooms(char *list);
void list_members(char room_name[], char *list);

#endif //IPCCHAT_ROOMS_H
