cmake_minimum_required(VERSION 3.29)
project(Chat C)

set(CMAKE_C_STANDARD 11)

add_executable(Chat main.c
        ChatObject.c
        server/serveur.c
        client/client.c)
