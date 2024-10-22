#ifndef CLIENT
#define CLIENT

typedef struct {
    int client_socket;
    char logged_in_user[50]; // username of the logged-in user for the session
} client_info;

#endif