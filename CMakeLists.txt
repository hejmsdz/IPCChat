cmake_minimum_required(VERSION 3.6)
project(IPCChat C)

set(SOURCE_FILES ipcchat.h)
add_executable(client ${SOURCE_FILES} client.c)
add_executable(server ${SOURCE_FILES} users.c rooms.c server.c commands.c commands.h server_funcs.c server_funcs.h)

target_link_libraries(client pthread)