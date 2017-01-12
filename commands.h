//
// Created by rozwad on 12.01.17.
//

#ifndef IPCCHAT_COMMANDS_H
#define IPCCHAT_COMMANDS_H

#include <sys/ipc.h>

extern struct user *connected_users;
extern int num_users;
extern struct room *available_rooms;
extern int num_rooms;

void cmd_login(char username[], key_t key);
void cmd_logout(char username[]);

void cmd_users(char username[]);
void cmd_rooms(char username[]);
void cmd_members(char room_name[], char username[]);
void cmd_join(char room_name[], char username[]);
void cmd_leave(char room_name[], char username[]);

void cmd_message(char message[], char username[]);

#endif //IPCCHAT_COMMANDS_H
