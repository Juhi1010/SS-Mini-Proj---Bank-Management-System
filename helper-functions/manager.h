#ifndef MANAGER_H
#define MANAGER_H

#include <stdbool.h>

#include "./record-struct/client.h" 

void get_manager_menu(char *menu, int size);
void process_manager_choice(client_info *client, int choice);
void activation(client_info *client);
void assign_loan_applications(client_info *client);

#endif // LOGIN_MANAGER_H