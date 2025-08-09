#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "user.h"
#include "admin.h"

void display_main_menu();
void show_ascii_welcome();

int main() {
    int choice;
    srand((unsigned int)time(NULL));

    show_ascii_welcome();

    while (1) {
        display_main_menu();
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        switch (choice) {
            case 1: user_login(); break;
            case 2: admin_login(); break;
            case 3: user_register(); break;
            case 4: printf("Thank you for using the Parking System.\n"); exit(0);
            default: printf("Invalid choice. Try again.\n");
        }
    }
    return 0;
}

void display_main_menu() {
    printf("\n=======================================\n");
    printf(" SMART PARKING LOT MANAGEMENT SYSTEM\n");
    printf("=======================================\n");
    printf("1. Login as User\n");
    printf("2. Login as Admin\n");
    printf("3. Register as New User\n");
    printf("4. Exit\n");
}

// Your custom ASCII welcome!
void show_ascii_welcome() {
    printf("░██╗░░░░░░░██╗███████╗██╗░░░░░░█████╗░░█████╗░███╗░░░███╗███████╗  ████████╗░█████╗░\n");
    printf("░██║░░██╗░░██║██╔════╝██║░░░░░██╔══██╗██╔══██╗████╗░████║██╔════╝  ╚══██╔══╝██╔══██╗\n");
    printf("░╚██╗████╗██╔╝█████╗░░██║░░░░░██║░░╚═╝██║░░██║██╔████╔██║█████╗░░  ░░░██║░░░██║░░██║\n");
    printf("░░████╔═████║░██╔══╝░░██║░░░░░██║░░██╗██║░░██║██║╚██╔╝██║██╔══╝░░  ░░░██║░░░██║░░██║\n");
    printf("░░╚██╔╝░╚██╔╝░███████╗███████╗╚█████╔╝╚█████╔╝██║░╚═╝░██║███████╗  ░░░██║░░░╚█████╔╝\n");
    printf("░░░╚═╝░░░╚═╝░░╚══════╝╚══════╝░╚════╝░░╚════╝░╚═╝░░░░░╚═╝╚══════╝  ░░░╚═╝░░░░╚════╝░\n\n");

    printf("░██████╗███╗░░░███╗░█████╗░██████╗░████████╗  ██████╗░░█████╗░██████╗░██╗░░██╗██╗███╗░░██╗░██████╗░\n");
    printf("██╔════╝████╗░████║██╔══██╗██╔══██╗╚══██╔══╝  ██╔══██╗██╔══██╗██╔══██╗██║░██╔╝██║████╗░██║██╔════╝░\n");
    printf("╚█████╗░██╔████╔██║███████║██████╔╝░░░██║░░░  ██████╔╝███████║██████╔╝█████═╝░██║██╔██╗██║██║░░██╗░\n");
    printf("░╚═══██╗██║╚██╔╝██║██╔══██║██╔══██╗░░░██║░░░  ██╔═══╝░██╔══██║██╔══██╗██╔═██╗░██║██║╚████║██║░░╚██╗\n");
    printf("██████╔╝██║░╚═╝░██║██║░░██║██║░░██║░░░██║░░░  ██║░░░░░██║░░██║██║░░██║██║░╚██╗██║██║░╚███║╚██████╔╝\n");
    printf("╚═════╝░╚═╝░░░░░╚═╝╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░  ╚═╝░░░░░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░╚═╝╚═╝╚═╝░░╚══╝░╚═════╝░\n");
}
