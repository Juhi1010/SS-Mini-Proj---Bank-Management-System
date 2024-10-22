#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include <stdbool.h>

void initialize_login_manager();

bool is_user_logged_in(const char *username);

bool log_in_user(const char *username);

void log_out_user(const char *username);

void destroy_login_manager();

#endif // LOGIN_MANAGER_H
