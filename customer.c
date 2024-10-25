#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "./helper-functions/customer.h"
#include "./helper-functions/change_password.h"
#include "./helper-functions/defines.h" 


void get_customer_menu(char *menu, int size) {
    snprintf(menu, size, "Customer Menu:\n1. View Account Balance\n2. Deposit money\n3. Withdraw money\n4. Transfer money to another bank account\n5. View Transaction history \n6. Apply for a loan\n7. Add feedback\n8. Change password\n9. Exit\nEnter your choice: ");
}

void process_customer_choice(client_info *client, int choice) {
    switch (choice) {
        case 1:
            printf("Control here\n");
            view_account_balance(client);
            break;
        case 2:
            deposit_money(client);
            break;
        case 3:
            withdraw_money(client);
            break;
        case 4:
            transfer_funds(client);
            break;
        case 5:
            view_transaction_history(client);
            break;
        case 6:
            loan_application(client);
            break;
        case 7: 
            add_feedback(client);
            break;
        case 8:
            change_password(client, CUSTOMERS);
            break;
        case 9: 
            break;
        default:
            send(client->client_socket, "Invalid choice!\n", 16, 0);
            break;
    }
}

void view_account_balance(client_info *client) {
   
    FILE *customer_file = fopen(CUSTOMERS, "r");
    if (customer_file == NULL) {
        send(client->client_socket, "Error opening customer file.\n", 29, 0);
        return;
    }

    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char gender;
    int age, account_number;
    int found = 0;

    while (fscanf(customer_file, "%s %s %c %d %d", username, password, &gender, &age, &account_number) != EOF) {
        if (strcmp(username, client->logged_in_user) == 0) {
            found = 1;
            break;
        }
    }

    fclose(customer_file);

    if (!found) {
        send(client->client_socket, "User not found in customer records.\n", 36, 0);
        return;
    }

    FILE *accounts_file = fopen(ACCOUNTS, "r");
    printf("lets check if control comes here\n");
    if (accounts_file == NULL) {
        send(client->client_socket, "Error opening accounts file.\n", 29, 0);
        return;
    }

    int acc_no;
    int isActive;
    int balance;
    found = 0;

    while (fscanf(accounts_file, "%d %d %d", &acc_no, &isActive, &balance) != EOF) {
        printf("checking accounts\n");
        if (acc_no == account_number && isActive == 1) {
            printf("Yay matched acc\n");
            found = 1;
            break;
        }
    }

    fclose(accounts_file);

    if (found) {
        char balance_msg[BUFFER_SIZE];
        snprintf(balance_msg, sizeof(balance_msg), "Your account balance is: %d\n", balance);
        send(client->client_socket, balance_msg, strlen(balance_msg), 0);
    } else {
        send(client->client_socket, "Account not found or inactive.\n", 31, 0);
    }

}

void deposit_money(client_info *client) {
    char buffer[BUFFER_SIZE];
    int deposit_amount;

    snprintf(buffer, sizeof(buffer), "Enter the amount to deposit: ");
    send(client->client_socket, buffer, strlen(buffer), 0);

    // Receive the deposit amount from the client
    if (recv(client->client_socket, &deposit_amount, sizeof(deposit_amount), 0) <= 0) {
        printf("Error receiving deposit amount from client.\n");
        return;
    }

    int old_balance, new_balance, account_number;
    if (get_account_details(client->logged_in_user, &account_number, &old_balance) == -1) {
        send(client->client_socket, "Error retrieving account details.\n", 35, 0);
        return;
    }

    new_balance = old_balance + deposit_amount;
    if (update_account_balance(account_number, new_balance) == -1) {
        send(client->client_socket, "Error updating account balance.\n", 32, 0);
        return;
    }

    record_transaction(account_number, 0, old_balance, new_balance);

    snprintf(buffer, sizeof(buffer), "Deposit successful. New balance: %d\n", new_balance);
    send(client->client_socket, buffer, strlen(buffer), 0);

}

int get_account_details(const char *username, int *account_number, int *balance) {

    FILE *customer_file = fopen(CUSTOMERS, "r");
    if (customer_file == NULL) {
        printf("Error opening customer file.\n");
        return 0;
    }

    char file_username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char gender;
    int age;
    int acc_no;
    int found = 0;

    while (fscanf(customer_file, "%s %s %c %d %d", file_username, password, &gender, &age, &acc_no) != EOF) {
        if (strcmp(file_username, username) == 0) {
            *account_number = acc_no;
            found = 1;
            break;
        }
    }
    fclose(customer_file);

    if (!found) {
        printf("User not found in customer records.\n");
        return 0;
    }

    FILE *accounts_file = fopen(ACCOUNTS, "r");
    if (accounts_file == NULL) {
        printf("Error opening accounts file.\n");
        return 0;
    }

    int acc_no_in_file;
    int isActive;
    int acc_balance;
    found = 0;

    while (fscanf(accounts_file, "%d %d %d", &acc_no_in_file, &isActive, &acc_balance) != EOF) {
        if (acc_no_in_file == *account_number && isActive == 1) {
            *balance = acc_balance;
            found = 1;
            break;
        }
    }
    fclose(accounts_file);

    if (!found) {
        printf("Account not found or inactive.\n");
        return 0;
    }

    return 1;
}

int update_account_balance(int account_number, int new_balance) {

    FILE *accounts_file = fopen(ACCOUNTS, "r");
    FILE *temp_file = fopen("temp_accounts.txt", "w");
    int acc_no, isActive, balance;
    int found = 0;

    int fd1 = fileno(temp_file);


    // Lock while updating balance 

    if (lock_file(temp_file) < 0) {
        perror("Failed to lock accounts file");
        fclose(temp_file);
        return 0;
    }

    if (accounts_file == NULL || temp_file == NULL) {
        printf("Error opening accounts file.\n");
        return 0;
    }

    while (fscanf(accounts_file, "%d %d %d", &acc_no, &isActive, &balance) != EOF) {
        if (acc_no == account_number && isActive == 1) {
            fprintf(temp_file, "%d %d %d\n", acc_no, isActive, new_balance);
            found = 1;
        } else {
            fprintf(temp_file, "%d %d %d\n", acc_no, isActive, balance);
        }
    }

    unlock_file(temp_file); // unlock after updating balance
    
    fclose(accounts_file);
    fclose(temp_file);

    if (found) {
        remove(ACCOUNTS);
        rename("temp_accounts.txt", ACCOUNTS);
    } else {
        printf("Account not found or is inactive.\n");
        remove("temp_accounts.txt");
        return 0;
    }

    return 1;

}

void record_transaction(int acc_no, int operation, int old_balance, int new_balance) {
    FILE *transactions_file = fopen(TRANSACTIONS, "a");
    if (transactions_file == NULL) {
        printf("Error opening transactions file.\n");
        return;
    }

    int transID;

    fseek(transactions_file, 0, SEEK_END);
    transID = ftell(transactions_file) / sizeof(int) + 1;

    fprintf(transactions_file, "%d %d %d %d %d\n", transID, acc_no, operation, old_balance, new_balance);
    fclose(transactions_file);

}

void withdraw_money(client_info *client) {

    char buffer[BUFFER_SIZE];
    int withdraw_amount;

    snprintf(buffer, sizeof(buffer), "Enter the amount to withdraw: ");
    send(client->client_socket, buffer, strlen(buffer), 0);

    if (recv(client->client_socket, &withdraw_amount, sizeof(withdraw_amount), 0) <= 0) {
        printf("Error receiving withdrawal amount from client.\n");
        return;
    }

    if (withdraw_amount <= 0) {
        send(client->client_socket, "Invalid withdrawal amount.\n", 27, 0);
        return;
    }

    int old_balance, new_balance, account_number;
    if (get_account_details(client->logged_in_user, &account_number, &old_balance) == 0) {
        send(client->client_socket, "Error retrieving account details.\n", 35, 0);
        return;
    }

    if (withdraw_amount > old_balance) {
        send(client->client_socket, "Insufficient funds.\n", 21, 0);
        return;
    }

    new_balance = old_balance - withdraw_amount;

    if (update_account_balance(account_number, new_balance) == 0) {
        send(client->client_socket, "Error updating account balance.\n", 32, 0);
        return;
    }

    record_transaction(account_number, 1, old_balance, new_balance);

    snprintf(buffer, sizeof(buffer), "Withdrawal successful. New balance: %d\n", new_balance);
    send(client->client_socket, buffer, strlen(buffer), 0);

}

void transfer_funds(client_info *client) {
    char buffer[BUFFER_SIZE];
    int target_account_number, transfer_amount;
    int source_account_number, source_balance, target_balance;

    if (get_account_details(client->logged_in_user, &source_account_number, &source_balance) == 0) {
        send(client->client_socket, "Error retrieving account details.\n", 35, 0);
        return;
    }

    snprintf(buffer, sizeof(buffer), "Enter the account number to transfer funds to: ");
    send(client->client_socket, buffer, strlen(buffer), 0);

    if (recv(client->client_socket, &target_account_number, sizeof(target_account_number), 0) <= 0) {
        printf("Error receiving target account number from client.\n");
        return;
    }

    if (!get_account_details_by_number(target_account_number, &target_balance)) {
        send(client->client_socket, "Target account not found or inactive.\n", 38, 0);
        return;
    }

    snprintf(buffer, sizeof(buffer), "Enter the amount to transfer: ");
    send(client->client_socket, buffer, strlen(buffer), 0);

    if (recv(client->client_socket, &transfer_amount, sizeof(transfer_amount), 0) <= 0) {
        printf("Error receiving transfer amount from client.\n");
        return;
    }

    if (transfer_amount <= 0 || transfer_amount > source_balance) {
        send(client->client_socket, "Invalid transfer amount or insufficient funds.\n", 47, 0);
        return;
    }

    int new_source_balance = source_balance - transfer_amount;
    int new_target_balance = target_balance + transfer_amount;


    if (update_account_balance(source_account_number, new_source_balance) == 0) {
        send(client->client_socket, "Error updating source account balance.\n", 40, 0);
        return;
    }

    // Update the target account balance
    if (update_account_balance(target_account_number, new_target_balance) == 0) {
        send(client->client_socket, "Error updating target account balance.\n", 40, 0);
        // Rollback source account balance in case of failure (to ensure Consistency)
        update_account_balance(source_account_number, source_balance);
        return;
    }


    record_transaction(source_account_number, 2, source_balance, new_source_balance);
    record_transaction(target_account_number, 2, target_balance, new_target_balance);

    snprintf(buffer, sizeof(buffer), "Transfer successful. New balance: %d\n", new_source_balance);
    send(client->client_socket, buffer, strlen(buffer), 0);

}

int get_account_details_by_number(int account_number, int *balance) {

    FILE *accounts_file = fopen(ACCOUNTS, "r");
    if (accounts_file == NULL) {
        printf("Error opening accounts file.\n");
        return 0;
    }

    int acc_no, isActive, acc_balance;
    int found = 0;

    // Search for the account and retrieve the balance if active
    while (fscanf(accounts_file, "%d %d %d", &acc_no, &isActive, &acc_balance) != EOF) {
        if (acc_no == account_number && isActive == 1) {
            *balance = acc_balance;
            found = 1;
            break;
        }
    }
    fclose(accounts_file);

    return found;

}

void view_transaction_history(client_info *client) {

    char buffer[BUFFER_SIZE];

    int account_number, balance;
    if (get_account_details(client->logged_in_user, &account_number, &balance) == 0) {
        send(client->client_socket, "Error retrieving account details.\n", 35, 0);
        return;
    }

    FILE *transactions_file = fopen(TRANSACTIONS, "r");
    if (transactions_file == NULL) {
        send(client->client_socket, "Error opening transactions file.\n", 34, 0);
        return;
    }

    int transID, trans_acc_no, operation, old_balance, new_balance;
    long trans_time;
    int found = 0;

    char transaction_info[BUFFER_SIZE];

    send(client->client_socket, "Transaction History:\n", 21, 0);

    while (fscanf(transactions_file, "%d %d %d %d %d %ld", &transID, &trans_acc_no, &operation, &old_balance, &new_balance, &trans_time) != EOF) {
        if (trans_acc_no == account_number) {
            found = 1;
            char *operation_type = (operation == 0) ? "Deposit" : (operation == 1) ? "Withdrawal" : "Transfer";

            snprintf(transaction_info, sizeof(transaction_info),
                     "Transaction ID: %d\nOperation: %s\nOld Balance: %d\nNew Balance: %d\nTime: %s\n\n",
                     transID, operation_type, old_balance, new_balance, ctime(&trans_time));

            send(client->client_socket, transaction_info, strlen(transaction_info), 0);
        }
    }

    fclose(transactions_file);

    if (!found) {
        send(client->client_socket, "No transactions found for your account.\n", 39, 0);
    }

}

void change_password(client_info *client, const char *file_name) {
    char old_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];
    char confirm_password[MAX_PASSWORD];
    char buffer[BUFFER_SIZE];

    send_prompt(client, "Enter your old password: ");
    if (recv(client->client_socket, old_password, sizeof(old_password), 0) <= 0) {
        printf("Error receiving old password.\n");
        return;
    }

    send_prompt(client, "Enter your new password: ");
    if (recv(client->client_socket, new_password, sizeof(new_password), 0) <= 0) {
        printf("Error receiving new password.\n");
        return;
    }

    send_prompt(client, "Confirm your new password: ");
    if (recv(client->client_socket, confirm_password, sizeof(confirm_password), 0) <= 0) {
        printf("Error receiving password confirmation.\n");
        return;
    }

    if (strcmp(new_password, confirm_password) != 0) {
        send_prompt(client, "New password and confirmation do not match.\n");
        return;
    }

    FILE *file = fopen(file_name, "r+");
    if (file == NULL) {
        printf("Error opening file: %s\n", file_name);
        return;
    }

    char file_username[MAX_USERNAME];
    char file_password[MAX_PASSWORD];
    char file_gender; 
    int file_age, file_acc;
    int found = 0;

    FILE *temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) {
        printf("Error opening temporary file.\n");
        fclose(file);
        return;
    }

    while (fscanf(file, "%s %s %c %d %d", file_username, file_password, &file_gender, &file_age, &file_acc) == 5) {
        if (strcmp(file_username, client->logged_in_user) == 0) {
            if (strcmp(file_password, old_password) == 0) {
                found = 1;
                fprintf(temp_file, "%s %s %c %d %d\n", file_username, new_password, file_gender, file_age, file_acc);
            } else {
                send_prompt(client, "Old password does not match.\n");
                fclose(file);
                fclose(temp_file);
                remove("temp.txt"); 
                return;
            }
        } else {
            fprintf(temp_file, "%s %s %c %d %d\n", file_username, file_password, file_gender, file_age, file_acc);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        remove(file_name);
        rename("temp.txt", file_name);
        send_prompt(client, "Password updated successfully.\n");
    } else {
        send_prompt(client, "Username not found.\n");
        remove("temp.txt"); 
    }

}

void add_feedback(client_info *client) {

    char feedback[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];


    send_prompt(client, "Enter feedback : ");
    if (recv(client->client_socket, buffer, sizeof(buffer), 0) <= 0) {
        printf("Error receiving feedback\n");
        return;
    }

    FILE *feedback_file = fopen(FEEDBACKS, "a");
    if (feedback_file == NULL) {
        send(client->client_socket, "Error opening feedback file.\n", 29, 0);
        return;
    }

    fprintf(feedback_file, "%s %s\n", client->logged_in_user, buffer);
    fclose(feedback_file);

    send(client->client_socket, "Feedback submitted successfully.\n", 33, 0);

}

int lock_file(FILE *file) {

    int fd = fileno(file); 
    struct flock fl;

    fl.l_type = F_WRLCK; 
    fl.l_whence = SEEK_SET;
    fl.l_start = 0; // Start of the file
    fl.l_len = 0; // Lock the entire file

    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("Failed to lock accounts file");
        return -1;
    }
    return 0;
}

int unlock_file(FILE *file) {
    int fd = fileno(file); 
    struct flock fl;

    fl.l_type = F_UNLCK; 
    fl.l_whence = SEEK_SET;
    fl.l_start = 0; // Start of the file
    fl.l_len = 0; // Unlock the entire file

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("Failed to unlock accounts file");
        return -1;
    }
    return 0;
}

// void loan_application(client_info *client) {
//     char buffer[BUFFER_SIZE];
//     int loan_amount;
//     char loan_purpose[BUFFER_SIZE];
    
//     recv(client->client_socket, buffer, sizeof(buffer), MSG_DONTWAIT);
    
//     snprintf(buffer, sizeof(buffer), "Enter the loan amount: ");
//     send(client->client_socket, buffer, strlen(buffer), 0);
    
//     memset(buffer, 0, sizeof(buffer));
//     if (recv(client->client_socket, buffer, sizeof(buffer), 0) <= 0) {
//         printf("Error receiving loan amount from client.\n");
//         return;
//     }
//     loan_amount = atoi(buffer);

//     if (loan_amount <= 0) {
//         send(client->client_socket, "Invalid loan amount.\n", 21, 0);
//         return;
//     }
    
//     usleep(100000);  
    
//     snprintf(buffer, sizeof(buffer), "Enter the purpose of the loan: ");
//     send(client->client_socket, buffer, strlen(buffer), 0);
    
//     memset(buffer, 0, sizeof(buffer));
//     if (recv(client->client_socket, buffer, sizeof(buffer), 0) <= 0) {
//         printf("Error receiving loan purpose from client.\n");
//         return;
//     }
//     strncpy(loan_purpose, buffer, sizeof(loan_purpose) - 1);
//     loan_purpose[sizeof(loan_purpose) - 1] = '\0';
    
//     FILE *loans_file = fopen(LOANS, "a");
//     if (loans_file == NULL) {
//         send(client->client_socket, "Error opening loans file.\n", 27, 0);
//         return;
//     }
    
//     int account_number, balance;
//     if (get_account_details(client->logged_in_user, &account_number, &balance) == 0) {
//         send(client->client_socket, "Error retrieving account details.\n", 35, 0);
//         fclose(loans_file);
//         return;
//     }
    
//     fprintf(loans_file, "%s %d %d %s %s\n", client->logged_in_user, account_number, loan_amount, loan_purpose, "Reviewing");
//     fclose(loans_file);
    
//     snprintf(buffer, sizeof(buffer), "Loan application submitted successfully for %d.\n", loan_amount);
//     send(client->client_socket, buffer, strlen(buffer), 0);
// }

void loan_application(client_info *client) {
    char buffer[BUFFER_SIZE];
    int loan_amount;
    char loan_purpose[BUFFER_SIZE];
    
    recv(client->client_socket, buffer, sizeof(buffer), MSG_DONTWAIT);
    
    snprintf(buffer, sizeof(buffer), "Enter the loan amount: ");
    send(client->client_socket, buffer, strlen(buffer), 0);
    
    memset(buffer, 0, sizeof(buffer));
    if (recv(client->client_socket, buffer, sizeof(buffer), 0) <= 0) {
        printf("Error receiving loan amount from client.\n");
        return;
    }
    loan_amount = atoi(buffer);

    if (loan_amount <= 0) {
        send(client->client_socket, "Invalid loan amount.\n", 21, 0);
        return;
    }
    
    usleep(100000);  
    
    snprintf(buffer, sizeof(buffer), "Enter the purpose of the loan: ");
    send(client->client_socket, buffer, strlen(buffer), 0);
    
    memset(buffer, 0, sizeof(buffer));
    if (recv(client->client_socket, buffer, sizeof(buffer), 0) <= 0) {
        printf("Error receiving loan purpose from client.\n");
        return;
    }

    strncpy(loan_purpose, buffer, sizeof(loan_purpose) - 1);
    loan_purpose[sizeof(loan_purpose) - 1] = '\0';
    loan_purpose[strcspn(loan_purpose, "\n")] = '\0';  

    FILE *loans_file = fopen(LOANS, "a");
    if (loans_file == NULL) {
        send(client->client_socket, "Error opening loans file.\n", 27, 0);
        return;
    }
    
    int account_number, balance;
    if (get_account_details(client->logged_in_user, &account_number, &balance) == 0) {
        send(client->client_socket, "Error retrieving account details.\n", 35, 0);
        fclose(loans_file);
        return;
    }
    
    fprintf(loans_file, "%s %d %d %s %s %s\n", client->logged_in_user, account_number, loan_amount, loan_purpose, "Unassigned", "Reviewing");
    fclose(loans_file);
    
    snprintf(buffer, sizeof(buffer), "Loan application submitted successfully for %d.\n", loan_amount);
    send(client->client_socket, buffer, strlen(buffer), 0);
}




