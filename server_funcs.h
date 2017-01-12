//
// Created by rozwad on 12.01.17.
//

#ifndef IPCCHAT_SERVER_FUNCS_H
#define IPCCHAT_SERVER_FUNCS_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/msg.h>
#include "users.h"
#include "rooms.h"
#include "ipcchat.h"

extern struct user *connected_users;
extern int num_users;
extern struct room *available_rooms ;
extern int num_rooms;

bool send(int queue, struct message msg);
int broadcast(struct message msg);
struct message server_message(long mtype, char to_symbol, char *to, char *format, ...);

#endif //IPCCHAT_SERVER_FUNCS_H
