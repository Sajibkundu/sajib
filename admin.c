#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "admin.h"
#include "user.h"
#include "parking.h"
#include "payment.h"
#include "validation.h"
#include "vehicle.h"

#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "123"

typedef struct {
    char user_id[20];
    char name[100];
    char email[100];
    char phone[20];
    char nid[20];
    char password[50];
} User;

void admin_dashboard();
void view_all_users();
void search_user();
void delete_user();
void view_all_vehicles();
void view_parking_slots_admin();
void add_parking_slot_admin();
void remove_parking_slot_admin();
void view_all_payments();
void search_payment();

static char logged_in_admin[100] = "admin";

void clear_input_buffer_admin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void admin_login() {
    char username[100];
    char password[100];

    printf("\n=== Admin Login ===\n");
    printf("Username: ");
    clear_input_buffer_admin();
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    if (strcmp(username, ADMIN_USERNAME) == 0 && strcmp(password, ADMIN_PASSWORD) == 0) {
        printf("Welcome, %s!\n", username);
        strcpy(logged_in_admin, username);
        admin_dashboard();
    } else {
        printf("Invalid credentials. Try again.\n");
    }
}

void admin_dashboard() {
    int choice;
    while (1) {
        printf("\n==============================\n");
        printf("ADMIN DASHBOARD (Logged in as: %s)\n", logged_in_admin);
        printf("==============================\n");
        printf("1. View All Users\n");
        printf("2. Search User\n");
        printf("3. Delete User\n");
        printf("4. View All Vehicles\n");
        printf("5. View Parking Slots\n");
        printf("6. Add Parking Slot\n");
        printf("7. Remove Parking Slot\n");
        printf("8. View All Payments\n");
        printf("9. Search Payment\n");
        printf("10. Logout\n");
        printf("------------------------------\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clear_input_buffer_admin();
            continue;
        }
        clear_input_buffer_admin();
        switch (choice) {
            case 1: view_all_users(); break;
            case 2: search_user(); break;
            case 3: delete_user(); break;
            case 4: view_all_vehicles(); break;
            case 5: view_parking_slots_admin(); break;
            case 6: add_parking_slot_admin(); break;
            case 7: remove_parking_slot_admin(); break;
            case 8: view_all_payments(); break;
            case 9: search_payment(); break;
            case 10: printf("Admin logged out.\n"); return;
            default: printf("Invalid choice.\n");
        }
    }
}

void view_all_users() {
    FILE *fp = fopen("data/users.txt", "r");
    if (!fp) {
        printf("No user data found.\n");
        return;
    }
    char line[512];
    printf("\n--- Registered Users ---\n");
    printf("ID\tName\t\tEmail\t\tPhone\t\tNID\n");
    printf("-------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), fp)) {
        User u;
        char *token = strtok(line, "|");
        strcpy(u.user_id, token ? token : "");
        strcpy(u.name, (token = strtok(NULL, "|")) ? token : "");
        strcpy(u.email, (token = strtok(NULL, "|")) ? token : "");
        strcpy(u.phone, (token = strtok(NULL, "|")) ? token : "");
        strcpy(u.nid, (token = strtok(NULL, "|")) ? token : "");
        printf("%s\t%s\t%s\t%s\t%s\n", u.user_id, u.name, u.email, u.phone, u.nid);
    }
    fclose(fp);
}

void search_user() {
    char query[100];
    printf("Enter email, phone, or NID to search: ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0';

    FILE *fp = fopen("data/users.txt", "r");
    if (!fp) {
        printf("No user data found.\n");
        return;
    }
    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        User u;
        char *token = strtok(line, "|");
        strcpy(u.user_id, token ? token : "");
        strcpy(u.name, (token = strtok(NULL, "|")) ? token : "");
        strcpy(u.email, (token = strtok(NULL, "|")) ? token : "");
        strcpy(u.phone, (token = strtok(NULL, "|")) ? token : "");
        strcpy(u.nid, (token = strtok(NULL, "|")) ? token : "");
        if (strstr(u.email, query) || strstr(u.phone, query) || strstr(u.nid, query)) {
            printf("UserID: %s\nName: %s\nEmail: %s\nPhone: %s\nNID: %s\n", u.user_id, u.name, u.email, u.phone, u.nid);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No user found matching that query.\n");
}

void delete_user() {
    char user_id[20];
    printf("Enter User ID to delete: ");
    fgets(user_id, sizeof(user_id), stdin);
    user_id[strcspn(user_id, "\n")] = '\0';

    FILE *fp = fopen("data/users.txt", "r");
    FILE *tmp = fopen("data/users_tmp.txt", "w");
    if (!fp || !tmp) {
        printf("Error accessing user data.\n");
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return;
    }
    char line[512], cur_id[20];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        strcpy(cur_id, strtok(line, "|"));
        char *rest = strtok(NULL, "\n");
        if (strcmp(cur_id, user_id) == 0) {
            found = 1;
            continue;
        }
        fprintf(tmp, "%s|%s\n", cur_id, rest);
    }
    fclose(fp);
    fclose(tmp);
    remove("data/users.txt");
    rename("data/users_tmp.txt", "data/users.txt");
    if (found) {
        printf("User deleted.\n");
    } else {
        printf("User not found.\n");
    }
}

void view_all_vehicles() {
    FILE *fp = fopen("data/vehicles.txt", "r");
    if (!fp) {
        printf("No vehicle data found.\n");
        return;
    }
    printf("\n--- Registered Vehicles ---\n");
    printf("VehicleID\tUserID\tType\tPlate\n");
    printf("---------------------------------------------\n");
    char line[100], vehicle_id[20], user_id[20], type[20], plate[20];
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^\n]", vehicle_id, user_id, type, plate);
        printf("%s\t%s\t%s\t%s\n", vehicle_id, user_id, type, plate);
    }
    fclose(fp);
}

void view_parking_slots_admin() {
    display_parking_slots();
}

void add_parking_slot_admin() {
    int slot_num;
    char vehicle_type[20];
    printf("Enter new Slot Number: ");
    scanf("%d", &slot_num);
    clear_input_buffer_admin();
    printf("Enter vehicle type for this slot (car/bike/truck): ");
    fgets(vehicle_type, sizeof(vehicle_type), stdin);
    vehicle_type[strcspn(vehicle_type, "\n")] = 0;

    FILE *fp = fopen("data/parking_slots.txt", "a");
    if (!fp) {
        printf("Error opening parking_slots.txt for append.\n");
        return;
    }
    fprintf(fp, "%d|0|-|%s|-\n", slot_num, vehicle_type);
    fclose(fp);
    printf("Slot %d added successfully for %s.\n", slot_num, vehicle_type);
}

void remove_parking_slot_admin() {
    int slot_num, found = 0;
    printf("Enter Slot Number to remove: ");
    scanf("%d", &slot_num);
    clear_input_buffer_admin();

    FILE *fp = fopen("data/parking_slots.txt", "r");
    FILE *tmp = fopen("data/parking_slots_tmp.txt", "w");
    if (!fp || !tmp) {
        printf("Error accessing parking slots.\n");
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return;
    }
    char line[200];
    int cur_slot;
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d|", &cur_slot);
        if (cur_slot == slot_num) {
            found = 1;
            continue;
        }
        fputs(line, tmp);
    }
    fclose(fp);
    fclose(tmp);
    remove("data/parking_slots.txt");
    rename("data/parking_slots_tmp.txt", "data/parking_slots.txt");
    if (found) {
        printf("Slot %d removed.\n", slot_num);
    } else {
        printf("Slot not found.\n");
    }
}

void view_all_payments() {
    display_all_payments();
}

void search_payment() {
    char query[100];
    printf("Enter User ID or Vehicle ID to search payment: ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0';

    FILE *fp = fopen("data/payments.txt", "r");
    if (!fp) {
        printf("No payment data found.\n");
        return;
    }
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char payment_id[20], user_id[20], amount[20], datetime[32], method[20], vehicle_id[20], slot_id[20];
        char *token = strtok(line, "|");
        strcpy(payment_id, token ? token : "");
        strcpy(user_id, (token = strtok(NULL, "|")) ? token : "");
        strcpy(amount, (token = strtok(NULL, "|")) ? token : "");
        strcpy(datetime, (token = strtok(NULL, "|")) ? token : "");
        strcpy(method, (token = strtok(NULL, "|")) ? token : "");
        strcpy(vehicle_id, (token = strtok(NULL, "|")) ? token : "");
        strcpy(slot_id, (token = strtok(NULL, "|\n")) ? token : "");
        if (strcmp(user_id, query) == 0 || strcmp(vehicle_id, query) == 0) {
            printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n", payment_id, user_id, amount, datetime, method, vehicle_id, slot_id);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No payment found matching that query.\n");
}
