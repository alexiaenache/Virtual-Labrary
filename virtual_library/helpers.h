#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

#include <string>
using namespace std;

#define DIE(condition, message, ...) \
	do { \
		if ((condition)) { \
			fprintf(stderr, "[(%s:%d)]: " # message "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
			perror(""); \
			exit(1); \
		} \
	} while (0)


// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// check if buffer is a number
bool is_numeric(char *buffer);

// checks if book details are valid
bool validate_book_details(char *title, char *author, char *genre,
                            char *publisher, char *page_count);

// checks if buffer contains whitespaces
bool contains_whitespace(char *buffer);

// read details for add_book
void read_book_details(char *title, char *author, char *genre,
                        char *publisher, char *page_count);

// read username and password
void read_username_password(char *username, char *password);

// returns response code of request
int get_response_code(char *response);

// returns cookie from response
string get_cookie(char *response);

// returns jwt token
string get_jwt(char *response);

// returns error details
string get_error(char *response);

#endif
