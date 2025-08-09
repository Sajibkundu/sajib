#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parking.h"
#include "validation.h"

// Structure for a parking slot
typedef struct {
    int slot_number;
    int is_occupied;
    char vehicle_id[20];
    char vehicle_type[20];
    char reserved_until[25];
} ParkingSlot;

void print_slot(const ParkingSlot *slot) {
    char *status;
    if (slot->is_occupied == 1)
        status = "Occupied";
    else if (slot->is_occupied == 2)
        status = "Reserved";
    else
        status = "Available";

    printf("%4d | %-10s | %-10s | %-8s | %s\n",
           slot->slot_number,
           status,
           slot->is_occupied ? slot->vehicle_id : "-",
           slot->is_occupied ? slot->vehicle_type : "-",
           slot->reserved_until);
}

void display_parking_slots() {
    FILE *fp = fopen("data/parking_slots.txt", "r");
    if (!fp) {
        printf("No parking slot data found.\n");
        return;
    }
    ParkingSlot slot;
    printf("\nSlot | Status     | VehicleID  | Type     | Reserved Until\n");
    printf("----------------------------------------------------------\n");
    while (fscanf(fp, "%d|%d|%19[^|]|%19[^|]|%24[^\n]\n",
                  &slot.slot_number, &slot.is_occupied, slot.vehicle_id,
                  slot.vehicle_type, slot.reserved_until) == 5) {
        print_slot(&slot);
    }
    fclose(fp);
}

void reserve_parking_slot() {
    int slot_num, found = 0;
    char reserved_time[25];
    printf("\n--- Reserve Parking Slot ---\n");
    printf("Enter Slot Number to reserve: ");
    if (scanf("%d", &slot_num) != 1) {
        printf("Invalid input.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        return;
    }
    int c; while ((c = getchar()) != '\n' && c != EOF) {}

    printf("Enter reservation expiry (YYYY-MM-DD HH:MM): ");
    fgets(reserved_time, sizeof(reserved_time), stdin);
    reserved_time[strcspn(reserved_time, "\n")] = 0;

    if (!validate_datetime(reserved_time)) {
        printf("Invalid date/time format.\n");
        return;
    }

    FILE *fp = fopen("data/parking_slots.txt", "r");
    FILE *temp = fopen("data/temp_slots.txt", "w");
    if (!fp || !temp) {
        printf("Error accessing parking slots file.\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    ParkingSlot slot;
    while (fscanf(fp, "%d|%d|%19[^|]|%19[^|]|%24[^\n]\n",
                  &slot.slot_number, &slot.is_occupied, slot.vehicle_id,
                  slot.vehicle_type, slot.reserved_until) == 5) {
        if (slot.slot_number == slot_num && slot.is_occupied == 0) {
            slot.is_occupied = 2;
            strncpy(slot.reserved_until, reserved_time, sizeof(slot.reserved_until));
            found = 1;
        }
        fprintf(temp, "%d|%d|%s|%s|%s\n",
                slot.slot_number, slot.is_occupied,
                slot.vehicle_id, slot.vehicle_type, slot.reserved_until);
    }
    fclose(fp);
    fclose(temp);

    remove("data/parking_slots.txt");
    rename("data/temp_slots.txt", "data/parking_slots.txt");

    if (found) {
        printf("Slot %d reserved until %s.\n", slot_num, reserved_time);
    } else {
        printf("Slot not available for reservation.\n");
    }
}

void release_parking_slot() {
    int slot_num, found = 0;
    printf("Enter Slot Number to release: ");
    if (scanf("%d", &slot_num) != 1) {
        printf("Invalid input.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        return;
    }
    int c; while ((c = getchar()) != '\n' && c != EOF) {}

    FILE *fp = fopen("data/parking_slots.txt", "r");
    FILE *temp = fopen("data/temp_slots.txt", "w");
    if (!fp || !temp) {
        printf("Error accessing parking slots file.\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    ParkingSlot slot;
    while (fscanf(fp, "%d|%d|%19[^|]|%19[^|]|%24[^\n]\n",
                  &slot.slot_number, &slot.is_occupied, slot.vehicle_id,
                  slot.vehicle_type, slot.reserved_until) == 5) {
        if (slot.slot_number == slot_num && slot.is_occupied != 0) {
            slot.is_occupied = 0;
            strcpy(slot.vehicle_id, "-");
            strcpy(slot.vehicle_type, "-");
            strcpy(slot.reserved_until, "-");
            found = 1;
        }
        fprintf(temp, "%d|%d|%s|%s|%s\n",
                slot.slot_number, slot.is_occupied,
                slot.vehicle_id, slot.vehicle_type, slot.reserved_until);
    }
    fclose(fp);
    fclose(temp);

    remove("data/parking_slots.txt");
    rename("data/temp_slots.txt", "data/parking_slots.txt");

    if (found) {
        printf("Slot %d is now available.\n", slot_num);
    } else {
        printf("Slot not currently occupied or reserved.\n");
    }
}
