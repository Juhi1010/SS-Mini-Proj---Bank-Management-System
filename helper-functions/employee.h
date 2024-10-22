#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "./record-struct/client.h" 

void get_employee_menu(char *menu, int size);
void process_employee_choice(client_info *client, int choice);
void add_new_customer(client_info *client);
void modify_customer_details(client_info *client);
void process_loans(client_info *client);
void approve_reject_loans(client_info *client);


#endif // EMPLOYEE_H