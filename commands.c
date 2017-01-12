//
// Created by rozwad on 12.01.17.
//

#include <sys/msg.h>
#include "commands.h"
#include "users.h"
#include "server_funcs.h"

void cmd_login(char username[], key_t key) {
    int user_q = msgget(key, 0);
    if (user_q == -1) {
        return;
    }

    int result = add_user(username, user_q);
    switch (result) {
        case 1:
            broadcast(server_message(2, '@', username, "Welcome to the server!", 2));
            break;
        case -1:
            send(user_q, server_message(3, '@', username, "No more users are allowed!"));
            break;
        case -2:
            send(user_q, server_message(3, '@', username, "The username @%s is already taken!", username));
            break;
    }
}

void cmd_logout(char username[]) {
    int user_q = remove_user(username);
    send(user_q, server_message(3, '@', username, "Goodbye!"));
}

void cmd_users(char username[]) {
    char buff[MAX_MESSAGE_LENGTH];
    list_users(buff);
    broadcast(server_message(2, '@', username, "%s", buff));
}

void cmd_rooms(char username[]) {
    char buff[MAX_MESSAGE_LENGTH];
    list_rooms(buff);
    broadcast(server_message(2, '@', username, "%s", buff));
}

void cmd_members(char room_name[], char username[]) {
    char buff[MAX_MESSAGE_LENGTH];
    list_members(room_name, buff);
    broadcast(server_message(2, '@', username, "%s", buff));
}

void cmd_join(char room_name[], char username[]) {
    int result = join_room(room_name, username);

    switch (result) {
        case -2:
            broadcast(server_message(2, '@', username, "You are already a member of %s!", room_name));
            break;
        case -1:
            broadcast(server_message(2, '@', username, "No more rooms are allowed!"));
            break;
        case 1:
            broadcast(server_message(2, '@', username, "You joined %s.", room_name));
            break;
        case 2:
            broadcast(server_message(2, '@', username, "You created and joined %s.", room_name));
            break;
    }
}

void cmd_leave(char room_name[], char username[]) {
    int result = leave_room(room_name, username);

    switch (result) {
        case -2:
            broadcast(server_message(2, '@', username, "You weren't a member of %s!", room_name));
            break;
        case -1:
            broadcast(server_message(2, '@', username, "Such room doesn't exist!"));
            break;
        case 1:
            broadcast(server_message(2, '@', username, "You left %s.", room_name));
            break;
        case 2:
            broadcast(server_message(2, '@', username, "You left and removed %s.", room_name));
            break;
    }
}

void cmd_help(char username[]) {
    broadcast(server_message(2, '@', username,
         "Available commands:\n"
         " @[user] [content] - private message\n"
         " @[room] [content] - room message\n"
         " * [content]       - public message\n"
         " users\n"
         " rooms\n"
         " members [room]\n"
         " join [room]\n"
         " leave [room]\n"
         " help\n"
         " logout\n"
    ));
}

void cmd_message(char message[], char username[]) {
    struct message msg;
    msg.mtype = 1;
    strncpy(msg.from, username, MAX_NAME_LENGTH);
    msg.to_symbol = message[0];
    if (msg.to_symbol == '*') {
        strcpy(msg.to, "");
        strncpy(msg.message, message + 2, MAX_MESSAGE_LENGTH);
    } else {
        sscanf(message + 1, "%s", msg.to);
        strncpy(msg.message, message + strlen(msg.to) + 2, MAX_MESSAGE_LENGTH);
    }

    broadcast(msg);
}

void cmd_unknown(char username[]) {
    broadcast(server_message(2, '@', username, "I don't understand! Type 'help' to see available commands."));
}