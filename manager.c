#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h> 
#include "./helper-functions/manager.h"
#include "./helper-functions/defines.h"


void get_manager_menu(char *menu, int size) {
    snprintf(menu, size, "Manager Menu:\n1. Activate/Deactivate Customer Accounts\n2. Assign Loan Application Processes to Employees\n3. Review Customer Feedback\n4. Exit\nEnter your choice: ");
}

void process_manager_choice(client_info *client, int choice) {
    switch (choice) {
        case 1:
            activation(client);
            break;
        case 2:
            assign_loan_applications(client);
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

void activation(client_info *client) {
    char buffer[BUFFER_SIZE];
    int account_number, is_active, balance;
    char response;
    FILE *file, *temp_file;
    int records_changed = 0;

    send(client->client_socket, "Enter account number: ", 22, 0);
    recv(client->client_socket, buffer, BUFFER_SIZE, 0);
    account_number = atoi(buffer);

    file = fopen(ACCOUNTS, "r");
    if (file == NULL) {
        send(client->client_socket, "Error: Unable to open accounts file.\n", 37, 0);
        return;
    }

    temp_file = fopen("temp_accounts.txt", "w");
    if (temp_file == NULL) {
        fclose(file);
        send(client->client_socket, "Error: Unable to create temporary file.\n", 40, 0);
        return;
    }

    while (fscanf(file, "%d %d %d", &account_number, &is_active, &balance) == 3) {
        if (account_number == atoi(buffer)) {
            snprintf(buffer, BUFFER_SIZE, "Account %d found. Current status: %s\nDo you want to change the status? (Y/N): ", 
                     account_number, is_active ? "Active" : "Inactive");
            send(client->client_socket, buffer, strlen(buffer), 0);
            recv(client->client_socket, &response, 1, 0);

            if (response == 'Y' || response == 'y') {
                is_active = !is_active;  
                records_changed = 1;
                snprintf(buffer, BUFFER_SIZE, "Account status changed to: %s\n", is_active ? "Active" : "Inactive");
                send(client->client_socket, buffer, strlen(buffer), 0);
            } else {
                send(client->client_socket, "Account status not changed.\n", 28, 0);
            }
        }
        fprintf(temp_file, "%d %d %d\n", account_number, is_active, balance);
    }

    fclose(file);
    fclose(temp_file);

    if (records_changed) {
        remove(ACCOUNTS);
        rename("temp_accounts.txt", ACCOUNTS);
        send(client->client_socket, "Account status updated successfully.\n", 36, 0);
    } else {
        remove("temp_accounts.txt");
        send(client->client_socket, "Account not found or no changes made.\n", 38, 0);
    }
}

void assign_loan_applications(client_info *client) {
    char buffer[BUFFER_SIZE];
    char customer_username[MAX_USERNAME] = {0};
    char employee_username[MAX_USERNAME] = {0};
    char temp_file[] = "temp_loans.txt";
    
    snprintf(buffer, sizeof(buffer), "Enter the customer username for loan assignment: ");
    send(client->client_socket, buffer, strlen(buffer), 0);

    if (recv(client->client_socket, customer_username, sizeof(customer_username) - 1, 0) <= 0) {
        printf("Error receiving customer username from client.\n");
        return;
    }
    customer_username[strcspn(customer_username, "\n")] = '\0';  

    snprintf(buffer, sizeof(buffer), "Enter the employee username to assign the loan to: ");
    send(client->client_socket, buffer, strlen(buffer), 0);

    if (recv(client->client_socket, employee_username, sizeof(employee_username) - 1, 0) <= 0) {
        printf("Error receiving employee username from client.\n");
        return;
    }
    employee_username[strcspn(employee_username, "\n")] = '\0'; 

    FILE *loans_file = fopen(LOANS, "r");
    FILE *temp_loans_file = fopen(temp_file, "w");
    if (loans_file == NULL || temp_loans_file == NULL) {
        send(client->client_socket, "Error opening loans file.\n", 27, 0);
        return;
    }

    char file_customername[MAX_USERNAME];
    int acc_no, amount;
    char loan_purpose[BUFFER_SIZE];
    char assigned_employee[MAX_USERNAME];
    char status[BUFFER_SIZE];
    int found = 0;

    while (fscanf(loans_file, "%s %d %d %s %s %s", file_customername, &acc_no, &amount, loan_purpose, assigned_employee, status) == 6) {
        if (strcmp(file_customername, customer_username) == 0) {
            strncpy(assigned_employee, employee_username, MAX_USERNAME - 1);
            assigned_employee[MAX_USERNAME - 1] = '\0';
            found = 1;
        }
        fprintf(temp_loans_file, "%s %d %d %s %s %s\n", file_customername, acc_no, amount, loan_purpose, assigned_employee, status);
    }

    fclose(loans_file);
    fclose(temp_loans_file);

    remove(LOANS);
    rename(temp_file, LOANS);

    if (found) {
        snprintf(buffer, sizeof(buffer), "Loan assigned successfully to employee: %s for customer: %s.\n", employee_username, customer_username);
    } else {
        snprintf(buffer, sizeof(buffer), "Customer %s not found in loan applications.\n", customer_username);
    }
    send(client->client_socket, buffer, strlen(buffer), 0);
}

