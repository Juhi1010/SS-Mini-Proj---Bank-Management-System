#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_ROLE 10

void display_customer_menu(int sock);
void handle_customer_operations(int sock, int choice);
void display_employee_menu(int sock);
void handle_employee_operations(int sock, int choice);
void display_manager_menu(int sock);
void handle_manager_operations(int sock, int choice);
void display_admin_menu(int sock);
void handle_admin_operations(int sock, int choice);

int main() {
    int sock;
    int authenticated = 0;
    char client_type[3];  // Adjusted to handle more input if needed
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed.\n");
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer));
    printf("%s", buffer);
    fgets(client_type, sizeof(client_type), stdin);
    client_type[strcspn(client_type, "\n")] = '\0'; 
    send(sock, client_type, strlen(client_type), 0);

    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer));
    printf("%s", buffer);
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; 
    send(sock, buffer, strlen(buffer), 0);

    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer));
    printf("%s", buffer);
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; 
    send(sock, buffer, strlen(buffer), 0);

    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer));

    if (strstr(buffer, "Authenticated.")) {
        printf("Login successful!\n");
        authenticated = 1;
    } else {
        printf("Authentication failed or you already have a session running\n");
    }

    if (authenticated) {
        if (strcmp(client_type, "1") == 0) {
            display_customer_menu(sock);
        } else if (strcmp(client_type, "2") == 0){
            display_employee_menu(sock);
        } else if (strcmp(client_type, "3") == 0) {
            display_manager_menu(sock);
        } else if (strcmp(client_type, "4") == 0) {
            display_manager_menu(sock);
        }
        else {
            printf("Client type not implemented yet.\n");
        }
    }

    close(sock);
    return 0;
}

void display_customer_menu(int sock) {

    int choice;
    char buffer[BUFFER_SIZE];

    while (1) {
        // Clear buffer and receive menu from the server
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Error receiving menu from server.\n");
            return;
        }
        printf("%s", buffer); 

        scanf("%d", &choice);
        getchar(); 

        if (send(sock, &choice, sizeof(choice), 0) <= 0) {
            printf("Error sending choice to the server.\n");
            return;
        }

        handle_customer_operations(sock, choice);

    }
}

void handle_customer_operations(int sock, int choice) {

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    char old_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];
    char confirm_password[MAX_PASSWORD];
    char loan_amount[BUFFER_SIZE];
    char loan_purpose[BUFFER_SIZE];

    switch (choice) {
        case 1: // View Account Balance
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving account balance.\n");
            } else {
                printf("%s\n", buffer);  
            }
            break;
        case 2:
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving deposit prompt.\n");
            } else {
                printf("%s", buffer);  

                int amount;
                scanf("%d", &amount);
                send(sock, &amount, sizeof(amount), 0);  
            }


            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving deposit response.\n");
            } else {
                printf("%s\n", buffer);  
            }
            break;
        case 3:
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving withdraw prompt.\n");
            } else {
                printf("%s", buffer);  

                int amount;
                scanf("%d", &amount);
                send(sock, &amount, sizeof(amount), 0);  
            }

            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving withdraw response.\n");
            } else {
                printf("%s\n", buffer);  
            }
            break; 
        case 4:
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving prompt.\n");
            } else {
                printf("%s", buffer);  

                int acc_no;
                scanf("%d", &acc_no);
                send(sock, &acc_no, sizeof(acc_no), 0);  
            }

            memset(buffer, 0, BUFFER_SIZE);
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving prompt.\n");
            } else {
                printf("%s", buffer);  

                int amount;
                scanf("%d", &amount);
                send(sock, &amount, sizeof(amount), 0);  
            }

            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving response.\n");
            } else {
                printf("%s\n", buffer);  
            }
            break;
        case 5:
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving response from server.\n");
            } else {
                printf("%s\n", buffer);
            }
            break;
        
        case 6:
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving loan amount prompt from server.\n");
                return;
            }
            printf("%s", buffer);
            
            fgets(loan_amount, sizeof(loan_amount), stdin);
            send(sock, loan_amount, strlen(loan_amount), 0);
            
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving server response.\n");
                return;
            }
            printf("%s", buffer);
            
            fgets(loan_purpose, sizeof(loan_purpose), stdin);
            send(sock, loan_purpose, strlen(loan_purpose), 0);
                
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving loan application confirmation.\n");
                return;
            }
            printf("%s", buffer);
           
            break;
        case 7:
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving feedback.\n");
                return;
            }
            printf("%s", buffer);

            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving feedback.\n");
                return;
            }
            printf("%s", buffer);
            break;
        case 8:
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving old password prompt from server.\n");
                break;
            }
            printf("%s", buffer);
            fgets(old_password, sizeof(old_password), stdin);
            old_password[strcspn(old_password, "\n")] = '\0'; 
            send(sock, old_password, strlen(old_password) + 1, 0);

            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving new password prompt from server.\n");
                break;
            }
            printf("%s", buffer);
            fgets(new_password, sizeof(new_password), stdin);
            new_password[strcspn(new_password, "\n")] = '\0'; 
            send(sock, new_password, strlen(new_password) + 1, 0);

            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving password confirmation prompt from server.\n");
                break;
            }
            printf("%s", buffer);
            fgets(confirm_password, sizeof(confirm_password), stdin);
            confirm_password[strcspn(confirm_password, "\n")] = '\0';
            send(sock, confirm_password, strlen(confirm_password) + 1, 0);

            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving result message from server.\n");
            } else {
                printf("%s", buffer);
            }
            break;
        case 9:
            memset(buffer, 0, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer), 0);
            printf("%s", buffer);
            close(sock); 
            exit(0); 
            break;

        default:
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving response from server.\n");
            } else {
                printf("%s\n", buffer);
            }
            break;
    }
}

void display_employee_menu(int sock) {
    int choice;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Error receiving menu from server.\n");
            return;
        }
        printf("%s", buffer); 

        scanf("%d", &choice);
        getchar(); 

        if (send(sock, &choice, sizeof(choice), 0) <= 0) {
            printf("Error sending choice to the server.\n");
            return;
        }

        handle_employee_operations(sock, choice);

    }
}

void handle_employee_operations(int sock, int choice) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char gender;
    int age;

    if (choice == 1) {

        for (int i = 0; i < 4; i++) {
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("Error receiving response from server.\n");
                return;
            }
            printf("%s", buffer); 

            if (i == 0) { 
                scanf("%s", username);
                username[strcspn(username, "\n")] = '\0'; 
                send(sock, username, strlen(username) + 1, 0); 
            } else if (i == 1) { 
                scanf("%s", password);
                password[strcspn(password, "\n")] = '\0'; 
                send(sock, password, strlen(password) + 1, 0); 
            } else if (i == 2) { 
                getchar(); 
                scanf("%c", &gender);
                send(sock, &gender, sizeof(gender), 0);
            } else if (i == 3) { 
                scanf("%d", &age);
                send(sock, &age, sizeof(age), 0);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
        } else {
            printf("%s\n", buffer);
        }
    }

    if (choice == 2) {
        char buffer[BUFFER_SIZE];
        char username[MAX_USERNAME];
        char gender;
        char age[2];
        int flag;

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
            return;
        }
        printf("%s", buffer); 
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0'; 
        send(sock, username, strlen(username) + 1, 0); 

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
            return;
        }
        printf("%s", buffer);
        char new_username[MAX_USERNAME];
        fgets(new_username, sizeof(new_username), stdin);
        new_username[strcspn(new_username, "\n")] = '\0'; 
        if (new_username[0] == '\0') {
            send(sock, new_username, 1, 0); 
        } else {
            send(sock, new_username, strlen(new_username) + 1, 0); 
        }

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
            return;
        }
        printf("%s", buffer); 
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; 
        if (buffer[0] == '\0') {
            gender = '\0'; 
            send(sock, &gender, sizeof(gender), 0);
        } else {
            gender = buffer[0]; 
            send(sock, &gender, sizeof(gender), 0); 
        }

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
            return;
        }
        printf("%s", buffer); 
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; 
        if (buffer[0] == '\0') {
            strcpy(buffer, "-1");
            send(sock, buffer, strlen(buffer) + 1, 0); 
        } else {
            send(sock, buffer, strlen(buffer) + 1, 0); 
        }

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
        } else {
            printf("%s\n", buffer); 
        }
    }

    else if (choice == 3) {
        char customer_username[MAX_USERNAME];
        char decision[2];

        recv(sock, buffer, sizeof(buffer) - 1, 0);
        printf("%s", buffer);
        fgets(customer_username, sizeof(customer_username), stdin);
        send(sock, customer_username, strlen(customer_username), 0);

        recv(sock, buffer, sizeof(buffer) - 1, 0);
        printf("%s", buffer);
        fgets(decision, sizeof(decision), stdin);
        send(sock, decision, strlen(decision), 0);

        recv(sock, buffer, sizeof(buffer) - 1, 0);
        printf("%s", buffer);
    }

    else if (choice == 4 ) {
        send(sock, "Requesting loan applications...\n", 32, 0);

      
        ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            perror("Error receiving loan applications");
        }

        buffer[bytes_received] = '\0';  
        printf("%s", buffer);   
        
        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving response from server.\n");
        } else {
            printf("%s\n", buffer); 
        }        
    }

    else if (choice == 5) {
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
        close(sock); 
        exit(0); 
    }

}

void display_admin_menu(int sock) {
    int choice;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Error receiving menu from server.\n");
            return;
        }
        printf("%s", buffer); 

        scanf("%d", &choice);
        getchar(); 

        if (send(sock, &choice, sizeof(choice), 0) <= 0) {
            printf("Error sending choice to the server.\n");
            return;
        }

        handle_admin_operations(sock, choice);

    }
}

void handle_admin_operations(int sock, int choice) {
   char buffer[BUFFER_SIZE];
   char input[MAX_USERNAME];
   char username[MAX_USERNAME];
   char password[MAX_USERNAME];


    if (choice == 1) { 
        char new_emp_username[MAX_USERNAME];
        char new_emp_password[MAX_PASSWORD];

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer);
        scanf("%s", new_emp_username);
        send(sock, new_emp_username, strlen(new_emp_username), 0);

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer);
        scanf("%s", new_emp_password);
        send(sock, new_emp_password, strlen(new_emp_password), 0);
    }
    
    else if (choice == 2) {
        char username[MAX_USERNAME];
        char new_username[MAX_USERNAME];
        char role[MAX_ROLE];

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer);
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';
        send(sock, username, strlen(username) + 1, 0);

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer);
        fgets(new_username, sizeof(new_username), stdin);
        new_username[strcspn(new_username, "\n")] = '\0';
        send(sock, new_username, strlen(new_username) + 1, 0);

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer);
        fgets(role, sizeof(role), stdin);
        role[strcspn(role, "\n")] = '\0';
        send(sock, role, strlen(role) + 1, 0);

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer);
    }

    else if (choice == 4) {
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
        close(sock); 
        exit(0); 
    }

    memset(buffer, 0, BUFFER_SIZE);
    if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
        printf("Error receiving menu from server.\n");
    } else {
        printf("\n%s", buffer);
    }
}


void display_manager_menu(int sock) {
    int choice;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Error receiving menu from server.\n");
            return;
        }
        printf("%s", buffer); 

        scanf("%d", &choice);
        getchar(); 

        if (send(sock, &choice, sizeof(choice), 0) <= 0) {
            printf("Error sending choice to the server.\n");
            return;
        }

        handle_manager_operations(sock, choice);

    }
}

void handle_manager_operations(int sock, int choice) {
    char buffer[BUFFER_SIZE];
    char account_number[20]; 
    char response[2]; 
    memset(buffer, 0, BUFFER_SIZE);

   if (choice == 1) { 
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);

        fgets(account_number, sizeof(account_number), stdin);
        account_number[strcspn(account_number, "\n")] = 0; 
        
        send(sock, account_number, strlen(account_number), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);

        fgets(response, sizeof(response), stdin);
        response[strcspn(response, "\n")] = 0; 

        send(sock, response, strlen(response), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);
    }
    else if (choice == 2) {
        char employee_username[MAX_USERNAME];
        char employee_password[MAX_PASSWORD];

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving customer username prompt from server.\n");
            return;
        }
        printf("%s", buffer);

        fgets(employee_username, sizeof(employee_username), stdin);
        employee_username[strcspn(employee_username, "\n")] = '\0';  
        send(sock, employee_username, strlen(employee_username), 0);

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving employee username prompt from server.\n");
            return;
        }
        printf("%s", buffer);

        fgets(employee_password, sizeof(employee_password), stdin);
        employee_password[strcspn(employee_password, "\n")] = '\0';  
        send(sock, employee_password, strlen(employee_password), 0);

        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            printf("Error receiving confirmation from server.\n");
        } else {
            printf("%s\n", buffer);
        }
        
    }
    else if (choice == 4) {
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
        close(sock); 
        exit(0); 
    }

}