#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>     /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <string>
#include "helpers.h"
#include "buffer.h"

using namespace std;

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

bool is_numeric(char *buffer) {
    for (size_t i = 0; i < strlen(buffer); i++) {
        if (!isdigit(buffer[i])) {
            return false;
        }
    }
    return true;
}

bool validate_book_details(char *title, char *author, char *genre,
                            char *publisher, char *page_count) {
    if (!strlen(title) || !strlen(author) || !strlen(genre)
        || !strlen(publisher) || !strlen(page_count)) {
        return false;
    }

    bool is_num = is_numeric(page_count);
    if (!is_num) {
        return false;
    }
    return true;
}

bool contains_whitespace(char *buffer) {
    char *space = strchr(buffer, ' ');
    char *tab = strchr(buffer, '\t');
    char *newline = strchr(buffer, '\n');
    if (space || tab || newline)
        return false;
    return true;
}

void read_book_details(char *title, char *author, char *genre,
                        char *publisher, char *page_count) {
    printf("title= ");
    fgets(title, LINELEN, stdin);
    
    printf("author= ");
    fgets(author, LINELEN, stdin);
    
    printf("genre= ");
    fgets(genre, LINELEN, stdin);
    
    printf("publisher= ");
    fgets(publisher, LINELEN, stdin);
    
    printf("page_count= ");
    fgets(page_count, LINELEN, stdin);

    // remove \n from strings
    title[strcspn(title, "\n")] = 0;
    author[strcspn(author, "\n")] = 0;
    genre[strcspn(genre, "\n")] = 0;
    publisher[strcspn(publisher, "\n")] = 0;
    page_count[strcspn(page_count, "\n")] = 0;
}

void read_username_password(char *username, char *password) {
    printf("username= ");
    fgets(username, LINELEN, stdin);
    
    printf("password= ");
    fgets(password, LINELEN, stdin);
    
    // remove newline characters
    username[strcspn(username, "\n")] = 0;
    password[strcspn(password, "\n")] = 0;
}

int get_response_code(char *response) {
    char *copy = strdup(response);
    char *token = strtok(copy, " ");
    token = strtok(NULL, " ");
    return atoi(token);
}

string get_cookie(char *response) {
    char *header = strstr(response, "Set-Cookie");
    
    // if there is no Set-Cookie header, return empty string
    if (header == NULL) {
        return "";
    }

    char *copy = strdup(header);
    char *token = strtok(copy, " ;");
    token = strtok(NULL, " ;");
    return string(token);
}

string get_jwt(char *response) {
    char *header = strstr(response, "token");
    
    // if there is no error field, return empty string
    if (header == NULL) {
        return "";
    }

    char *copy = strdup(header);
    char *token = strtok(copy, "{}:\"");
    token = strtok(NULL, "{}:\"");
    return string(token);
}

string get_error(char *response) {
    char *header = strstr(response, "error");
    
    // if there is no error field, return empty string
    if (header == NULL) {
        return "";
    }

    char *copy = strdup(header);
    char *token = strtok(copy, "{}:\"");
    token = strtok(NULL, "{}:\"");
    return string(token);
}
