#ifndef _COMMANDS_
#define _COMMANDS_

#include <string>
using namespace std;

#define REGISTER_SUCCESS 201
#define LOGIN_SUCCESS 200
#define PORT 8080

// registers a new user
void register_user(char *host, char *register_path, char *data_type);

// logins an user
void login_user(char *host, char *login_path, char *data_type, string &cookie,
                    string &jwt_token);

// enters the library
void enter_library(char *host, char *library_path, char *data_type, string &cookie,
                    string &jwt_token);

// gets all books
void get_all_books(char *host, char *get_books_path, char *data_type,
                    string jwt_token);

// gets a book by id
void get_book_by_id(char *host, char *get_book_id_path, char *data_type,
                    string jwt_token);

// adds a new book
void add_book(char *host, char *add_book_path, char *data_type, string jwt_token);

// deletes book from library
void delete_book(char *host, char *delete_book_path, char *data_type, string jwt_token);

// logouts an user and deletes jwt and cookie
void logout_user(char *host, char *logout_path, char *data_type, string &cookie,
                    string &jwt_token);

#endif