#ifndef CHANGE_PASSWORD_H
#define CHANGE_PASSWORD_H

#include <stdbool.h>
#include "./record-struct/client.h" 

void change_password(client_info *client, const char *file_name);
void send_prompt(client_info *client, const char *prompt);

#endif // LOGIN_MANAGER_H
