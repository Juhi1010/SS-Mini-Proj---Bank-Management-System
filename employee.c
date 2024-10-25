#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "./helper-functions/employee.h"
#include "./helper-functions/change_password.h"
#include "./helper-functions/defines.h" 


void get_employee_menu(char *menu, int size) {
    snprintf(menu, size, "Employee Menu:\n1. Add New Customer\n2. Modify Customer Details\n3. Approve/Reject Loans\n4. Process Loan Applications\n5. Exit\nEnter your choice: ");
}

void process_employee_choice(client_info *client, int choice) {
    switch (choice) {
        case 1:
            add_new_customer(client);
            break;
        case 2:
            modify_customer_details(client);
            break;
        case 3:
            approve_reject_loans(client);
            break;
        case 4:
            process_loans(client);
            break;
        case 5:
            break;
        default:
            send(client->client_socket, "Invalid choice!\n", 16, 0);
            break;
    }
}


int generate_new_account_number() {

    FILE *file = fopen(ACCOUNTS, "r");
    if (file == NULL) {
        perror("Error opening accounts file");
        return -1;
    }

    int account_number = 0;
    int max_account_number = 0;
    int active;
    int balance;

    while (fscanf(file, "%d %d %d", &account_number, &active, &balance) != EOF) {
        if (account_number > max_account_number) {
            max_account_number = account_number;
        }
    }

    fclose(file);
    return max_account_number + 1; 

}

void add_new_customer(client_info *client) {

    int client_socket = client->client_socket;
    char username[MAX_USERNAME]={0}, password[MAX_PASSWORD]={0};
    char gender;
    int age;
    int account_number;

    send(client_socket, "Enter new customer username: ", 29, 0);
    recv(client_socket, username, sizeof(username) - 1, 0);
    username[strcspn(username, "\n")] = '\0';

    send(client_socket, "Enter password: ", 17, 0);
    recv(client_socket, password, sizeof(password) - 1, 0);
    password[strcspn(password, "\n")] = '\0';

    send(client_socket, "Enter gender (M/F): ", 20, 0);
    recv(client_socket, &gender, sizeof(gender), 0);

    send(client_socket, "Enter age: ", 11, 0);
    recv(client_socket, &age, sizeof(age), 0);

    account_number = generate_new_account_number();
    if (account_number == -1) {
        send(client_socket, "Error generating new account number.\n", 38, 0);
        return;
    }

    FILE *customers_file = fopen(CUSTOMERS, "a");
    if (customers_file == NULL) {
        perror("Error opening customers file");
        send(client_socket, "Error opening customers file.\n", 30, 0);
        return;
    }

    fprintf(customers_file, "%s %s %c %d %d\n", username, password, gender, age, account_number);
    fclose(customers_file);

    FILE *accounts_file = fopen(ACCOUNTS, "a");
    if (accounts_file == NULL) {
        perror("Error opening accounts file");
        send(client_socket, "Error opening accounts file.\n", 29, 0);
        return;
    }

    fprintf(accounts_file, "%d %d %d\n", account_number, 1, 0); 
    fclose(accounts_file);

    char success_msg[BUFFER_SIZE];
    snprintf(success_msg, sizeof(success_msg), "Customer added successfully with account number: %d\n", account_number);
    send(client_socket, success_msg, strlen(success_msg), 0);

}

void modify_customer_details(client_info *client) {

    int client_socket = client->client_socket;
    char username[MAX_USERNAME], new_username[MAX_USERNAME], gender_input;
    char buffer[BUFFER_SIZE];
    char file_username[MAX_USERNAME], file_password[MAX_PASSWORD];
    char file_gender;
    int file_age, account_number;
    int found = 0;

    send(client_socket, "Enter the username of the customer to modify: ", 45, 0);
    recv(client_socket, username, sizeof(username) - 1, 0);
    username[strcspn(username, "\n")] = '\0';

    FILE *customers_file = fopen(CUSTOMERS, "r");
    FILE *temp_file = fopen("./data-files/temp_customers.txt", "w");
    if (customers_file == NULL || temp_file == NULL) {
        perror("Error opening file");
        send(client_socket, "Error opening file.\n", 20, 0);
        return;
    }

    while (fscanf(customers_file, "%s %s %c %d %d", file_username, file_password, &file_gender, &file_age, &account_number) == 5) {
        if (strcmp(username, file_username) == 0) {
            found = 1;

            send(client_socket, "Enter the new username (or press enter to keep the same): ", 56, 0);
            recv(client_socket, new_username, sizeof(new_username) - 1, 0);
            new_username[strcspn(new_username, "\n")] = '\0';

            if (strlen(new_username) == 0) {
                strcpy(new_username, file_username);
            }

            send(client_socket, "Enter the new gender (M/F) (or press enter to keep the same): ", 61, 0);
            recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strlen(buffer) == 1 && (buffer[0] == 'M' || buffer[0] == 'F')) {
                gender_input = buffer[0];
            } else {
                gender_input = file_gender; 
            }

            char age_buffer[3];
            send(client_socket, "Enter the new age (or press enter to keep the same): ", 52, 0);
            recv(client_socket, age_buffer, sizeof(age_buffer), 0);
            int age = atoi(age_buffer);
            if (age == -1) {
               age = file_age; 
            }

            fprintf(temp_file, "%s %s %c %d %d\n", new_username, file_password, gender_input, age, account_number);
            send(client_socket, "Customer details updated successfully.\n", 39, 0);
        } else {
            fprintf(temp_file, "%s %s %c %d %d\n", file_username, file_password, file_gender, file_age, account_number);
        }
    }

    fclose(customers_file);
    fclose(temp_file);

    if (found) {
        remove(CUSTOMERS);
        rename("./data-files/temp_customers.txt", CUSTOMERS);
    } else {
        send(client_socket, "Customer not found.\n", 20, 0);
        remove("./data-files/temp_customers.txt");
    }

}

void process_loans(client_info *client) {
    int client_socket = client->client_socket;
    char employee_username[MAX_USERNAME];
    char loan_customer[MAX_USERNAME];
    char assigned_employee[MAX_USERNAME];
    char acc_no[5];
    char loan_amount[10];
    char loan_type[50];
    char loan_status[10];
    char buffer[BUFFER_SIZE];
    int found = 0;

    snprintf(employee_username, sizeof(employee_username), "%s", client->logged_in_user);

    FILE *loans_file = fopen(LOANS, "r");
    if (loans_file == NULL) {
        perror("Error opening loans.txt");
        send(client_socket, "Error accessing loan applications.\n", 36, 0);
        return;
    }

    send(client_socket, "Loan applications assigned to you:\n", 35, 0);

    while (fscanf(loans_file, "%s %s %s %49s %49s %9s", loan_customer, acc_no, loan_amount, loan_type, assigned_employee, loan_status) != EOF) {
        if (strcmp(assigned_employee, employee_username) == 0) {
            snprintf(buffer, sizeof(buffer), "Customer: %s, Acc_No: %s, Amount: %s, Purpose: %s, Status: %s\n", 
                     loan_customer, acc_no, loan_amount, loan_type, loan_status);
            send(client_socket, buffer, strlen(buffer), 0);
            found = 1;
        }
    }

    fclose(loans_file);

    if (!found) {
        send(client_socket, "No loan applications assigned to you.\n", 38, 0);
    } else {
        send(client_socket, "End of loan applications.\n", 27, 0);
    }
}

void approve_reject_loans(client_info *client) {
    int client_socket = client->client_socket;
    char customer_username[MAX_USERNAME];
    char decision[2];
    char buffer[BUFFER_SIZE];

    send(client_socket, "Enter the username of the customer whose loan is to be processed: ", 65, 0);
    recv(client_socket, customer_username, sizeof(customer_username) - 1, 0);
    customer_username[strcspn(customer_username, "\n")] = '\0';

    send(client_socket, "Enter 'Y' to approve or 'N' to reject the loan: ", 47, 0);
    recv(client_socket, decision, sizeof(decision) - 1, 0);
    decision[strcspn(decision, "\n")] = '\0';

    FILE *loans_file = fopen(LOANS, "r+"); 
    if (loans_file == NULL) {
        perror("Error opening loans.txt");
        send(client_socket, "Error accessing loans data.\n", 28, 0);
        return;
    }

    int found = 0;
    char line[BUFFER_SIZE];
    long line_pos;

    while (fgets(line, sizeof(line), loans_file)) {
        char loan_customer[MAX_USERNAME], acc_no[10], loan_amount[10], loan_type[50], employee_username[MAX_USERNAME], loan_status[10];
        
        sscanf(line, "%s %s %s %s %s %s", loan_customer, acc_no, loan_amount, loan_type, employee_username, loan_status);

        if (strcmp(loan_customer, customer_username) == 0) {
            found = 1;

            line_pos = ftell(loans_file) - strlen(line);
            fseek(loans_file, line_pos, SEEK_SET);

            if (strcmp(decision, "Y") == 0) {
                snprintf(line, sizeof(line), "%s %s %s %s %s Approved\n", loan_customer, acc_no, loan_amount, loan_type, employee_username);
                send(client_socket, "Loan approved.\n", 15, 0);
            } else if (strcmp(decision, "N") == 0) {
                snprintf(line, sizeof(line), "%s %s %s %s %s Rejected\n", loan_customer, acc_no, loan_amount, loan_type, employee_username);
                send(client_socket, "Loan rejected.\n", 16, 0);
            } else {
                send(client_socket, "Invalid decision. Please use 'Y' or 'N'.\n", 41, 0);
                fclose(loans_file);
                return;
            }

            fprintf(loans_file, "%s", line);
            fflush(loans_file);  
            break;
        }
    }

    fclose(loans_file);

    if (!found) {
        send(client_socket, "Customer loan not found.\n", 26, 0);
    }
}
