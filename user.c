#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "vehicle.h"
#include "parking.h"
#include "payment.h"
#include "validation.h"

typedef struct {
    char user_id[20];
    char name[100];
    char email[100];
    char phone[20];
    char nid[20];
    char password[50];
} User;

static User logged_in_user;

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void user_dashboard();
void view_profile();
void update_profile();
void change_password();
void user_payments();
void user_vehicle_menu();
void user_parking_menu();

void user_register() {
    char name[100], email[100], phone[20], nid[20], password[50], confirm_password[50];
    FILE *fp = fopen("data/users.txt", "a");
    if (!fp) {
        printf("Error: Unable to open users.txt for writing.\n");
        return;
    }

    printf("\n--- User Registration ---\n");

    printf("Enter name: ");
    clear_input_buffer();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    do {
        printf("Enter email: ");
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = '\0';
        if (!validate_email(email)) {
            printf("Invalid email format. Try again.\n");
        }
    } while (!validate_email(email));

    do {
        printf("Enter phone (11 digits, starts with 01): ");
        fgets(phone, sizeof(phone), stdin);
        phone[strcspn(phone, "\n")] = '\0';
        if (!validate_phone(phone)) {
            printf("Invalid phone number. Try again.\n");
        }
    } while (!validate_phone(phone));

    do {
        printf("Enter NID (10 or 17 digits): ");
        fgets(nid, sizeof(nid), stdin);
        nid[strcspn(nid, "\n")] = '\0';
        if (!validate_nid(nid)) {
            printf("Invalid NID. Try again.\n");
        }
    } while (!validate_nid(nid));

    do {
        printf("Enter password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0';

        printf("Confirm password: ");
        fgets(confirm_password, sizeof(confirm_password), stdin);
        confirm_password[strcspn(confirm_password, "\n")] = '\0';

        if (strcmp(password, confirm_password) != 0) {
            printf("Passwords do not match. Try again.\n");
        }
    } while (strcmp(password, confirm_password) != 0);

    int user_id = rand() % 100000;

    fprintf(fp, "%d|%s|%s|%s|%s|%s\n", user_id, name, email, phone, nid, password);
    fclose(fp);
    printf("Registration successful! You can now log in.\n");
}

void user_login() {
    char email[100], password[50], line[512];
    int found = 0;

    FILE *fp = fopen("data/users.txt", "r");
    if (!fp) {
        printf("Error: Unable to open users.txt for reading.\n");
        return;
    }

    printf("\n--- User Login ---\n");

    printf("Enter email: ");
    clear_input_buffer();
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = '\0';

    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        char *token;
        char temp_user_id[20], temp_name[100], temp_email[100], temp_phone[20], temp_nid[20], temp_password[50];
        strcpy(temp_user_id, strtok(line, "|"));
        strcpy(temp_name, strtok(NULL, "|"));
        strcpy(temp_email, strtok(NULL, "|"));
        strcpy(temp_phone, strtok(NULL, "|"));
        strcpy(temp_nid, strtok(NULL, "|"));
        strcpy(temp_password, strtok(NULL, "|\n"));

        if (strcmp(email, temp_email) == 0 && strcmp(password, temp_password) == 0) {
            found = 1;
            strcpy(logged_in_user.user_id, temp_user_id);
            strcpy(logged_in_user.name, temp_name);
            strcpy(logged_in_user.email, temp_email);
            strcpy(logged_in_user.phone, temp_phone);
            strcpy(logged_in_user.nid, temp_nid);
            strcpy(logged_in_user.password, temp_password);

            printf("\nWelcome, %s! (UserID: %s)\n", temp_name, temp_user_id);
            user_dashboard();
            break;
        }
    }
    fclose(fp);

    if (!found) {
        printf("Login failed. Invalid email or password.\n");
    }
}

void user_dashboard() {
    int choice;
    while (1) {
        printf("\n=====================================\n");
        printf("User Dashboard (User: %s)\n", logged_in_user.name);
        printf("=====================================\n");
        printf("1. View Profile\n");
        printf("2. Update Profile\n");
        printf("3. Change Password\n");
        printf("4. My Vehicles\n");
        printf("5. Parking Management\n");
        printf("6. My Payments\n");
        printf("7. Logout\n");
        printf("-------------------------------------\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Try again.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        switch (choice) {
            case 1: view_profile(); break;
            case 2: update_profile(); break;
            case 3: change_password(); break;
            case 4: user_vehicle_menu(); break;
            case 5: user_parking_menu(); break;
            case 6: user_payments(); break;
            case 7: printf("Logout successful.\n"); return;
            default: printf("Invalid choice. Try again.\n");
        }
    }
}

void view_profile() {
    printf("\n--- My Profile ---\n");
    printf("User ID: %s\n", logged_in_user.user_id);
    printf("Name   : %s\n", logged_in_user.name);
    printf("Email  : %s\n", logged_in_user.email);
    printf("Phone  : %s\n", logged_in_user.phone);
    printf("NID    : %s\n", logged_in_user.nid);
}

void update_profile() {
    char new_name[100], new_phone[20];
    printf("\n--- Update Profile ---\n");
    printf("Enter new name (or press Enter to keep current: %s): ", logged_in_user.name);
    fgets(new_name, sizeof(new_name), stdin);
    new_name[strcspn(new_name, "\n")] = '\0';
    if (strlen(new_name) > 0) strcpy(logged_in_user.name, new_name);

    do {
        printf("Enter new phone (or press Enter to keep current: %s): ", logged_in_user.phone);
        fgets(new_phone, sizeof(new_phone), stdin);
        new_phone[strcspn(new_phone, "\n")] = '\0';
        if (strlen(new_phone) == 0) break;
        if (!validate_phone(new_phone)) {
            printf("Invalid phone number. Try again.\n");
        } else {
            strcpy(logged_in_user.phone, new_phone);
            break;
        }
    } while (1);

    FILE *fp = fopen("data/users.txt", "r");
    FILE *tmp = fopen("data/users_tmp.txt", "w");
    if (!fp || !tmp) {
        printf("Error updating profile.\n");
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return;
    }
    char line[512], cur_id[20];
    while (fgets(line, sizeof(line), fp)) {
        strcpy(cur_id, strtok(line, "|"));
        char *rest = strtok(NULL, "\n");
        if (strcmp(cur_id, logged_in_user.user_id) == 0) {
            fprintf(tmp, "%s|%s|%s|%s|%s|%s\n", logged_in_user.user_id, logged_in_user.name, logged_in_user.email, logged_in_user.phone, logged_in_user.nid, logged_in_user.password);
        } else {
            fprintf(tmp, "%s|%s\n", cur_id, rest);
        }
    }
    fclose(fp);
    fclose(tmp);
    remove("data/users.txt");
    rename("data/users_tmp.txt", "data/users.txt");
    printf("Profile updated successfully.\n");
}

void change_password() {
    char old_pass[50], new_pass[50], confirm_pass[50];
    printf("\n--- Change Password ---\n");
    printf("Enter current password: ");
    fgets(old_pass, sizeof(old_pass), stdin);
    old_pass[strcspn(old_pass, "\n")] = '\0';

    if (strcmp(old_pass, logged_in_user.password) != 0) {
        printf("Incorrect current password.\n");
        return;
    }

    do {
        printf("Enter new password: ");
        fgets(new_pass, sizeof(new_pass), stdin);
        new_pass[strcspn(new_pass, "\n")] = '\0';
        printf("Confirm new password: ");
        fgets(confirm_pass, sizeof(confirm_pass), stdin);
        confirm_pass[strcspn(confirm_pass, "\n")] = '\0';
        if (strcmp(new_pass, confirm_pass) != 0) {
            printf("Passwords do not match. Try again.\n");
        }
    } while (strcmp(new_pass, confirm_pass) != 0);

    strcpy(logged_in_user.password, new_pass);

    FILE *fp = fopen("data/users.txt", "r");
    FILE *tmp = fopen("data/users_tmp.txt", "w");
    if (!fp || !tmp) {
        printf("Error updating password.\n");
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return;
    }
    char line[512], cur_id[20];
    while (fgets(line, sizeof(line), fp)) {
        strcpy(cur_id, strtok(line, "|"));
        char *rest = strtok(NULL, "\n");
        if (strcmp(cur_id, logged_in_user.user_id) == 0) {
            fprintf(tmp, "%s|%s|%s|%s|%s|%s\n", logged_in_user.user_id, logged_in_user.name, logged_in_user.email, logged_in_user.phone, logged_in_user.nid, logged_in_user.password);
        } else {
            fprintf(tmp, "%s|%s\n", cur_id, rest);
        }
    }
    fclose(fp);
    fclose(tmp);
    remove("data/users.txt");
    rename("data/users_tmp.txt", "data/users.txt");
    printf("Password changed successfully.\n");
}

void user_payments() {
    display_payments_for_user(logged_in_user.user_id);
}

void user_vehicle_menu() {
    int choice;
    while (1) {
        printf("\n--- My Vehicles ---\n");
        printf("1. View My Vehicles\n");
        printf("2. Add Vehicle\n");
        printf("3. Remove Vehicle\n");
        printf("4. Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        switch (choice) {
            case 1: view_user_vehicles(logged_in_user.user_id); break;
            case 2: add_vehicle(logged_in_user.user_id); break;
            case 3: remove_vehicle(logged_in_user.user_id); break;
            case 4: return;
            default: printf("Invalid choice.\n");
        }
    }
}

void user_parking_menu() {
    int choice;
    while (1) {
        printf("\n--- Parking Management ---\n");
        printf("1. View Parking Slots\n");
        printf("2. Reserve Parking Slot\n");
        printf("3. Pay for Parking\n");
        printf("4. Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        switch (choice) {
            case 1: display_parking_slots(); break;
            case 2: reserve_parking_slot(); break;
            case 3: process_payment(); break;
            case 4: return;
            default: printf("Invalid choice.\n");
        }
    }
}
