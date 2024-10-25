#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "./helper-functions/customer.h"
#include "./helper-functions/employee.h"
#include "./helper-functions/manager.h"
#include "./helper-functions/admin.h"
#include "./helper-functions/login_manager.h"
#include "./helper-functions/record-struct/client.h" 
#include "./helper-functions/defines.h" 

// #define BUFFER_SIZE 1024

void *handle_client(void *arg);
void authenticate(client_info *client, const char *filename);
int check_credentials(FILE *file, const char *username, const char *password);
void send_message(int client_socket, const char *message);
void handle_customer(client_info *client);
void handle_employee(client_info *client);
void handle_admin(client_info *client);
void handle_manager(client_info *client);

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    int client_socket = client->client_socket;
    char buffer[BUFFER_SIZE];

    // Prompt user for role
    send_message(client_socket, "Choose a role:\n1. Customer\n2. Employee\n3. Manager\n4. Admin\nEnter your choice: ");
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        printf("Error receiving role choice. Closing connection.\n");
        close(client_socket);
        free(client);
        return NULL;
    }

    int choice = atoi(buffer);
    const char *filename;
    int authenticated;

    switch (choice) {
        case 1:
            authenticate(client, CUSTOMERS);
            authenticated = strlen(client->logged_in_user) > 0; 
            if (authenticated) {
                handle_customer(client);
            }
            break;
        case 2:
            authenticate(client, EMPLOYEES);
            authenticated = strlen(client->logged_in_user) > 0; 
            if (authenticated) {
                handle_employee(client);
            }
            break;
        case 3:
            authenticate(client, MANAGERS);
            authenticated = strlen(client->logged_in_user) > 0; 
            if (authenticated) {
                handle_manager(client);
            }
            break;
        case 4:
            authenticate(client, ADMIN);
            authenticated = strlen(client->logged_in_user) > 0; 
            if (authenticated) {
                handle_admin(client);
            }
            break;
        default:
            send_message(client_socket, "Invalid choice! Disconnecting...\n");
            close(client_socket);
            free(client);
            return NULL;
    }

    if (authenticated) {
        log_out_user(client->logged_in_user);
    }

    close(client_socket);
    free(client);
    return NULL;
}

void authenticate(client_info *client, const char *filename) {
    char username[50], password[50];
    char buffer[BUFFER_SIZE];

    send_message(client->client_socket, "Enter username: ");
    memset(username, 0, sizeof(username));  
    int bytes_received = recv(client->client_socket, username, sizeof(username) - 1, 0);
    if (bytes_received <= 0) {
        send_message(client->client_socket, "Error receiving username.\n");
        return;
    }
    username[strcspn(username, "\n")] = '\0';  

    send_message(client->client_socket, "Enter password: ");
    memset(password, 0, sizeof(password));  
    bytes_received = recv(client->client_socket, password, sizeof(password) - 1, 0);
    if (bytes_received <= 0) {
        send_message(client->client_socket, "Error receiving password.\n");
        return;
    }
    password[strcspn(password, "\n")] = '\0';  

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        send_message(client->client_socket, "Error opening file.\n");
        return;
    }


    if (check_credentials(file, username, password) && log_in_user(username)) {
        send_message(client->client_socket, "Authenticated.\n"); 
        strncpy(client->logged_in_user, username, sizeof(client->logged_in_user) - 1);
        client->logged_in_user[sizeof(client->logged_in_user) - 1] = '\0';  
    } else {
        send_message(client->client_socket, "Invalid credentials.\n");
        log_out_user(username); 
    }

    fclose(file);
}

void handle_customer(client_info *client) {
    char menu[BUFFER_SIZE];
    int choice;
    int client_socket = client->client_socket;

    while (1) {
        get_customer_menu(menu, sizeof(menu));
        send(client_socket, menu, strlen(menu), 0);

        if (recv(client_socket, &choice, sizeof(choice), 0) <= 0) {
            printf("Client disconnected.\n");
            close(client_socket);
            return;
        }

        printf("Received choice: %d\n", choice); 

        process_customer_choice(client, choice);

    }

    close(client_socket);
}

void handle_employee(client_info *client) {
    char menu[BUFFER_SIZE];
    int choice;
    int client_socket = client->client_socket;

    while (1) {
        get_employee_menu(menu, sizeof(menu));
        send(client_socket, menu, strlen(menu), 0);

        if (recv(client_socket, &choice, sizeof(choice), 0) <= 0) {
            printf("Client disconnected.\n");
            close(client_socket);
            return;
        }

        printf("Received choice: %d\n", choice); 

        process_employee_choice(client, choice);

    }

    close(client_socket);
}

void handle_admin(client_info *client) {
    char menu[BUFFER_SIZE];
    int choice;
    int client_socket = client->client_socket;

    while (1) {
        get_admin_menu(menu, sizeof(menu));
        send(client_socket, menu, strlen(menu), 0);

        if (recv(client_socket, &choice, sizeof(choice), 0) <= 0) {
            printf("Client disconnected.\n");
            close(client_socket);
            return;
        }

        printf("Received choice: %d\n", choice); 

        process_admin_choice(client, choice);

    }

    close(client_socket);
}

void handle_manager(client_info *client) {
    char menu[BUFFER_SIZE];
    int choice;
    int client_socket = client->client_socket;

    while (1) {
        get_manager_menu(menu, sizeof(menu));
        send(client_socket, menu, strlen(menu), 0);

        if (recv(client_socket, &choice, sizeof(choice), 0) <= 0) {
            printf("Client disconnected.\n");
            close(client_socket);
            return;
        }

        printf("Received choice: %d\n", choice); 

        process_manager_choice(client, choice);

    }

    close(client_socket);
}

int check_credentials(FILE *file, const char *username, const char *password) {
    char line[BUFFER_SIZE];
    char file_username[50], file_password[50];

    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%s %s", file_username, file_password);
        if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0) {
            return 1;  
        }
    }
    return 0;  
}

void send_message(int client_socket, const char *message) {
    send(client_socket, message, strlen(message), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept clients and handle them in separate threads
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            perror("Accept");
            continue;
        }

        pthread_t thread_id;
        client_info *client = malloc(sizeof(client_info));
        client->client_socket = client_socket;
        memset(client->logged_in_user, 0, sizeof(client->logged_in_user));

        // Create a new thread for each client connection
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client) != 0) {
            perror("Could not create thread");
            free(client);
            close(client_socket);
            continue;
        }

        pthread_detach(thread_id);
    }

    return 0;
}
