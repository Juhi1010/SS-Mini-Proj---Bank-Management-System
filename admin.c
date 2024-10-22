#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include "./helper-functions/admin.h"
#include "./helper-functions/defines.h"


#define BUFFER_SIZE 1024
#define MAX_USERNAME 50
#define MAX_PASSWORD 50

void get_admin_menu(char *menu, int size) {
    snprintf(menu, size, "Admin Menu:\n1. Add New Bank Employee\n2. Modify Customer/Employee Details\n3. Manage User Roles\n4. Logout\n5. Exit\nEnter your choice: ");
}

void process_admin_choice(client_info *client, int choice) {
    switch (choice) {
        case 1:
            add_new_employee(client);
            break;
        case 2:
            modify_employee_details(client);
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        default:
            send(client->client_socket, "Invalid choice!\n", 16, 0);
            break;
    }
}

// void add_new_employee(client_info *client) {

//     int client_socket = client->client_socket;
//     char username[MAX_USERNAME], password[MAX_PASSWORD];

//     send(client_socket, "Enter new employee username: ", 30, 0);
//     recv(client_socket, username, sizeof(username) - 1, 0);
//     username[strcspn(username, "\n")] = '\0';

//     send(client_socket, "Enter password: ", 17, 0);
//     recv(client_socket, password, sizeof(password) - 1, 0);
//     password[strcspn(password, "\n")] = '\0';

//     FILE *employees_file = fopen(EMPLOYEES, "a");
//     if (employees_file == NULL) {
//         perror("Error opening employees file");
//         send(client_socket, "Error opening employees file.\n", 31, 0);
//         return;
//     }

//     fprintf(employees_file, "%s %s %s\n", username, password, "employee");
//     fclose(employees_file);

//     char success_msg[BUFFER_SIZE];
//     snprintf(success_msg, sizeof(success_msg), "Employee added successfully: %s\n", username);
//     send(client_socket, success_msg, strlen(success_msg), 0);

// }

void add_new_employee(client_info *client) {

    int client_socket = client->client_socket;

    char username[MAX_USERNAME] = {0};
    char password[MAX_PASSWORD] = {0};
    char role[20] = {0};
    char response[BUFFER_SIZE];

    // Send prompt to client to enter the username
    send(client_socket, "Enter new employee username: ", 29, 0);
    if (recv(client_socket, username, sizeof(username), 0) <= 0) {
        printf("Error receiving username or client disconnected.\n");
        return;
    }

    // Send prompt to client to enter the password
    send(client_socket, "Enter new employee password: ", 29, 0);
    if (recv(client_socket, password, sizeof(password), 0) <= 0) {
        printf("Error receiving password or client disconnected.\n");
        return;
    }


    FILE* file = fopen(EMPLOYEES, "a");
    if (file == NULL) {
        send(client_socket, "Error adding new employee\n", 26, 0);
    } else {
        fprintf(file, "%s %s employee\n", username, password);
        fclose(file);
        send(client_socket, "New employee added successfully\n", 32, 0);
    }
}

   

void modify_employee_details(client_info *client) {

    int client_socket = client->client_socket;
    char username[MAX_USERNAME], new_username[MAX_USERNAME], role[MAX_USERNAME];
    char buffer[BUFFER_SIZE];
    int found = 0;

    send(client_socket, "Enter the username of the employee to modify: ", 46, 0);
    recv(client_socket, username, sizeof(username) - 1, 0);
    username[strcspn(username, "\n")] = '\0';

    FILE *employees_file = fopen(EMPLOYEES, "r");
    FILE *temp_file = fopen("temp_employees.txt", "w");
    if (employees_file == NULL || temp_file == NULL) {
        perror("Error opening file");
        send(client_socket, "Error opening file.\n", 21, 0);
        return;
    }

    char file_username[MAX_USERNAME], file_password[MAX_PASSWORD], file_role[MAX_USERNAME];

    while (fscanf(employees_file, "%s %s %s", file_username, file_password, file_role) == 3) {
        if (strcmp(username, file_username) == 0) {
            found = 1;

            send(client_socket, "Enter the new username (or press enter to keep the same): ", 57, 0);
            recv(client_socket, new_username, sizeof(new_username) - 1, 0);
            new_username[strcspn(new_username, "\n")] = '\0';

            if (strlen(new_username) == 0) {
                strcpy(new_username, file_username);
            }

            send(client_socket, "Enter the new role (only 'manager' is allowed): ", 47, 0);
            recv(client_socket, role, sizeof(role) - 1, 0);
            role[strcspn(role, "\n")] = '\0';

            if (strcmp(role, "manager") == 0) {
                FILE *managers_file = fopen(MANAGERS, "a");
                if (managers_file == NULL) {
                    perror("Error opening managers file");
                    send(client_socket, "Error opening managers file.\n", 30, 0);
                    fclose(employees_file);
                    fclose(temp_file);
                    return;
                }
                fprintf(managers_file, "%s %s manager\n", new_username, file_password);
                fclose(managers_file);

                send(client_socket, "Employee role changed to manager and moved to managers file.\n", 61, 0);
            } else {
                fprintf(temp_file, "%s %s %s\n", new_username, file_password, "employee");
                send(client_socket, "Employee details updated successfully.\n", 38, 0);
            }
        } else {
            fprintf(temp_file, "%s %s %s\n", file_username, file_password, file_role);
        }
    }

    fclose(employees_file);
    fclose(temp_file);

    if (found) {
        remove(EMPLOYEES);
        rename("temp_employees.txt", EMPLOYEES);
    } else {
        send(client_socket, "Employee not found.\n", 20, 0);
        remove("temp_employees.txt");
    }

}

void send_prompt(client_info *client, const char *prompt) {
    send(client->client_socket, prompt, strlen(prompt), 0);
}



