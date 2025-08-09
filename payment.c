#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "payment.h"
#include "vehicle.h"
#include "validation.h"

typedef struct {
    char payment_id[20];
    char user_id[20];
    int amount;
    char datetime[25];
    char method[20];
    char vehicle_id[20];
    char slot_id[20];
} Payment;

void generate_payment_id(char *payment_id) {
    sprintf(payment_id, "P%06d", rand() % 1000000);
}

int calculate_fee(const char *vehicle_type, int duration_hours) {
    int rate_per_hour = 0;
    if (strcmp(vehicle_type, "car") == 0)
        rate_per_hour = 50;
    else if (strcmp(vehicle_type, "bike") == 0)
        rate_per_hour = 20;
    else if (strcmp(vehicle_type, "truck") == 0)
        rate_per_hour = 100;
    else
        rate_per_hour = 50;
    return rate_per_hour * duration_hours;
}

void display_payments_for_user(const char *user_id) {
    FILE *fp = fopen("data/payments.txt", "r");
    if (!fp) {
        printf("No payment records exist.\n");
        return;
    }
    char line[256];
    int found = 0;
    printf("\n--- Payment History ---\n");
    printf("ID     | Amount | Date/Time           | Method   | Vehicle   | Slot\n");
    printf("---------------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        char *token = strtok(line, "|");
        strcpy(p.payment_id, token ? token : "");
        strcpy(p.user_id, (token = strtok(NULL, "|")) ? token : "");
        p.amount = atoi((token = strtok(NULL, "|")) ? token : "0");
        strcpy(p.datetime, (token = strtok(NULL, "|")) ? token : "");
        strcpy(p.method, (token = strtok(NULL, "|")) ? token : "");
        strcpy(p.vehicle_id, (token = strtok(NULL, "|")) ? token : "");
        strcpy(p.slot_id, (token = strtok(NULL, "|\n")) ? token : "");
        if (strcmp(p.user_id, user_id) == 0) {
            printf("%-6s | %-6d | %-19s | %-8s | %-9s | %s\n",
                   p.payment_id, p.amount, p.datetime, p.method, p.vehicle_id, p.slot_id);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No payments found for this user.\n");
}

void display_all_payments() {
    FILE *fp = fopen("data/payments.txt", "r");
    if (!fp) {
        printf("No payment records exist.\n");
        return;
    }
    char line[256];
    printf("\n--- All Payment Records ---\n");
    printf("ID     | UserID | Amount | Date/Time           | Method   | Vehicle   | Slot\n");
    printf("-------------------------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        char *token = strtok(line, "|");
        strcpy(p.payment_id, token ? token : "");
        strcpy(p.user_id, (token = strtok(NULL, "|")) ? token : "");
        p.amount = atoi((token = strtok(NULL, "|")) ? token : "0");
        strcpy(p.datetime, (token = strtok(NULL, "|")) ? token : "");
        strcpy(p.method, (token = strtok(NULL, "|")) ? token : "");
        strcpy(p.vehicle_id, (token = strtok(NULL, "|")) ? token : "");
        strcpy(p.slot_id, (token = strtok(NULL, "|\n")) ? token : "");
        printf("%-6s | %-6s | %-6d | %-19s | %-8s | %-9s | %s\n",
               p.payment_id, p.user_id, p.amount, p.datetime, p.method, p.vehicle_id, p.slot_id);
    }
    fclose(fp);
}

void select_vehicle_for_payment(char *vehicle_id, const char *user_id) {
    printf("Select your vehicle for payment:\n");
    view_user_vehicles(user_id);
    printf("Enter Vehicle ID: ");
    fgets(vehicle_id, 20, stdin);
    vehicle_id[strcspn(vehicle_id, "\n")] = 0;
}

void select_slot_for_payment(char *slot_id) {
    printf("Enter Parking Slot ID: ");
    fgets(slot_id, 20, stdin);
    slot_id[strcspn(slot_id, "\n")] = 0;
}

void process_payment() {
    char user_id[20], vehicle_id[20], vehicle_type[20], slot_id[20], method[20];
    int duration, fee;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char datetime[25];

    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("\n--- Process Payment ---\n");

    printf("Enter User ID: ");
    fgets(user_id, sizeof(user_id), stdin);
    user_id[strcspn(user_id, "\n")] = 0;

    select_vehicle_for_payment(vehicle_id, user_id);

    FILE *vfp = fopen("data/vehicles.txt", "r");
    int found_vehicle = 0;
    if (vfp) {
        char line[100], v_id[20], v_user[20], v_type[20], v_plate[20];
        while (fgets(line, sizeof(line), vfp)) {
            sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^\n]", v_id, v_user, v_type, v_plate);
            if (strcmp(v_id, vehicle_id) == 0 && strcmp(v_user, user_id) == 0) {
                strcpy(vehicle_type, v_type);
                found_vehicle = 1;
                break;
            }
        }
        fclose(vfp);
    }
    if (!found_vehicle) {
        printf("Vehicle not found for this user. Payment cancelled.\n");
        return;
    }

    select_slot_for_payment(slot_id);

    printf("Parking Duration (in hours): ");
    if (scanf("%d", &duration) != 1 || duration < 1) {
        printf("Invalid duration.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        return;
    }
    int c; while ((c = getchar()) != '\n' && c != EOF) {}

    fee = calculate_fee(vehicle_type, duration);

    printf("Payment Methods: cash/card/mobile\n");
    printf("Enter Payment Method: ");
    fgets(method, sizeof(method), stdin);
    method[strcspn(method, "\n")] = 0;

    if (strcmp(method, "cash") != 0 && strcmp(method, "card") != 0 && strcmp(method, "mobile") != 0) {
        printf("Unsupported payment method.\n");
        return;
    }

    char payment_id[20];
    generate_payment_id(payment_id);

    FILE *fp = fopen("data/payments.txt", "a");
    if (!fp) {
        printf("Error: Could not open data/payments.txt for writing.\n");
        return;
    }
    fprintf(fp, "%s|%s|%d|%s|%s|%s|%s\n", payment_id, user_id, fee, datetime, method, vehicle_id, slot_id);
    fclose(fp);

    printf("\n====== Receipt ======\n");
    printf("Payment ID : %s\n", payment_id);
    printf("User ID    : %s\n", user_id);
    printf("Vehicle    : %s (%s)\n", vehicle_id, vehicle_type);
    printf("Slot       : %s\n", slot_id);
    printf("Duration   : %d hours\n", duration);
    printf("Method     : %s\n", method);
    printf("Amount     : %d BDT\n", fee);
    printf("Date/Time  : %s\n", datetime);
    printf("=====================\n");
}
