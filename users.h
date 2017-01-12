//
// Created by rozwad on 12.01.17.
//

#include <stdlib.h>
#include <stdbool.h>
#include "ipcchat.h"

#ifndef IPCCHAT_USERS_H
#define IPCCHAT_USERS_H

struct user {
    char username[MAX_NAME_LENGTH];
    int q;
    struct user *next;
};

extern struct user *connected_users;
extern int num_users;

bool match_user(struct user *user, char to_symbol, char *to);
struct user* find_user(char username[], struct user **prev);
int add_user(char username[], int user_q);
int remove_user(char username[]);
void list_users(char *list);

#endif //IPCCHAT_USERS_H
