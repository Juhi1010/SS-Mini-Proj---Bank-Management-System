#include "./helper-functions/login_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "./helper-functions/defines.h" 

#define MAX_USERS 100

static char logged_in_users[MAX_USERS][50];
static int user_count = 0;
static pthread_mutex_t login_mutex;

void initialize_login_manager() {
    pthread_mutex_init(&login_mutex, NULL);
}

bool is_user_logged_in(const char *username) {
    pthread_mutex_lock(&login_mutex);
    for (int i = 0; i < user_count; i++) {
        if (strcmp(logged_in_users[i], username) == 0) {
            pthread_mutex_unlock(&login_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&login_mutex);
    return false;
}

bool log_in_user(const char *username) {
    pthread_mutex_lock(&login_mutex);
    if (is_user_logged_in(username)) {
        pthread_mutex_unlock(&login_mutex);
        return false;
    }
    strcpy(logged_in_users[user_count], username);
    user_count++;
    pthread_mutex_unlock(&login_mutex);
    return true;
}

void log_out_user(const char *username) {
    pthread_mutex_lock(&login_mutex);
    for (int i = 0; i < user_count; i++) {
        if (strcmp(logged_in_users[i], username) == 0) {
            for (int j = i; j < user_count - 1; j++) {
                strcpy(logged_in_users[j], logged_in_users[j + 1]);
            }
            user_count--;
            break;
        }
    }
    pthread_mutex_unlock(&login_mutex);
}

void destroy_login_manager() {
    pthread_mutex_destroy(&login_mutex);
}
