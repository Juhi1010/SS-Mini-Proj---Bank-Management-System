#ifndef DEFINES_H
#define DEFINES_H

#include <stdbool.h>
#include "./record-struct/client.h" 

#include "./change_password.h" 
#define CUSTOMERS "./data-files/customers.txt"
#define TRANSACTIONS "./data-files/transactions.txt"
#define ACCOUNTS "./data-files/accounts.txt"
#define EMPLOYEES "./data-files/employees.txt"
#define LOANS "./data-files/loans.txt"
#define PROCESSEDLOANS "./data-files/processed_loans.txt"
#define MANAGERS "./data-files/managers.txt"
#define ADMIN "./data-files/auth.txt"
#define FEEDBACKS "./data-files/feedbacks.txt"
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define BUFFER_SIZE 1024
#define PORT 8000

#endif 