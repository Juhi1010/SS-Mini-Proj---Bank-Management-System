#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "./record-struct/client.h" 

void get_customer_menu(char *menu, int size);
void process_customer_choice(client_info *client, int choice);
void view_account_balance(client_info *client);
void deposit_money(client_info *client);
void withdraw_money(client_info *client);
void transfer_funds(client_info *client);
void view_transaction_history(client_info *client);
void add_feedback(client_info *client);
void loan_application(client_info *client);
int get_account_details(const char *username, int *account_number, int *balance);
int get_account_details_by_number(int account_number, int *balance);
int update_account_balance(int account_number, int new_balance);
int lock_file(FILE *file);
int unlock_file(FILE *file);
void record_transaction(int acc_no, int operation, int old_balance, int new_balance);
void logout_cust(client_info *client);

#endif // CUSTOMER_H
