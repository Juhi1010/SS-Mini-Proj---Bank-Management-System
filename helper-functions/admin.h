#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include <stdbool.h>
#include "./record-struct/client.h" 

void get_admin_menu(char *menu, int size);

void process_admin_choice(client_info *client, int choice);

void add_new_employee(client_info *client);

void modify_employee_details(client_info *client);

void change_password_admin(client_info *client, const char *file_name);

#endif // ADMIN_H



