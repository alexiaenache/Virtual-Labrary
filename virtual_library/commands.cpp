#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <string>
#include "helpers.h"
#include "requests.h"
#include "commands.h"
#include "parson.h"

void register_user(char *host, char *register_path, char *data_type) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

    char *username = (char *)calloc(LINELEN, sizeof(char));
    char *password = (char *)calloc(LINELEN, sizeof(char));
    DIE(username == NULL || password == NULL, "memory allocation failed");
    read_username_password(username, password);

    // check if data is valid
    bool valid_username = contains_whitespace(username);
    bool valid_password = contains_whitespace(password);
    if (!valid_username || !valid_password) {
        printf("ERROR: username and password must not contain spaces\n");
        free(username);
        free(password);
        close(sockfd);
        return;
    }

    JSON_Value *root_value;
    JSON_Object *root_object;
    char *serialized_string = NULL;

    // convert data in JSON
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    serialized_string = json_serialize_to_string_pretty(root_value);
            
    char **body_data = (char **)calloc(1, sizeof(char *));
    DIE(body_data == NULL, "memory allocation failed");
    body_data[0] = (char *)calloc(LINELEN, sizeof(char));
    DIE(body_data[0] == NULL, "memory allocation failed");
    strcpy(body_data[0], serialized_string);

    // we dont need to put any cookies in the register request
    char *message = compute_post_request(host, register_path,data_type,
                                            body_data, 1, NULL, 0, "");

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == REGISTER_SUCCESS) {
        printf("SUCCESS: user %s successfully registered\n", username);
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    close(sockfd);
    free(username);
    free(password);
    free(body_data[0]);
    free(body_data);
    free(message);
    free(response);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void login_user(char *host, char *login_path, char *data_type, string &cookie,
                    string &jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

    char *username = (char *)calloc(LINELEN, sizeof(char));
    char *password = (char *)calloc(LINELEN, sizeof(char));
    DIE(username == NULL || password == NULL, "memory allocation failed");
    read_username_password(username, password);

    // check if data is valid
    bool valid_username = contains_whitespace(username);
    bool valid_password = contains_whitespace(password);
    if (!valid_username || !valid_password) {
        printf("ERROR: username and password must not contain spaces\n");
        free(username);
        free(password);
        close(sockfd);
        return;
    }

    cookie = "";
    jwt_token = "";

    // convert in JSON
    JSON_Value *root_value;
    JSON_Object *root_object;
    char *serialized_string = NULL;

    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    serialized_string = json_serialize_to_string_pretty(root_value);
    
    char **body_data = (char **)calloc(1, sizeof(char *));
    DIE(body_data == NULL, "memory allocation failed");
    body_data[0] = (char *)calloc(LINELEN, sizeof(char));
    DIE(body_data[0] == NULL, "memory allocation failed");
    strcpy(body_data[0], serialized_string);

    // we dont need to put any cookies in the login request
    char *message = compute_post_request(host, login_path, data_type,
                                    body_data, 1, NULL, 0, "");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == LOGIN_SUCCESS) {
        printf("SUCCESS: logged in as user %s\n", username);
        cookie = get_cookie(response);
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    close(sockfd);
    free(username);
    free(password);
    free(body_data[0]);
    free(body_data);
    free(message);
    free(response);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void enter_library(char *host, char *library_path, char *data_type, string &cookie,
                    string &jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

    if (cookie == "") {
        printf("ERROR: not logged in.\n");
        close(sockfd);
        return;
    }
    
    // send out cookie to the server
    char **cookie_ptr = (char **)calloc(1, sizeof(char *));
    DIE(cookie_ptr == NULL, "memory allocation failed");
    cookie_ptr[0] = (char *)calloc(LINELEN, sizeof(char));
    DIE(cookie_ptr[0] == NULL, "memory allocation failed");
    strcpy(cookie_ptr[0], cookie.c_str());

    char *message = compute_get_request(host, library_path, NULL, cookie_ptr, 1, "");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == 200) {
        printf("SUCCESS: entered library\n");
        // extract jwt from response
        jwt_token = get_jwt(response);
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    close(sockfd);
    free(cookie_ptr[0]);
    free(cookie_ptr);
    free(message);
    free(response);
}

void get_all_books(char *host, char *get_books_path, char *data_type,
                    string jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

    if (jwt_token == "") {
        printf("ERROR: no access in library.\n");
        close(sockfd);
        return;
    }

    char *message = compute_get_request(host, get_books_path, NULL, NULL, 0, jwt_token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == 200) {
        printf("SUCCESS: got all books: \n");
        char *content = strstr(response, "{");
        if (content != NULL)
            printf("%s\n", content);
        else
            printf("\n");
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    free(message);
    free(response);
    close(sockfd);
}

void get_book_by_id(char *host, char *get_book_id_path, char *data_type,
                    string jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);
    char *buffer = (char *)calloc(LINELEN, sizeof(char));
    DIE(buffer == NULL, "memory allocation failed");

    printf("id= ");
    fgets(buffer, LINELEN, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    bool valid = true;
    if (!strlen(buffer)) {
        valid = false;
    }

    bool check_num = is_numeric(buffer);

    if (!valid || !check_num) {
        printf("ERROR: ID must be numeric.\n");
        close(sockfd);
        free(buffer);
        return;
    }

    if (jwt_token == "") {
        printf("ERROR: no access in library.\n");
        close(sockfd);
        return;
    }

    // make url
    char *book_path = (char *)calloc(LINELEN, sizeof(char));
    DIE(book_path == NULL, "memory allocation failed");
    strcpy(book_path, get_book_id_path);
    strcat(book_path, buffer);

    char *message = compute_get_request(host, book_path, NULL, NULL, 0, jwt_token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == 200) {
        printf("SUCCESS: book details: ");
        char *content = strstr(response, "{");
        if (content != NULL)
            printf("%s\n", content);
        else {
            printf("\n");
        }
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    free(message);
    free(book_path);
    free(response);
    close(sockfd);
}

void add_book(char *host, char *add_book_path, char *data_type, string jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

    char *title = (char *)calloc(LINELEN, sizeof(char));
    char *author = (char *)calloc(LINELEN, sizeof(char));
    char *genre = (char *)calloc(LINELEN, sizeof(char));
    char *publisher = (char *)calloc(LINELEN, sizeof(char));
    char *page_count = (char *)calloc(LINELEN, sizeof(char));
    DIE(title == NULL || author == NULL || genre == NULL
        || publisher == NULL || page_count == NULL, "memory allocation failed");
    read_book_details(title, author, genre, publisher, page_count);

    bool valid_details = validate_book_details(title, author, genre,
                                                publisher, page_count);
    if (!valid_details) {
        printf("ERROR: invalid format for book fields.\n");
        close(sockfd);
        return;
    }

    if (jwt_token == "") {
        printf("ERROR: no access in library.\n");
        close(sockfd);
        return;
    }

    JSON_Value *root_value;
    JSON_Object *root_object;
    char *serialized_string = NULL;

    // convert data in JSON
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    int page_count_num = atoi(page_count);
    json_object_set_number(root_object, "page_count", page_count_num);

    serialized_string = json_serialize_to_string_pretty(root_value);

    char **body_data = (char **)calloc(1, sizeof(char *));
    DIE(body_data == NULL, "memory allocation failed");
    body_data[0] = (char *)calloc(LINELEN, sizeof(char));
    DIE(body_data[0] == NULL, "memory allocation failed");
    strcpy(body_data[0], serialized_string);

    char *message = compute_post_request(host, add_book_path, data_type,
                                            body_data, 1, NULL, 0, jwt_token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == LOGIN_SUCCESS) {
        printf("SUCCESS: added new book\n");
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    close(sockfd);
    free(body_data[0]);
    free(body_data);
    free(message);
    free(response);
    free(author);
    free(title);
    free(genre);
    free(publisher);
    free(page_count);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void delete_book(char *host, char *delete_book_path, char *data_type,
                    string jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);
    char *buffer = (char *)calloc(LINELEN, sizeof(char));
    DIE(buffer == NULL, "memory allocation failed");

    printf("id= ");
    fgets(buffer, LINELEN, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    bool valid = true;
    if (!strlen(buffer)) {
        valid = false;
    }

    bool check_num = is_numeric(buffer);

    if (!valid || !check_num) {
        printf("ERROR: ID must be numeric.\n");
        close(sockfd);
        free(buffer);
        return;
    }

    if (jwt_token == "") {
        printf("ERROR: no access in library.\n");
        close(sockfd);
        free(buffer);
        return;
    }

    // copy book id path
    char *book_path = (char *)calloc(LINELEN, sizeof(char));
    DIE(book_path == NULL, "memory allocation failed");
    strcpy(book_path, delete_book_path);
    strcat(book_path, buffer);

    char *message = compute_delete_request(delete_book_path, book_path,
                                            NULL, NULL, 0, jwt_token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == 200) {
        printf("SUCCESS: deleted book with id: %s\n", buffer);
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    free(message);
    free(response);
    free(buffer);
    close(sockfd);
}

void logout_user(char *host, char *logout_path, char *data_type, string &cookie,
                    string &jwt_token) {
    int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

    if (cookie == "") {
        printf("ERROR: not logged in.\n");
        close(sockfd);
        return;
    }
    
    char **cookie_ptr = (char **)calloc(1, sizeof(char *));
    DIE(cookie_ptr == NULL, "memory allocation failed");
    cookie_ptr[0] = (char *)calloc(LINELEN, sizeof(char));
    DIE(cookie_ptr[0] == NULL, "memory allocation failed");
    strcpy(cookie_ptr[0], cookie.c_str());

    char *message = compute_get_request(host, logout_path, NULL, cookie_ptr, 1, "");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (get_response_code(response) == 200) {
        printf("SUCCESS: logged out successfully\n");
        jwt_token = "";
        cookie = "";
    } else {
        string error = get_error(response);
        printf("ERROR: %s\n", error.c_str());
    }

    free(message);
    free(cookie_ptr[0]);
    free(cookie_ptr);
    free(response);
    close(sockfd);
}