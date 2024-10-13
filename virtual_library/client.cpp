#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "commands.h"

using namespace std;

int main() {
    char *command;
    string cookie = "";
    string jwt_token = "";

    char server_ip[] = "34.246.184.49";
    char register_path[] = "/api/v1/tema/auth/register";
    char login_path[] = "/api/v1/tema/auth/login";
    char logout_path[] = "/api/v1/tema/auth/logout";
    char library_access_path[] = "/api/v1/tema/library/access";
    char get_books_path[] = "/api/v1/tema/library/books/";
    char json_type[] = "application/json";

    while (1) {
        command = (char *)calloc(LINELEN, sizeof(char));
        DIE(command == NULL, "memory allocation failed");

        bool valid_command = false;

        // read user command from stdin
        fgets(command, LINELEN, stdin);

        if (!strcmp(command, "register\n")) {
            register_user(server_ip, register_path, json_type);
            valid_command = true;
        }

        if (!strcmp(command, "login\n")) {
            login_user(server_ip, login_path, json_type, cookie, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "enter_library\n")) {
            enter_library(server_ip, library_access_path, json_type, cookie, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "get_books\n")) {
            get_all_books(server_ip, get_books_path, json_type, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "get_book\n")) {
            get_book_by_id(server_ip, get_books_path, json_type, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "add_book\n")) {
            add_book(server_ip, get_books_path, json_type, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "delete_book\n")) {
            delete_book(server_ip, get_books_path, json_type, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "logout\n")) {
            logout_user(server_ip, logout_path, json_type, cookie, jwt_token);
            valid_command = true;
        }

        if (!strcmp(command, "exit\n")) {
            free(command);
            valid_command = true;
            break;
        }

        if (!valid_command) {
            printf("ERROR: Invalid command\n");
        }

        free(command);
    }

}