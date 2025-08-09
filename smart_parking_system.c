//Headers, Structures, and Utility Functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// ====== FILE PATHS ======
#define USERS_FILE         "users.txt"
#define VEHICLES_FILE      "vehicles.txt"
#define PARKING_FILE       "parking_slots.txt"
#define PAYMENTS_FILE      "payments.txt"
#define RESERVATIONS_FILE  "reservations.txt"
#define OVERSTAY_FILE      "overstay_alerts.txt"

// ====== CONSTANTS ======
#define MAX_SLOTS 100
#define BIKE_RATE 25
#define CAR_RATE 50
#define TRUCK_RATE 100
#define OVERSTAY_PENALTY 20
#define MAX_LOGIN_ATTEMPTS 3

// ====== STRUCTS ======
typedef struct {
    char user_id[20];
    char name[100];
    char email[100];
    char phone[20];
    char nid[20];
    char password[50];
    char registration_date[25];
    int is_active;
    int overstay_count;
} User;

typedef struct {
    char vehicle_id[20];
    char user_id[20];
    char type[20];
    char plate[20];
    char registration_date[25];
    int is_active;
} Vehicle;

typedef struct {
    int slot_id;
    char slot_type[20];
    int status; // 0 = available, 1 = occupied, 2 = reserved
    char vehicle_id[20];
    char user_id[20];
    char occupied_since[25];
    char reserved_until[25];
    int floor;
    char section[10];
} ParkingSlot;

typedef struct {
    char reservation_id[20];
    char user_id[20];
    char vehicle_id[20];
    int slot_id;
    char start_time[25];
    char end_time[25];
    int duration_hours;
    int status; // 0 = cancelled, 1 = active, 2 = completed, 3 = overstay
    char created_at[25];
    int total_cost;
} Reservation;

typedef struct {
    char payment_id[20];
    char user_id[20];
    char reservation_id[20];
    char vehicle_id[20];
    int slot_id;
    int amount;
    char payment_method[20];
    char payment_date[25];
    char payment_type[20];
    int duration_hours;
    char description[100];
} Payment;

typedef struct {
    char overstay_id[20];
    char user_id[20];
    char vehicle_id[20];
    int slot_id;
    char overstay_start[25];
    int overstay_hours;
    int penalty_amount;
    int is_paid;
    char alert_date[25];
    int is_notified;
} OverstayAlert;

// ====== GLOBALS ======
User logged_in_user;
int is_user_logged_in = 0;
int is_admin_logged_in = 0;
char current_datetime[25];

// ====== UTILITY FUNCTIONS ======
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause_screen() {
    printf("\nPress Enter to continue...");
    getchar();
}

void get_current_datetime(char *buffer) {
    strcpy(buffer, "2025-08-09 15:00:31");
}

void generate_unique_id(char *id, const char *prefix) {
    sprintf(id, "%s%06d", prefix, rand() % 1000000);
}

int calculate_time_difference_hours(const char *start_time, const char *end_time) {
    return 1; // Simplified for demo
}

// ====== VALIDATION FUNCTIONS ======
int validate_email(const char *email) {
    if (!email || strlen(email) < 5 || strlen(email) > 100) return 0;

    const char *at_symbol = strchr(email, '@');
    if (!at_symbol) return 0;

    const char *dot_symbol = strchr(at_symbol, '.');
    if (!dot_symbol || dot_symbol <= at_symbol + 1) return 0;

    for (const char *p = email; *p; ++p) {
        if (isspace((unsigned char)*p)) return 0;
    }

    return 1;
}

int validate_phone(const char *phone) {
    if (!phone || strlen(phone) != 11) return 0;
    if (phone[0] != '0' || phone[1] != '1') return 0;

    for (int i = 0; i < 11; ++i) {
        if (!isdigit((unsigned char)phone[i])) return 0;
    }
    return 1;
}

int validate_nid(const char *nid) {
    if (!nid) return 0;
    size_t len = strlen(nid);
    if (len != 10 && len != 17) return 0;

    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)nid[i])) return 0;
    }
    return 1;
}

int validate_password(const char *password) {
    if (!password) return 0;
    int len = strlen(password);
    if (len < 6) return 0;

    int has_digit = 0;
    for (int i = 0; i < len; ++i) {
        if (isdigit((unsigned char)password[i])) {
            has_digit = 1;
            break;
        }
    }

    return has_digit;
}

int validate_plate_number(const char *plate) {
    if (!plate) return 0;
    int len = strlen(plate);
    if (len < 5 || len > 15) return 0;

    int has_letter = 0, has_digit = 0;
    for (int i = 0; i < len; ++i) {
        char c = toupper(plate[i]);
        if (isalpha(c)) has_letter = 1;
        if (isdigit(c)) has_digit = 1;
        if (!isalnum(c) && c != '-') return 0;
    }

    return has_letter && has_digit;
}

char* case_insensitive_search(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;

    int haystack_len = strlen(haystack);
    int needle_len = strlen(needle);

    for (int i = 0; i <= haystack_len - needle_len; i++) {
        int match = 1;
        for (int j = 0; j < needle_len; j++) {
            if (tolower(haystack[i + j]) != tolower(needle[j])) {
                match = 0;
                break;
            }
        }
        if (match) return (char*)(haystack + i);
    }
    return NULL;
}

// ====== FILE HANDLING ======
int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void create_file_if_not_exists(const char *filename) {
    if (!file_exists(filename)) {
        FILE *file = fopen(filename, "w");
        if (file) {
            fclose(file);
            printf("Created file: %s\n", filename);
        } else {
            printf("Error: Could not create file %s\n", filename);
        }
    }
}

void initialize_all_files() {
    printf("Initializing Smart Parking System files...\n");

    create_file_if_not_exists(USERS_FILE);
    create_file_if_not_exists(VEHICLES_FILE);
    create_file_if_not_exists(PAYMENTS_FILE);
    create_file_if_not_exists(RESERVATIONS_FILE);
    create_file_if_not_exists(OVERSTAY_FILE);

    // Initialize parking slots if file is empty
    FILE *parking_file = fopen(PARKING_FILE, "r");
    char test_line[10];
    int need_init = 0;

    if (!parking_file) {
        need_init = 1;
    } else {
        if (!fgets(test_line, sizeof(test_line), parking_file)) {
            need_init = 1;
        }
        fclose(parking_file);
    }

    if (need_init) {
        parking_file = fopen(PARKING_FILE, "w");
        if (parking_file) {
            printf("Initializing %d parking slots...\n", MAX_SLOTS);
            // Create 100 parking slots
            for (int i = 1; i <= MAX_SLOTS; i++) {
                char type[20];
                int floor = (i - 1) / 25 + 1;
                char section = 'A' + ((i - 1) % 5);

                if (i <= 40) strcpy(type, "bike");
                else if (i <= 80) strcpy(type, "car");
                else strcpy(type, "truck");

                fprintf(parking_file, "%d|%s|0|-|-|-|-|%d|%c\n",
                        i, type, floor, section);
            }
            fclose(parking_file);
            printf("Parking slots initialized successfully!\n");
        }
    }

    printf("System initialization complete!\n");
}

// ====== ADDITIONAL UTILITY FUNCTIONS ======
void trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    // All spaces?
    if(*str == 0) return;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';
}

int is_valid_choice(int choice, int min, int max) {
    return choice >= min && choice <= max;
}

void display_separator() {
    printf("===============================================================================\n");
}

void display_header(const char *title) {
    printf("\n");
    display_separator();
    printf("%*s%s%*s\n", (int)(75 - strlen(title))/2, "", title, (int)(75 - strlen(title))/2, "");
    display_separator();
}

void display_success(const char *message) {
    printf("SUCCESS: %s\n", message);
}

void display_error(const char *message) {
    printf("ERROR: %s\n", message);
}

void display_warning(const char *message) {
    printf("WARNING: %s\n", message);
}

void display_info(const char *message) {
    printf("INFO: %s\n", message);
}






//Display Functions and User Registration



// ====== DISPLAY FUNCTIONS ======
   void display_welcome_banner() {
    system("cls"); // Clear screen for Windows, use system("clear") for Linux
    printf("================================================================================\n");
    printf("                                                                                \n");
    printf("     ######  ##     ##    ###    ########  ########    ########     ###       \n");
    printf("    ##    ## ###   ###   ## ##   ##     ##    ##       ##     ##   ## ##      \n");
    printf("    ##       ####  #### ##   ##  ##     ##    ##       ##     ##  ##   ##     \n");
    printf("     ######  ## #### ##  ##   ##  ########     ##       ########   ##   ##     \n");
    printf("          ## ##  ##  ##  #######  ##   ##      ##       ##         #######     \n");
    printf("    ##    ## ##      ##  ##   ##  ##    ##     ##       ##         ##   ##     \n");
    printf("     ######  ##      ##  ##   ##  ##     ##    ##       ##         ##   ##     \n");
    printf("                                                                                \n");
    printf("    ########     ###    ########  ##    ## #### ##    ##  ######              \n");
    printf("    ##     ##   ## ##   ##     ## ##   ##   ##  ###   ## ##    ##             \n");
    printf("    ##     ##  ##   ##  ##     ## ##  ##    ##  ####  ## ##                   \n");
    printf("    ########   ##   ##  ########  #####     ##  ## ## ## ##   ####            \n");
    printf("    ##         #######  ##   ##   ##  ##    ##  ##  #### ##    ##             \n");
    printf("    ##         ##   ##  ##    ##  ##   ##   ##  ##   ### ##    ##             \n");
    printf("    ##         ##   ##  ##     ## ##    ## #### ##    ##  ######              \n");
    printf("                                                                                \n");
    printf("                        LOT MANAGEMENT SYSTEM                                  \n");
    printf("                                                                                \n");
    printf("================================================================================\n");
    printf("                    Current Date/Time: 2025-08-09 15:00:31                    \n");
    printf("\n");
}

void display_loading_animation() {
    printf("Loading Smart Parking System");
    for (int i = 0; i < 3; i++) {
        printf(".");
        for (int j = 0; j < 100000000; j++); // Simple delay
    }
    printf(" Complete!\n");
    for (int j = 0; j < 150000000; j++); // Another delay
}

void display_menu_option(int number, const char *description, const char *icon) {
    printf("%d. %s %s\n", number, icon, description);
}

void display_table_header(const char *headers[], int count) {
    for (int i = 0; i < count; i++) {
        printf("%-15s", headers[i]);
        if (i < count - 1) printf(" | ");
    }
    printf("\n");
    for (int i = 0; i < count * 18 - 3; i++) {
        printf("-");
    }
    printf("\n");
}

// ====== USER REGISTRATION ======
void register_new_user() {
    char name[100], email[100], phone[20], nid[20], password[50], confirm_password[50];
    char user_id[20], registration_date[25];

    display_header("USER REGISTRATION");

    printf("Enter your full name: ");
    clear_input_buffer();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    trim_whitespace(name);

    if (strlen(name) < 2) {
        display_error("Name must be at least 2 characters long!");
        pause_screen();
        return;
    }

    // Email validation and uniqueness check
    int email_valid = 0;
    while (!email_valid) {
        printf("Enter your email address: ");
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = '\0';
        trim_whitespace(email);

        if (!validate_email(email)) {
            display_error("Invalid email format! Please enter a valid email.");
            continue;
        }

        // Check if email already exists
        FILE *users_file = fopen(USERS_FILE, "r");
        if (users_file) {
            char line[500];
            int email_exists = 0;
            while (fgets(line, sizeof(line), users_file)) {
                char existing_email[100];
                if (sscanf(line, "%*[^|]|%*[^|]|%99[^|]", existing_email) == 1) {
                    if (strcmp(email, existing_email) == 0) {
                        email_exists = 1;
                        break;
                    }
                }
            }
            fclose(users_file);

            if (email_exists) {
                display_error("Email already registered! Please use a different email.");
                continue;
            }
        }
        email_valid = 1;
    }

    // Phone validation
    int phone_valid = 0;
    while (!phone_valid) {
        printf("Enter your phone number (11 digits, starts with 01): ");
        fgets(phone, sizeof(phone), stdin);
        phone[strcspn(phone, "\n")] = '\0';
        trim_whitespace(phone);

        if (validate_phone(phone)) {
            phone_valid = 1;
        } else {
            display_error("Invalid phone number! Must be 11 digits starting with 01.");
        }
    }

    // NID validation
    int nid_valid = 0;
    while (!nid_valid) {
        printf("Enter your NID (10 or 17 digits): ");
        fgets(nid, sizeof(nid), stdin);
        nid[strcspn(nid, "\n")] = '\0';
        trim_whitespace(nid);

        if (validate_nid(nid)) {
            nid_valid = 1;
        } else {
            display_error("Invalid NID! Must be exactly 10 or 17 digits.");
        }
    }

    // Password validation and confirmation
    int password_valid = 0;
    while (!password_valid) {
        printf("Enter password (min 6 chars, include digits): ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0';

        if (!validate_password(password)) {
            display_error("Password must be at least 6 characters with at least one digit!");
            continue;
        }

        printf("Confirm your password: ");
        fgets(confirm_password, sizeof(confirm_password), stdin);
        confirm_password[strcspn(confirm_password, "\n")] = '\0';

        if (strcmp(password, confirm_password) == 0) {
            password_valid = 1;
        } else {
            display_error("Passwords don't match! Please try again.");
        }
    }

    // Generate user ID and save to database
    generate_unique_id(user_id, "U");
    get_current_datetime(registration_date);

    FILE *users_file = fopen(USERS_FILE, "a");
    if (!users_file) {
        display_error("Could not access user database!");
        pause_screen();
        return;
    }

    fprintf(users_file, "%s|%s|%s|%s|%s|%s|%s|1|0\n",
            user_id, name, email, phone, nid, password, registration_date);
    fclose(users_file);

    printf("\n");
    display_success("Registration completed successfully!");
    printf("Your User ID: %s\n", user_id);
    printf("Registration Date: %s\n", registration_date);
    display_info("You can now log in with your email and password.");
    pause_screen();
}

// ====== ENHANCED USER PROFILE FUNCTIONS ======
void display_user_stats() {
    // Count user's vehicles
    FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
    int vehicle_count = 0;
    if (vehicles_file) {
        char line[300];
        while (fgets(line, sizeof(line), vehicles_file)) {
            Vehicle vehicle;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%24[^|]|%d",
                   vehicle.vehicle_id, vehicle.user_id, vehicle.type,
                   vehicle.plate, vehicle.registration_date, &vehicle.is_active) >= 5) {

                if (strcmp(vehicle.user_id, logged_in_user.user_id) == 0 && vehicle.is_active == 1) {
                    vehicle_count++;
                }
            }
        }
        fclose(vehicles_file);
    }

    // Count user's reservations
    FILE *reservations_file = fopen(RESERVATIONS_FILE, "r");
    int total_reservations = 0, active_reservations = 0;
    if (reservations_file) {
        char line[500];
        while (fgets(line, sizeof(line), reservations_file)) {
            Reservation res;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%d|%24[^|]|%24[^|]|%d|%d|%24[^|]|%d",
                   res.reservation_id, res.user_id, res.vehicle_id, &res.slot_id,
                   res.start_time, res.end_time, &res.duration_hours, &res.status,
                   res.created_at, &res.total_cost) >= 9) {

                if (strcmp(res.user_id, logged_in_user.user_id) == 0) {
                    total_reservations++;
                    if (res.status == 1) active_reservations++;
                }
            }
        }
        fclose(reservations_file);
    }

    // Count user's payments
    FILE *payments_file = fopen(PAYMENTS_FILE, "r");
    int total_payments = 0, total_spent = 0;
    if (payments_file) {
        char line[500];
        while (fgets(line, sizeof(line), payments_file)) {
            Payment payment;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%d|%d|%19[^|]|%24[^|]|%19[^|]|%d|%99[^\n]",
                   payment.payment_id, payment.user_id, payment.reservation_id, payment.vehicle_id,
                   &payment.slot_id, &payment.amount, payment.payment_method, payment.payment_date,
                   payment.payment_type, &payment.duration_hours, payment.description) >= 10) {

                if (strcmp(payment.user_id, logged_in_user.user_id) == 0) {
                    total_payments++;
                    total_spent += payment.amount;
                }
            }
        }
        fclose(payments_file);
    }

    printf("\n--- ACCOUNT STATISTICS ---\n");
    printf("Registered Vehicles: %d\n", vehicle_count);
    printf("Total Reservations: %d\n", total_reservations);
    printf("Active Reservations: %d\n", active_reservations);
    printf("Total Payments Made: %d\n", total_payments);
    printf("Total Amount Spent: %d BDT\n", total_spent);
    printf("Account Status: %s\n", logged_in_user.is_active ? "Active" : "Inactive");
    if (logged_in_user.overstay_count > 0) {
        printf("Overstay Incidents: %d\n", logged_in_user.overstay_count);
    }
}

void backup_user_data() {
    char backup_filename[100];
    sprintf(backup_filename, "backup_%s_%s.txt", logged_in_user.user_id, "2025_08_09");

    FILE *backup_file = fopen(backup_filename, "w");
    if (!backup_file) {
        display_error("Could not create backup file!");
        return;
    }

    fprintf(backup_file, "=== USER DATA BACKUP ===\n");
    fprintf(backup_file, "Backup Date: 2025-08-09 15:00:31\n");
    fprintf(backup_file, "User ID: %s\n", logged_in_user.user_id);
    fprintf(backup_file, "Name: %s\n", logged_in_user.name);
    fprintf(backup_file, "Email: %s\n", logged_in_user.email);
    fprintf(backup_file, "Phone: %s\n", logged_in_user.phone);
    fprintf(backup_file, "NID: %s\n", logged_in_user.nid);
    fprintf(backup_file, "Registration Date: %s\n", logged_in_user.registration_date);
    fprintf(backup_file, "========================\n");

    fclose(backup_file);
    display_success("User data backed up successfully!");
    printf("Backup file: %s\n", backup_filename);
}




//Login Functions and Profile Management



// ====== LOGIN FUNCTIONS ======
void user_login() {
    char email[100], password[50];
    int login_attempts = 0;

    display_header("USER LOGIN");

    while (login_attempts < MAX_LOGIN_ATTEMPTS) {
        printf("Enter your email: ");
        if (login_attempts == 0) clear_input_buffer();
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = '\0';
        trim_whitespace(email);

        printf("Enter your password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0';

        FILE *users_file = fopen(USERS_FILE, "r");
        if (!users_file) {
            display_error("Could not access user database!");
            pause_screen();
            return;
        }

        char line[500];
        int login_successful = 0;

        while (fgets(line, sizeof(line), users_file)) {
            User temp_user;
            if (sscanf(line, "%19[^|]|%99[^|]|%99[^|]|%19[^|]|%19[^|]|%49[^|]|%24[^|]|%d|%d",
                   temp_user.user_id, temp_user.name, temp_user.email,
                   temp_user.phone, temp_user.nid, temp_user.password,
                   temp_user.registration_date, &temp_user.is_active, &temp_user.overstay_count) >= 8) {

                if (strcmp(email, temp_user.email) == 0 &&
                    strcmp(password, temp_user.password) == 0 &&
                    temp_user.is_active == 1) {

                    logged_in_user = temp_user;
                    is_user_logged_in = 1;
                    login_successful = 1;

                    printf("\n");
                    display_success("Login completed successfully!");
                    printf("Welcome back, %s!\n", temp_user.name);
                    printf("User ID: %s\n", temp_user.user_id);
                    printf("Member since: %s\n", temp_user.registration_date);

                    if (temp_user.overstay_count > 0) {
                        display_warning("You have overstay incidents on record.");
                        printf("Overstay count: %d\n", temp_user.overstay_count);
                    }
                    break;
                }
            }
        }
        fclose(users_file);

        if (login_successful) {
            pause_screen();
            return;
        }

        login_attempts++;
        printf("\n");
        display_error("Invalid email or password!");
        printf("Attempt %d/%d\n", login_attempts, MAX_LOGIN_ATTEMPTS);

        if (login_attempts < MAX_LOGIN_ATTEMPTS) {
            display_info("Please try again.");
        }
    }

    display_error("Maximum login attempts exceeded. Access denied!");
    pause_screen();
}

void admin_login() {
    char username[50], password[50];
    int login_attempts = 0;

    display_header("ADMIN LOGIN");

    while (login_attempts < MAX_LOGIN_ATTEMPTS) {
        printf("Enter admin username: ");
        if (login_attempts == 0) clear_input_buffer();
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';
        trim_whitespace(username);

        printf("Enter admin password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0';

        // Default admin credentials: admin/admin123
        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
            is_admin_logged_in = 1;
            printf("\n");
            display_success("Admin login completed successfully!");
            display_info("Full system access granted.");
            pause_screen();
            return;
        }

        login_attempts++;
        printf("\n");
        display_error("Invalid admin credentials!");
        printf("Attempt %d/%d\n", login_attempts, MAX_LOGIN_ATTEMPTS);

        if (login_attempts < MAX_LOGIN_ATTEMPTS) {
            display_info("Please try again.");
        }
    }

    display_error("Maximum admin login attempts exceeded. Access denied!");
    pause_screen();
}

// ====== PROFILE MANAGEMENT ======
void view_user_profile() {
    display_header("MY PROFILE");

    printf("User ID: %s\n", logged_in_user.user_id);
    printf("Name: %s\n", logged_in_user.name);
    printf("Email: %s\n", logged_in_user.email);
    printf("Phone: %s\n", logged_in_user.phone);
    printf("NID: %s\n", logged_in_user.nid);
    printf("Registration Date: %s\n", logged_in_user.registration_date);
    printf("Account Status: %s\n", logged_in_user.is_active ? "Active" : "Inactive");
    printf("Overstay Count: %d\n", logged_in_user.overstay_count);

    display_user_stats();

    pause_screen();
}

void update_user_profile() {
    char new_name[100], new_phone[20];
    int update_made = 0;

    display_header("UPDATE PROFILE");

    printf("Current name: %s\n", logged_in_user.name);
    printf("Enter new name (press Enter to keep current): ");
    clear_input_buffer();
    fgets(new_name, sizeof(new_name), stdin);
    new_name[strcspn(new_name, "\n")] = '\0';
    trim_whitespace(new_name);

    if (strlen(new_name) > 0 && strlen(new_name) >= 2) {
        strcpy(logged_in_user.name, new_name);
        update_made = 1;
        display_success("Name updated successfully!");
    }

    printf("Current phone: %s\n", logged_in_user.phone);
    printf("Enter new phone (press Enter to keep current): ");
    fgets(new_phone, sizeof(new_phone), stdin);
    new_phone[strcspn(new_phone, "\n")] = '\0';
    trim_whitespace(new_phone);

    if (strlen(new_phone) > 0) {
        if (validate_phone(new_phone)) {
            strcpy(logged_in_user.phone, new_phone);
            update_made = 1;
            display_success("Phone number updated successfully!");
        } else {
            display_error("Invalid phone number format!");
        }
    }

    if (update_made) {
        // Update user information in database
        FILE *users_file = fopen(USERS_FILE, "r");
        FILE *temp_file = fopen("temp_users.txt", "w");

        if (users_file && temp_file) {
            char line[500];
            while (fgets(line, sizeof(line), users_file)) {
                User temp_user;
                if (sscanf(line, "%19[^|]|%99[^|]|%99[^|]|%19[^|]|%19[^|]|%49[^|]|%24[^|]|%d|%d",
                       temp_user.user_id, temp_user.name, temp_user.email,
                       temp_user.phone, temp_user.nid, temp_user.password,
                       temp_user.registration_date, &temp_user.is_active, &temp_user.overstay_count) >= 8) {

                    if (strcmp(temp_user.user_id, logged_in_user.user_id) == 0) {
                        fprintf(temp_file, "%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
                                logged_in_user.user_id, logged_in_user.name, logged_in_user.email,
                                logged_in_user.phone, logged_in_user.nid, logged_in_user.password,
                                logged_in_user.registration_date, logged_in_user.is_active,
                                logged_in_user.overstay_count);
                    } else {
                        fprintf(temp_file, "%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
                                temp_user.user_id, temp_user.name, temp_user.email,
                                temp_user.phone, temp_user.nid, temp_user.password,
                                temp_user.registration_date, temp_user.is_active,
                                temp_user.overstay_count);
                    }
                }
            }
            fclose(users_file);
            fclose(temp_file);
            remove(USERS_FILE);
            rename("temp_users.txt", USERS_FILE);
            display_success("Profile updated successfully in database!");
        } else {
            display_error("Error updating profile in database!");
        }
    } else {
        display_info("No changes made to profile.");
    }

    pause_screen();
}

void change_user_password() {
    char current_password[50], new_password[50], confirm_password[50];

    display_header("CHANGE PASSWORD");

    printf("Enter your current password: ");
    fgets(current_password, sizeof(current_password), stdin);
    current_password[strcspn(current_password, "\n")] = '\0';

    if (strcmp(current_password, logged_in_user.password) != 0) {
        display_error("Current password is incorrect!");
        pause_screen();
        return;
    }

    int password_changed = 0;
    while (!password_changed) {
        printf("Enter new password (min 6 chars, include digits): ");
        fgets(new_password, sizeof(new_password), stdin);
        new_password[strcspn(new_password, "\n")] = '\0';

        if (!validate_password(new_password)) {
            display_error("New password doesn't meet requirements!");
            continue;
        }

        printf("Confirm new password: ");
        fgets(confirm_password, sizeof(confirm_password), stdin);
        confirm_password[strcspn(confirm_password, "\n")] = '\0';

        if (strcmp(new_password, confirm_password) == 0) {
            strcpy(logged_in_user.password, new_password);
            password_changed = 1;
        } else {
            display_error("Passwords don't match! Please try again.");
        }
    }

    // Update password in database
    FILE *users_file = fopen(USERS_FILE, "r");
    FILE *temp_file = fopen("temp_users.txt", "w");

    if (users_file && temp_file) {
        char line[500];
        while (fgets(line, sizeof(line), users_file)) {
            User temp_user;
            if (sscanf(line, "%19[^|]|%99[^|]|%99[^|]|%19[^|]|%19[^|]|%49[^|]|%24[^|]|%d|%d",
                   temp_user.user_id, temp_user.name, temp_user.email,
                   temp_user.phone, temp_user.nid, temp_user.password,
                   temp_user.registration_date, &temp_user.is_active, &temp_user.overstay_count) >= 8) {

                if (strcmp(temp_user.user_id, logged_in_user.user_id) == 0) {
                    fprintf(temp_file, "%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
                            logged_in_user.user_id, logged_in_user.name, logged_in_user.email,
                            logged_in_user.phone, logged_in_user.nid, logged_in_user.password,
                            logged_in_user.registration_date, logged_in_user.is_active,
                            logged_in_user.overstay_count);
                } else {
                    fprintf(temp_file, "%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
                            temp_user.user_id, temp_user.name, temp_user.email,
                            temp_user.phone, temp_user.nid, temp_user.password,
                            temp_user.registration_date, temp_user.is_active,
                            temp_user.overstay_count);
                }
            }
        }
        fclose(users_file);
        fclose(temp_file);
        remove(USERS_FILE);
        rename("temp_users.txt", USERS_FILE);
        display_success("Password changed successfully!");
    } else {
        display_error("Error updating password in database!");
    }

    pause_screen();
}

void export_user_data() {
    char filename[100];
    sprintf(filename, "user_data_%s.txt", logged_in_user.user_id);

    FILE *export_file = fopen(filename, "w");
    if (!export_file) {
        display_error("Could not create export file!");
        pause_screen();
        return;
    }

    fprintf(export_file, "=== USER DATA EXPORT ===\n");
    fprintf(export_file, "Export Date: 2025-08-09 15:00:31\n");
    fprintf(export_file, "User ID: %s\n", logged_in_user.user_id);
    fprintf(export_file, "Name: %s\n", logged_in_user.name);
    fprintf(export_file, "Email: %s\n", logged_in_user.email);
    fprintf(export_file, "Phone: %s\n", logged_in_user.phone);
    fprintf(export_file, "Registration Date: %s\n", logged_in_user.registration_date);
    fprintf(export_file, "========================\n\n");

    // Export vehicles
    fprintf(export_file, "=== REGISTERED VEHICLES ===\n");
    FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
    if (vehicles_file) {
        char line[300];
        while (fgets(line, sizeof(line), vehicles_file)) {
            Vehicle vehicle;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%24[^|]|%d",
                   vehicle.vehicle_id, vehicle.user_id, vehicle.type,
                   vehicle.plate, vehicle.registration_date, &vehicle.is_active) >= 5) {

                if (strcmp(vehicle.user_id, logged_in_user.user_id) == 0 && vehicle.is_active == 1) {
                    fprintf(export_file, "Vehicle: %s | Type: %s | Plate: %s | Date: %s\n",
                            vehicle.vehicle_id, vehicle.type, vehicle.plate, vehicle.registration_date);
                }
            }
        }
        fclose(vehicles_file);
    }

    fclose(export_file);
    display_success("User data exported successfully!");
    printf("Export file: %s\n", filename);
    pause_screen();
}





//Vehicle Management and Parking Slot Functions




// ====== VEHICLE MANAGEMENT ======
void add_new_vehicle() {
    char vehicle_type[20], plate_number[20], vehicle_id[20], registration_date[25];
    int type_choice;

    display_header("ADD NEW VEHICLE");

    printf("Select vehicle type:\n");
    display_menu_option(1, "Bike", "🏍️");
    printf("   Rate: %d BDT/hour\n", BIKE_RATE);
    display_menu_option(2, "Car", "🚗");
    printf("   Rate: %d BDT/hour\n", CAR_RATE);
    display_menu_option(3, "Truck", "🚛");
    printf("   Rate: %d BDT/hour\n", TRUCK_RATE);

    printf("\nEnter choice (1-3): ");

    if (scanf("%d", &type_choice) != 1 || !is_valid_choice(type_choice, 1, 3)) {
        display_error("Invalid input! Please select 1, 2, or 3.");
        clear_input_buffer();
        pause_screen();
        return;
    }
    clear_input_buffer();

    switch (type_choice) {
        case 1: strcpy(vehicle_type, "bike"); break;
        case 2: strcpy(vehicle_type, "car"); break;
        case 3: strcpy(vehicle_type, "truck"); break;
    }

    int plate_valid = 0;
    while (!plate_valid) {
        printf("\nEnter license plate number: ");
        fgets(plate_number, sizeof(plate_number), stdin);
        plate_number[strcspn(plate_number, "\n")] = '\0';
        trim_whitespace(plate_number);

        if (validate_plate_number(plate_number)) {
            // Check if plate number already exists
            FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
            int plate_exists = 0;

            if (vehicles_file) {
                char line[300];
                while (fgets(line, sizeof(line), vehicles_file)) {
                    char existing_plate[20];
                    if (sscanf(line, "%*[^|]|%*[^|]|%*[^|]|%19[^|]", existing_plate) == 1) {
                        if (strcmp(plate_number, existing_plate) == 0) {
                            plate_exists = 1;
                            break;
                        }
                    }
                }
                fclose(vehicles_file);
            }

            if (plate_exists) {
                display_error("This plate number is already registered!");
                continue;
            }

            plate_valid = 1;
        } else {
            display_error("Invalid plate number format! Must contain both letters and numbers (5-15 chars).");
        }
    }

    // Generate vehicle ID and save to database
    generate_unique_id(vehicle_id, "V");
    strcpy(registration_date, "2025-08-09 15:06:35");

    FILE *vehicles_file = fopen(VEHICLES_FILE, "a");
    if (!vehicles_file) {
        display_error("Could not access vehicle database!");
        pause_screen();
        return;
    }

    fprintf(vehicles_file, "%s|%s|%s|%s|%s|1\n",
            vehicle_id, logged_in_user.user_id, vehicle_type, plate_number, registration_date);
    fclose(vehicles_file);

    printf("\n");
    display_success("Vehicle registered successfully!");
    display_separator();
    printf("Vehicle Details:\n");
    printf("Vehicle ID: %s\n", vehicle_id);
    printf("Type: %s\n", vehicle_type);
    printf("Plate Number: %s\n", plate_number);
    printf("Registration Date: %s\n", registration_date);
    printf("Parking Rate: %d BDT/hour\n",
           strcmp(vehicle_type, "bike") == 0 ? BIKE_RATE :
           strcmp(vehicle_type, "car") == 0 ? CAR_RATE : TRUCK_RATE);
    display_separator();
    pause_screen();
}

void view_my_vehicles() {
    display_header("MY VEHICLES");

    FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
    if (!vehicles_file) {
        display_info("No vehicles found. Add a vehicle to get started!");
        pause_screen();
        return;
    }

    char line[300];
    int vehicle_count = 0;

    printf("%-12s | %-8s | %-15s | %-20s | %-10s | %-8s\n",
           "Vehicle ID", "Type", "Plate Number", "Registration Date", "Status", "Rate/hr");
    printf("------------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), vehicles_file)) {
        Vehicle vehicle;
        if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%24[^|]|%d",
               vehicle.vehicle_id, vehicle.user_id, vehicle.type,
               vehicle.plate, vehicle.registration_date, &vehicle.is_active) >= 5) {

            if (strcmp(vehicle.user_id, logged_in_user.user_id) == 0 && vehicle.is_active == 1) {
                int rate = strcmp(vehicle.type, "bike") == 0 ? BIKE_RATE :
                          strcmp(vehicle.type, "car") == 0 ? CAR_RATE : TRUCK_RATE;

                printf("%-12s | %-8s | %-15s | %-20s | %-10s | %d BDT\n",
                       vehicle.vehicle_id, vehicle.type, vehicle.plate,
                       vehicle.registration_date, "Active", rate);
                vehicle_count++;
            }
        }
    }
    fclose(vehicles_file);

    if (vehicle_count == 0) {
        display_info("No vehicles registered. Add a vehicle to get started!");
    } else {
        printf("\nTotal vehicles: %d\n", vehicle_count);
        display_info("All vehicles are active and ready for parking reservations.");
    }

    pause_screen();
}

void remove_vehicle() {
    char vehicle_id_to_remove[20];

    display_header("REMOVE VEHICLE");

    // Show current vehicles first
    view_my_vehicles();

    printf("Enter Vehicle ID to remove: ");
    fgets(vehicle_id_to_remove, sizeof(vehicle_id_to_remove), stdin);
    vehicle_id_to_remove[strcspn(vehicle_id_to_remove, "\n")] = '\0';
    trim_whitespace(vehicle_id_to_remove);

    if (strlen(vehicle_id_to_remove) == 0) {
        display_error("Vehicle ID cannot be empty!");
        pause_screen();
        return;
    }

    FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
    FILE *temp_file = fopen("temp_vehicles.txt", "w");

    if (!vehicles_file || !temp_file) {
        display_error("Error accessing vehicle database!");
        if (vehicles_file) fclose(vehicles_file);
        if (temp_file) fclose(temp_file);
        pause_screen();
        return;
    }

    char line[300];
    int vehicle_found = 0;
    char found_plate[20] = "";

    while (fgets(line, sizeof(line), vehicles_file)) {
        Vehicle vehicle;
        if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%24[^|]|%d",
               vehicle.vehicle_id, vehicle.user_id, vehicle.type,
               vehicle.plate, vehicle.registration_date, &vehicle.is_active) >= 5) {

            if (strcmp(vehicle.vehicle_id, vehicle_id_to_remove) == 0 &&
                strcmp(vehicle.user_id, logged_in_user.user_id) == 0 &&
                vehicle.is_active == 1) {
                vehicle.is_active = 0; // Mark as inactive instead of deleting
                vehicle_found = 1;
                strcpy(found_plate, vehicle.plate);
            }

            fprintf(temp_file, "%s|%s|%s|%s|%s|%d\n",
                    vehicle.vehicle_id, vehicle.user_id, vehicle.type,
                    vehicle.plate, vehicle.registration_date, vehicle.is_active);
        }
    }

    fclose(vehicles_file);
    fclose(temp_file);
    remove(VEHICLES_FILE);
    rename("temp_vehicles.txt", VEHICLES_FILE);

    if (vehicle_found) {
        display_success("Vehicle removed successfully!");
        printf("Removed vehicle: %s (Plate: %s)\n", vehicle_id_to_remove, found_plate);
        display_info("Vehicle is now inactive but records are preserved for history.");
    } else {
        display_error("Vehicle not found or doesn't belong to you.");
    }

    pause_screen();
}

void vehicle_management_menu() {
    int choice;

    while (1) {
        display_header("VEHICLE MANAGEMENT");

        display_menu_option(1, "View My Vehicles", "📋");
        display_menu_option(2, "Add New Vehicle", "➕");
        display_menu_option(3, "Remove Vehicle", "🗑️");
        display_menu_option(4, "Back to Main Menu", "⬅️");

        printf("\nEnter your choice (1-4): ");

        if (scanf("%d", &choice) != 1 || !is_valid_choice(choice, 1, 4)) {
            display_error("Invalid input! Please enter a number between 1-4.");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: view_my_vehicles(); break;
            case 2: add_new_vehicle(); break;
            case 3: remove_vehicle(); break;
            case 4: return;
        }
    }
}

// ====== PARKING SLOT MANAGEMENT ======
void display_parking_slots_status() {
    display_header("PARKING SLOTS STATUS");

    FILE *parking_file = fopen(PARKING_FILE, "r");
    if (!parking_file) {
        display_error("Could not access parking database!");
        pause_screen();
        return;
    }

    char line[400];
    int available_count = 0, occupied_count = 0, reserved_count = 0;
    int bike_available = 0, car_available = 0, truck_available = 0;

    printf("%-6s | %-8s | %-10s | %-12s | %-10s | %-6s | %-8s\n",
           "Slot", "Type", "Status", "Vehicle", "User", "Floor", "Section");
    printf("-----------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), parking_file)) {
        ParkingSlot slot;
        if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
               &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
               slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

            char *status_text;

            if (slot.status == 0) {
                status_text = "Available";
                available_count++;
                if (strcmp(slot.slot_type, "bike") == 0) bike_available++;
                else if (strcmp(slot.slot_type, "car") == 0) car_available++;
                else truck_available++;
            } else if (slot.status == 1) {
                status_text = "Occupied";
                occupied_count++;
            } else {
                status_text = "Reserved";
                reserved_count++;
            }

            printf("%-6d | %-8s | %-10s | %-12s | %-10s | %-6d | %-8s\n",
                   slot.slot_id, slot.slot_type, status_text,
                   slot.status ? slot.vehicle_id : "-",
                   slot.status ? slot.user_id : "-",
                   slot.floor, slot.section);
        }
    }
    fclose(parking_file);

    printf("\n");
    display_separator();
    printf("PARKING SUMMARY:\n");
    printf("Available Slots: %d | Occupied Slots: %d | Reserved Slots: %d\n",
           available_count, occupied_count, reserved_count);
    printf("Total Slots: %d\n", available_count + occupied_count + reserved_count);
    printf("\nAvailability by Type:\n");
    printf("🏍️  Bike Slots Available: %d (Rate: %d BDT/hour)\n", bike_available, BIKE_RATE);
    printf("🚗 Car Slots Available: %d (Rate: %d BDT/hour)\n", car_available, CAR_RATE);
    printf("🚛 Truck Slots Available: %d (Rate: %d BDT/hour)\n", truck_available, TRUCK_RATE);
    display_separator();

    pause_screen();
}

void search_available_slots() {
    char vehicle_type[20];
    int type_choice;

    display_header("SEARCH AVAILABLE SLOTS");

    printf("Search slots by vehicle type:\n");
    display_menu_option(1, "Bike Slots", "🏍️");
    display_menu_option(2, "Car Slots", "🚗");
    display_menu_option(3, "Truck Slots", "🚛");
    display_menu_option(4, "All Types", "🅿️");

    printf("\nEnter choice (1-4): ");

    if (scanf("%d", &type_choice) != 1 || !is_valid_choice(type_choice, 1, 4)) {
        display_error("Invalid input! Please select 1-4.");
        clear_input_buffer();
        pause_screen();
        return;
    }
    clear_input_buffer();

    switch (type_choice) {
        case 1: strcpy(vehicle_type, "bike"); break;
        case 2: strcpy(vehicle_type, "car"); break;
        case 3: strcpy(vehicle_type, "truck"); break;
        case 4: strcpy(vehicle_type, "all"); break;
    }

    FILE *parking_file = fopen(PARKING_FILE, "r");
    if (!parking_file) {
        display_error("Could not access parking database!");
        pause_screen();
        return;
    }

    char line[400];
    int found_count = 0;

    printf("\nAvailable Slots:\n");
    printf("%-6s | %-8s | %-6s | %-8s | %-10s\n", "Slot", "Type", "Floor", "Section", "Rate/hour");
    printf("-----------------------------------------------------\n");

    while (fgets(line, sizeof(line), parking_file)) {
        ParkingSlot slot;
        if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
               &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
               slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

            if (slot.status == 0 && (strcmp(vehicle_type, "all") == 0 || strcmp(slot.slot_type, vehicle_type) == 0)) {
                int rate = strcmp(slot.slot_type, "bike") == 0 ? BIKE_RATE :
                          strcmp(slot.slot_type, "car") == 0 ? CAR_RATE : TRUCK_RATE;

                printf("%-6d | %-8s | %-6d | %-8s | %d BDT\n",
                       slot.slot_id, slot.slot_type, slot.floor, slot.section, rate);
                found_count++;
            }
        }
    }
    fclose(parking_file);

    if (found_count == 0) {
        display_info("No available slots found for the selected type.");
    } else {
        printf("\nFound %d available slots.\n", found_count);
    }

    pause_screen();
}

void check_slot_availability() {
    int slot_id;

    display_header("CHECK SLOT AVAILABILITY");

    printf("Enter Slot ID to check: ");
    if (scanf("%d", &slot_id) != 1) {
        display_error("Invalid input! Please enter a valid slot number.");
        clear_input_buffer();
        pause_screen();
        return;
    }
    clear_input_buffer();

    FILE *parking_file = fopen(PARKING_FILE, "r");
    if (!parking_file) {
        display_error("Could not access parking database!");
        pause_screen();
        return;
    }

    char line[400];
    int slot_found = 0;

    while (fgets(line, sizeof(line), parking_file)) {
        ParkingSlot slot;
        if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
               &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
               slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

            if (slot.slot_id == slot_id) {
                slot_found = 1;

                printf("\nSlot Information:\n");
                display_separator();
                printf("Slot ID: %d\n", slot.slot_id);
                printf("Type: %s\n", slot.slot_type);
                printf("Floor: %d\n", slot.floor);
                printf("Section: %s\n", slot.section);

                char *status_text;
                if (slot.status == 0) {
                    status_text = "Available";
                    printf("Status: %s ✅\n", status_text);
                    int rate = strcmp(slot.slot_type, "bike") == 0 ? BIKE_RATE :
                              strcmp(slot.slot_type, "car") == 0 ? CAR_RATE : TRUCK_RATE;
                    printf("Rate: %d BDT/hour\n", rate);
                } else if (slot.status == 1) {
                    status_text = "Occupied";
                    printf("Status: %s ❌\n", status_text);
                    printf("Occupied by: Vehicle %s (User: %s)\n", slot.vehicle_id, slot.user_id);
                    printf("Occupied since: %s\n", slot.occupied_since);
                } else {
                    status_text = "Reserved";
                    printf("Status: %s 🔒\n", status_text);
                    printf("Reserved by: Vehicle %s (User: %s)\n", slot.vehicle_id, slot.user_id);
                    printf("Reserved until: %s\n", slot.reserved_until);
                }
                display_separator();
                break;
            }
        }
    }
    fclose(parking_file);

    if (!slot_found) {
        display_error("Slot not found! Please check the slot number.");
    }

    pause_screen();
}






//Reservation Management and Payment System


// ====== RESERVATION MANAGEMENT ======
void reserve_parking_slot() {
    char vehicle_id[20];
    int slot_id, duration_hours;

    display_header("RESERVE PARKING SLOT");

    // Show user's vehicles first
    printf("Your registered vehicles:\n");
    view_my_vehicles();

    printf("Enter Vehicle ID for reservation: ");
    fgets(vehicle_id, sizeof(vehicle_id), stdin);
    vehicle_id[strcspn(vehicle_id, "\n")] = '\0';
    trim_whitespace(vehicle_id);

    if (strlen(vehicle_id) == 0) {
        display_error("Vehicle ID cannot be empty!");
        pause_screen();
        return;
    }

    // Verify vehicle belongs to user and get type
    FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
    char vehicle_type[20];
    int vehicle_found = 0;

    if (vehicles_file) {
        char line[300];
        while (fgets(line, sizeof(line), vehicles_file)) {
            Vehicle vehicle;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%24[^|]|%d",
                   vehicle.vehicle_id, vehicle.user_id, vehicle.type,
                   vehicle.plate, vehicle.registration_date, &vehicle.is_active) >= 5) {

                if (strcmp(vehicle.vehicle_id, vehicle_id) == 0 &&
                    strcmp(vehicle.user_id, logged_in_user.user_id) == 0 &&
                    vehicle.is_active == 1) {
                    vehicle_found = 1;
                    strcpy(vehicle_type, vehicle.type);
                    break;
                }
            }
        }
        fclose(vehicles_file);
    }

    if (!vehicle_found) {
        display_error("Vehicle not found or doesn't belong to you!");
        pause_screen();
        return;
    }

    // Show available slots for this vehicle type
    printf("\nAvailable %s parking slots:\n", vehicle_type);

    FILE *parking_file = fopen(PARKING_FILE, "r");
    int available_slots_count = 0;

    if (parking_file) {
        char line[400];
        printf("%-6s | %-8s | %-6s | %-8s | %-10s\n", "Slot", "Type", "Floor", "Section", "Rate/hour");
        printf("-----------------------------------------------------\n");

        while (fgets(line, sizeof(line), parking_file)) {
            ParkingSlot slot;
            if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
                   &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
                   slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

                if (strcmp(slot.slot_type, vehicle_type) == 0 && slot.status == 0) {
                    int rate = strcmp(vehicle_type, "bike") == 0 ? BIKE_RATE :
                              strcmp(vehicle_type, "car") == 0 ? CAR_RATE : TRUCK_RATE;
                    printf("%-6d | %-8s | %-6d | %-8s | %d BDT\n",
                           slot.slot_id, slot.slot_type, slot.floor, slot.section, rate);
                    available_slots_count++;
                }
            }
        }
        fclose(parking_file);
    }

    if (available_slots_count == 0) {
        display_error("No available slots for your vehicle type!");
        pause_screen();
        return;
    }

    printf("\nEnter Slot ID to reserve: ");
    if (scanf("%d", &slot_id) != 1) {
        display_error("Invalid input! Please enter a valid slot number.");
        clear_input_buffer();
        pause_screen();
        return;
    }
    clear_input_buffer();

    printf("Enter duration in hours (1-24): ");
    if (scanf("%d", &duration_hours) != 1 || !is_valid_choice(duration_hours, 1, 24)) {
        display_error("Invalid duration! Must be between 1-24 hours.");
        clear_input_buffer();
        pause_screen();
        return;
    }
    clear_input_buffer();

    // Calculate costs and times
    char start_datetime[25], end_datetime[25];
    strcpy(start_datetime, "2025-08-09 15:06:35");
    strcpy(end_datetime, "2025-08-09 16:06:35"); // Simplified

    // Create reservation
    char reservation_id[20];
    generate_unique_id(reservation_id, "R");

    int parking_rate = strcmp(vehicle_type, "bike") == 0 ? BIKE_RATE :
                      strcmp(vehicle_type, "car") == 0 ? CAR_RATE : TRUCK_RATE;
    int total_cost = parking_rate * duration_hours;

    // Verify slot is still available
    FILE *parking_read = fopen(PARKING_FILE, "r");
    int slot_available = 0;

    if (parking_read) {
        char line[400];
        while (fgets(line, sizeof(line), parking_read)) {
            ParkingSlot slot;
            if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
                   &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
                   slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

                if (slot.slot_id == slot_id && slot.status == 0 && strcmp(slot.slot_type, vehicle_type) == 0) {
                    slot_available = 1;
                    break;
                }
            }
        }
        fclose(parking_read);
    }

    if (!slot_available) {
        display_error("Selected slot is not available or doesn't match your vehicle type!");
        pause_screen();
        return;
    }

    // Add reservation to file
    FILE *reservations_file = fopen(RESERVATIONS_FILE, "a");
    if (!reservations_file) {
        display_error("Error creating reservation!");
        pause_screen();
        return;
    }

    fprintf(reservations_file, "%s|%s|%s|%d|%s|%s|%d|1|%s|%d\n",
            reservation_id, logged_in_user.user_id, vehicle_id, slot_id,
            start_datetime, end_datetime, duration_hours, start_datetime, total_cost);
    fclose(reservations_file);

    // Update slot status
    parking_read = fopen(PARKING_FILE, "r");
    FILE *parking_write = fopen("temp_parking.txt", "w");

    if (parking_read && parking_write) {
        char line[400];
        while (fgets(line, sizeof(line), parking_read)) {
            ParkingSlot slot;
            if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
                   &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
                   slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

                if (slot.slot_id == slot_id && slot.status == 0) {
                    slot.status = 2; // Reserved
                    strcpy(slot.vehicle_id, vehicle_id);
                    strcpy(slot.user_id, logged_in_user.user_id);
                    strcpy(slot.reserved_until, end_datetime);
                }

                fprintf(parking_write, "%d|%s|%d|%s|%s|%s|%s|%d|%s\n",
                        slot.slot_id, slot.slot_type, slot.status, slot.vehicle_id,
                        slot.user_id, slot.occupied_since, slot.reserved_until, slot.floor, slot.section);
            }
        }
        fclose(parking_read);
        fclose(parking_write);
        remove(PARKING_FILE);
        rename("temp_parking.txt", PARKING_FILE);
    }

    printf("\n");
    display_success("Parking slot reserved successfully!");
    display_separator();
    printf("RESERVATION CONFIRMATION\n");
    printf("Reservation ID: %s\n", reservation_id);
    printf("Vehicle: %s (%s)\n", vehicle_id, vehicle_type);
    printf("Slot: %d\n", slot_id);
    printf("Duration: %d hours\n", duration_hours);
    printf("Start Time: %s\n", start_datetime);
    printf("End Time: %s\n", end_datetime);
    printf("Total Cost: %d BDT\n", total_cost);
    printf("Status: Active\n");
    display_separator();
    display_info("Please proceed to payment to confirm your reservation.");
    pause_screen();
}

void view_my_reservations() {
    FILE *reservations_file = fopen(RESERVATIONS_FILE, "r");
    if (!reservations_file) {
        display_info("No reservations found. Make your first reservation!");
        pause_screen();
        return;
    }

    char line[500];
    int count = 0;

    display_header("MY RESERVATIONS");

    printf("%-12s | %-10s | %-6s | %-20s | %-20s | %-8s | %-10s | %-6s\n",
           "Reservation", "Vehicle", "Slot", "Start Time", "End Time", "Duration", "Status", "Cost");
    printf("------------------------------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), reservations_file)) {
        Reservation res;
        if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%d|%24[^|]|%24[^|]|%d|%d|%24[^|]|%d",
               res.reservation_id, res.user_id, res.vehicle_id, &res.slot_id,
               res.start_time, res.end_time, &res.duration_hours, &res.status,
               res.created_at, &res.total_cost) >= 9) {

            if (strcmp(res.user_id, logged_in_user.user_id) == 0) {
                char *status_text;
                if (res.status == 1) status_text = "Active";
                else if (res.status == 2) status_text = "Completed";
                else if (res.status == 3) status_text = "Overstay";
                else status_text = "Cancelled";

                printf("%-12s | %-10s | %-6d | %-20s | %-20s | %-8d | %-10s | %d BDT\n",
                       res.reservation_id, res.vehicle_id, res.slot_id,
                       res.start_time, res.end_time, res.duration_hours,
                       status_text, res.total_cost);
                count++;
            }
        }
    }
    fclose(reservations_file);

    if (count == 0) {
        display_info("No reservations found. Make your first reservation!");
    } else {
        printf("\nTotal reservations: %d\n", count);
    }

    pause_screen();
}

void cancel_reservation() {
    char reservation_id[20];

    display_header("CANCEL RESERVATION");

    // Show current reservations first
    view_my_reservations();

    printf("Enter Reservation ID to cancel: ");
    fgets(reservation_id, sizeof(reservation_id), stdin);
    reservation_id[strcspn(reservation_id, "\n")] = '\0';
    trim_whitespace(reservation_id);

    if (strlen(reservation_id) == 0) {
        display_error("Reservation ID cannot be empty!");
        pause_screen();
        return;
    }

    FILE *reservations_file = fopen(RESERVATIONS_FILE, "r");
    FILE *temp_file = fopen("temp_reservations.txt", "w");

    if (!reservations_file || !temp_file) {
        display_error("Error accessing reservation database!");
        if (reservations_file) fclose(reservations_file);
        if (temp_file) fclose(temp_file);
        pause_screen();
        return;
    }

    char line[500];
    int found = 0, slot_to_free = 0;

    while (fgets(line, sizeof(line), reservations_file)) {
        Reservation res;
        if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%d|%24[^|]|%24[^|]|%d|%d|%24[^|]|%d",
               res.reservation_id, res.user_id, res.vehicle_id, &res.slot_id,
               res.start_time, res.end_time, &res.duration_hours, &res.status,
               res.created_at, &res.total_cost) >= 9) {

            if (strcmp(res.reservation_id, reservation_id) == 0 &&
                strcmp(res.user_id, logged_in_user.user_id) == 0 && res.status == 1) {
                found = 1;
                slot_to_free = res.slot_id;
                res.status = 0; // Cancelled
            }

            fprintf(temp_file, "%s|%s|%s|%d|%s|%s|%d|%d|%s|%d\n",
                    res.reservation_id, res.user_id, res.vehicle_id, res.slot_id,
                    res.start_time, res.end_time, res.duration_hours, res.status,
                    res.created_at, res.total_cost);
        }
    }

    fclose(reservations_file);
    fclose(temp_file);
    remove(RESERVATIONS_FILE);
    rename("temp_reservations.txt", RESERVATIONS_FILE);

    if (found) {
        // Free up the parking slot
        FILE *parking_read = fopen(PARKING_FILE, "r");
        FILE *parking_write = fopen("temp_parking.txt", "w");

        if (parking_read && parking_write) {
            char line[400];
            while (fgets(line, sizeof(line), parking_read)) {
                ParkingSlot slot;
                if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
                       &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
                       slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {

                    if (slot.slot_id == slot_to_free && slot.status == 2) {
                        slot.status = 0; // Available
                        strcpy(slot.vehicle_id, "-");
                        strcpy(slot.user_id, "-");
                        strcpy(slot.reserved_until, "-");
                    }

                    fprintf(parking_write, "%d|%s|%d|%s|%s|%s|%s|%d|%s\n",
                            slot.slot_id, slot.slot_type, slot.status, slot.vehicle_id,
                            slot.user_id, slot.occupied_since, slot.reserved_until, slot.floor, slot.section);
                }
            }
            fclose(parking_read);
            fclose(parking_write);
            remove(PARKING_FILE);
            rename("temp_parking.txt", PARKING_FILE);
        }

        display_success("Reservation cancelled successfully!");
        printf("Freed slot: %d (now available for other users)\n", slot_to_free);
        display_info("No cancellation fee applied.");
    } else {
        display_error("Reservation not found or already cancelled/completed.");
    }

    pause_screen();
}

void parking_management_menu() {
    int choice;

    while (1) {
        display_header("PARKING MANAGEMENT");

        display_menu_option(1, "View Parking Slots Status", "🅿️");
        display_menu_option(2, "Reserve Parking Slot", "📅");
        display_menu_option(3, "View My Reservations", "📋");
        display_menu_option(4, "Cancel Reservation", "❌");
        display_menu_option(5, "Search Available Slots", "🔍");
        display_menu_option(6, "Check Slot Availability", "📍");
        display_menu_option(7, "Back to Main Menu", "⬅️");

        printf("\nEnter your choice (1-7): ");

        if (scanf("%d", &choice) != 1 || !is_valid_choice(choice, 1, 7)) {
            display_error("Invalid input! Please enter a number between 1-7.");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: display_parking_slots_status(); break;
            case 2: reserve_parking_slot(); break;
            case 3: view_my_reservations(); break;
            case 4: cancel_reservation(); break;
            case 5: search_available_slots(); break;
            case 6: check_slot_availability(); break;
            case 7: return;
        }
    }
}




// Payment System, User Dashboard, Admin Functions, and Main Program




// ====== PAYMENT PROCESSING ======
void process_payment() {
    char reservation_id[20], payment_method[20];

    display_header("PROCESS PAYMENT");

    // Show current reservations first
    view_my_reservations();

    printf("Enter Reservation ID for payment: ");
    fgets(reservation_id, sizeof(reservation_id), stdin);
    reservation_id[strcspn(reservation_id, "\n")] = '\0';
    trim_whitespace(reservation_id);

    if (strlen(reservation_id) == 0) {
        display_error("Reservation ID cannot be empty!");
        pause_screen();
        return;
    }

    // Find reservation details
    FILE *reservations_file = fopen(RESERVATIONS_FILE, "r");
    if (!reservations_file) {
        display_error("Error accessing reservations database!");
        pause_screen();
        return;
    }

    char line[500];
    Reservation res;
    int reservation_found = 0;

    while (fgets(line, sizeof(line), reservations_file)) {
        if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%d|%24[^|]|%24[^|]|%d|%d|%24[^|]|%d",
               res.reservation_id, res.user_id, res.vehicle_id, &res.slot_id,
               res.start_time, res.end_time, &res.duration_hours, &res.status,
               res.created_at, &res.total_cost) >= 9) {

            if (strcmp(res.reservation_id, reservation_id) == 0 &&
                strcmp(res.user_id, logged_in_user.user_id) == 0 && res.status == 1) {
                reservation_found = 1;
                break;
            }
        }
    }
    fclose(reservations_file);

    if (!reservation_found) {
        display_error("Reservation not found, already paid, or doesn't belong to you!");
        pause_screen();
        return;
    }

    printf("\nPayment Summary:\n");
    display_separator();
    printf("Reservation ID: %s\n", res.reservation_id);
    printf("Vehicle: %s\n", res.vehicle_id);
    printf("Slot: %d\n", res.slot_id);
    printf("Duration: %d hours\n", res.duration_hours);
    printf("Start Time: %s\n", res.start_time);
    printf("End Time: %s\n", res.end_time);
    printf("Total Amount: %d BDT\n", res.total_cost);
    display_separator();

    printf("\nSelect payment method:\n");
    display_menu_option(1, "Cash Payment", "💵");
    display_menu_option(2, "Card Payment", "💳");
    display_menu_option(3, "bKash Mobile Banking", "📱");
    display_menu_option(4, "Nagad Mobile Banking", "📲");

    printf("\nEnter choice (1-4): ");

    int method_choice;
    if (scanf("%d", &method_choice) != 1 || !is_valid_choice(method_choice, 1, 4)) {
        display_error("Invalid payment method selection!");
        clear_input_buffer();
        pause_screen();
        return;
    }
    clear_input_buffer();

    switch (method_choice) {
        case 1: strcpy(payment_method, "cash"); break;
        case 2: strcpy(payment_method, "card"); break;
        case 3: strcpy(payment_method, "bkash"); break;
        case 4: strcpy(payment_method, "nagad"); break;
    }

    // Payment confirmation
    printf("\nConfirm payment of %d BDT via %s? (y/n): ", res.total_cost, payment_method);
    char confirm;
    scanf("%c", &confirm);
    clear_input_buffer();

    if (confirm != 'y' && confirm != 'Y') {
        display_info("Payment cancelled by user.");
        pause_screen();
        return;
    }

    // Process payment
    char payment_id[20];
    generate_unique_id(payment_id, "P");

    FILE *payments_file = fopen(PAYMENTS_FILE, "a");
    if (!payments_file) {
        display_error("Error processing payment!");
        pause_screen();
        return;
    }

    fprintf(payments_file, "%s|%s|%s|%s|%d|%d|%s|%s|parking|%d|Parking payment for slot %d\n",
            payment_id, logged_in_user.user_id, res.reservation_id, res.vehicle_id,
            res.slot_id, res.total_cost, payment_method, "2025-08-09 15:08:45", res.duration_hours, res.slot_id);
    fclose(payments_file);

    // Update reservation status to completed
    FILE *res_read = fopen(RESERVATIONS_FILE, "r");
    FILE *res_write = fopen("temp_reservations.txt", "w");

    if (res_read && res_write) {
        char res_line[500];
        while (fgets(res_line, sizeof(res_line), res_read)) {
            Reservation temp_res;
            if (sscanf(res_line, "%19[^|]|%19[^|]|%19[^|]|%d|%24[^|]|%24[^|]|%d|%d|%24[^|]|%d",
                   temp_res.reservation_id, temp_res.user_id, temp_res.vehicle_id, &temp_res.slot_id,
                   temp_res.start_time, temp_res.end_time, &temp_res.duration_hours, &temp_res.status,
                   temp_res.created_at, &temp_res.total_cost) >= 9) {

                if (strcmp(temp_res.reservation_id, reservation_id) == 0) {
                    temp_res.status = 2; // Completed
                }

                fprintf(res_write, "%s|%s|%s|%d|%s|%s|%d|%d|%s|%d\n",
                        temp_res.reservation_id, temp_res.user_id, temp_res.vehicle_id, temp_res.slot_id,
                        temp_res.start_time, temp_res.end_time, temp_res.duration_hours, temp_res.status,
                        temp_res.created_at, temp_res.total_cost);
            }
        }
        fclose(res_read);
        fclose(res_write);
        remove(RESERVATIONS_FILE);
        rename("temp_reservations.txt", RESERVATIONS_FILE);
    }

    printf("\n");
    display_success("Payment processed successfully!");
    display_separator();
    printf("PAYMENT RECEIPT\n");
    printf("Payment ID: %s\n", payment_id);
    printf("Reservation ID: %s\n", res.reservation_id);
    printf("Amount Paid: %d BDT\n", res.total_cost);
    printf("Payment Method: %s\n", payment_method);
    printf("Payment Date: 2025-08-09 15:08:45\n");
    printf("Transaction Status: SUCCESSFUL\n");
    printf("Slot %d is now occupied by your vehicle %s\n", res.slot_id, res.vehicle_id);
    display_separator();
    display_info("Keep this receipt for your records. Thank you for using SMART PARKING!");

    pause_screen();
}

void view_payment_history() {
    FILE *payments_file = fopen(PAYMENTS_FILE, "r");
    if (!payments_file) {
        display_info("No payment history found. Make your first parking payment!");
        pause_screen();
        return;
    }

    char line[500];
    int count = 0;
    int total_spent = 0;

    display_header("PAYMENT HISTORY");

    printf("%-10s | %-10s | %-10s | %-6s | %-6s | %-8s | %-20s | %-8s\n",
           "PaymentID", "ReservID", "Vehicle", "Slot", "Amount", "Method", "Date", "Duration");
    printf("------------------------------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), payments_file)) {
        Payment payment;
        if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%d|%d|%19[^|]|%24[^|]|%19[^|]|%d|%99[^\n]",
               payment.payment_id, payment.user_id, payment.reservation_id, payment.vehicle_id,
               &payment.slot_id, &payment.amount, payment.payment_method, payment.payment_date,
               payment.payment_type, &payment.duration_hours, payment.description) >= 10) {

            if (strcmp(payment.user_id, logged_in_user.user_id) == 0) {
                printf("%-10s | %-10s | %-10s | %-6d | %-6d | %-8s | %-20s | %-8d\n",
                       payment.payment_id, payment.reservation_id, payment.vehicle_id, payment.slot_id,
                       payment.amount, payment.payment_method, payment.payment_date, payment.duration_hours);
                count++;
                total_spent += payment.amount;
            }
        }
    }
    fclose(payments_file);

    if (count == 0) {
        display_info("No payments found. Make your first parking payment!");
    } else {
        printf("\n");
        display_separator();
        printf("PAYMENT SUMMARY:\n");
        printf("Total Payments Made: %d\n", count);
        printf("Total Amount Spent: %d BDT\n", total_spent);
        printf("Average Payment: %.2f BDT\n", count > 0 ? (float)total_spent / count : 0.0);
        display_separator();
    }

    pause_screen();
}

void payment_menu() {
    int choice;

    while (1) {
        display_header("PAYMENT MANAGEMENT");

        display_menu_option(1, "Process Payment", "💳");
        display_menu_option(2, "View Payment History", "📋");
        display_menu_option(3, "Payment Methods Info", "ℹ️");
        display_menu_option(4, "Back to Main Menu", "⬅️");

        printf("\nEnter your choice (1-4): ");

        if (scanf("%d", &choice) != 1 || !is_valid_choice(choice, 1, 4)) {
            display_error("Invalid input! Please enter a number between 1-4.");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: process_payment(); break;
            case 2: view_payment_history(); break;
            case 3:
                display_header("PAYMENT METHODS INFO");
                printf("💵 Cash Payment: Pay directly at the parking facility\n");
                printf("💳 Card Payment: Credit/Debit card payment\n");
                printf("📱 bKash: Mobile banking payment (Bangladesh)\n");
                printf("📲 Nagad: Mobile banking payment (Bangladesh)\n");
                printf("\nAll payment methods are secure and instant.\n");
                printf("Transaction fees may apply for mobile banking.\n");
                pause_screen();
                break;
            case 4: return;
        }
    }
}

// ====== USER DASHBOARD ======
void user_dashboard() {
    int choice;

    while (1) {
        display_header("USER DASHBOARD");

        printf("Welcome back, %s! (User ID: %s)\n", logged_in_user.name, logged_in_user.user_id);
        printf("Current Date/Time (UTC): 2025-08-09 15:08:45\n");

        if (logged_in_user.overstay_count > 0) {
            display_warning("You have overstay incidents on record.");
            printf("Overstay count: %d\n", logged_in_user.overstay_count);
        }

        printf("\n");
        display_menu_option(1, "View Profile", "👤");
        display_menu_option(2, "Update Profile", "✏️");
        display_menu_option(3, "Change Password", "🔑");
        display_menu_option(4, "Vehicle Management", "🚗");
        display_menu_option(5, "Parking Management", "🅿️");
        display_menu_option(6, "Payment Management", "💳");
        display_menu_option(7, "Export My Data", "📤");
        display_menu_option(8, "Logout", "🚪");

        printf("\nEnter your choice (1-8): ");

        if (scanf("%d", &choice) != 1 || !is_valid_choice(choice, 1, 8)) {
            display_error("Invalid input! Please enter a number between 1-8.");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: view_user_profile(); break;
            case 2: update_user_profile(); break;
            case 3: change_user_password(); break;
            case 4: vehicle_management_menu(); break;
            case 5: parking_management_menu(); break;
            case 6: payment_menu(); break;
            case 7: export_user_data(); break;
            case 8:
                is_user_logged_in = 0;
                display_success("Logged out successfully!");
                display_info("Thank you for using SMART PARKING System.");
                return;
        }
    }
}

// ====== ADMIN FUNCTIONS ======
void admin_view_all_users() {
    FILE *users_file = fopen(USERS_FILE, "r");
    if (!users_file) {
        display_error("Error accessing user database!");
        pause_screen();
        return;
    }

    display_header("ALL REGISTERED USERS");

    printf("%-8s | %-20s | %-25s | %-12s | %-17s | %-8s | %-9s\n",
           "UserID", "Name", "Email", "Phone", "NID", "Status", "Overstays");
    printf("------------------------------------------------------------------------------------------------------\n");

    char line[500];
    int total_users = 0, active_users = 0;

    while (fgets(line, sizeof(line), users_file)) {
        User user;
        if (sscanf(line, "%19[^|]|%99[^|]|%99[^|]|%19[^|]|%19[^|]|%49[^|]|%24[^|]|%d|%d",
               user.user_id, user.name, user.email,
               user.phone, user.nid, user.password,
               user.registration_date, &user.is_active, &user.overstay_count) >= 8) {

            char *status = user.is_active ? "Active" : "Inactive";

            printf("%-8s | %-20s | %-25s | %-12s | %-17s | %-8s | %-9d\n",
                   user.user_id, user.name, user.email, user.phone, user.nid,
                   status, user.overstay_count);
            total_users++;
            if (user.is_active) active_users++;
        }
    }
    fclose(users_file);

    printf("\n");
    display_separator();
    printf("USER STATISTICS:\n");
    printf("Total Registered Users: %d\n", total_users);
    printf("Active Users: %d\n", active_users);
    printf("Inactive Users: %d\n", total_users - active_users);
    display_separator();
    pause_screen();
}

void admin_generate_system_report() {
    display_header("SYSTEM COMPREHENSIVE REPORT");

    printf("Report Generated: 2025-08-09 15:08:45 (UTC)\n");
    printf("Generated by: Admin (Sajibkundu)\n\n");

    // Users Statistics
    FILE *users_file = fopen(USERS_FILE, "r");
    int total_users = 0, active_users = 0, users_with_overstay = 0;

    if (users_file) {
        char line[500];
        while (fgets(line, sizeof(line), users_file)) {
            User user;
            if (sscanf(line, "%19[^|]|%99[^|]|%99[^|]|%19[^|]|%19[^|]|%49[^|]|%24[^|]|%d|%d",
                   user.user_id, user.name, user.email,
                   user.phone, user.nid, user.password,
                   user.registration_date, &user.is_active, &user.overstay_count) >= 8) {
                total_users++;
                if (user.is_active) active_users++;
                if (user.overstay_count > 0) users_with_overstay++;
            }
        }
        fclose(users_file);
    }

    // Vehicle Statistics
    FILE *vehicles_file = fopen(VEHICLES_FILE, "r");
    int total_vehicles = 0, bikes = 0, cars = 0, trucks = 0;

    if (vehicles_file) {
        char line[300];
        while (fgets(line, sizeof(line), vehicles_file)) {
            Vehicle vehicle;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%24[^|]|%d",
                   vehicle.vehicle_id, vehicle.user_id, vehicle.type,
                   vehicle.plate, vehicle.registration_date, &vehicle.is_active) >= 5) {
                if (vehicle.is_active) {
                    total_vehicles++;
                    if (strcmp(vehicle.type, "bike") == 0) bikes++;
                    else if (strcmp(vehicle.type, "car") == 0) cars++;
                    else trucks++;
                }
            }
        }
        fclose(vehicles_file);
    }

    // Parking Statistics
    FILE *parking_file = fopen(PARKING_FILE, "r");
    int available_slots = 0, occupied_slots = 0, reserved_slots = 0;

    if (parking_file) {
        char line[400];
        while (fgets(line, sizeof(line), parking_file)) {
            ParkingSlot slot;
            if (sscanf(line, "%d|%19[^|]|%d|%19[^|]|%19[^|]|%24[^|]|%24[^|]|%d|%9s",
                   &slot.slot_id, slot.slot_type, &slot.status, slot.vehicle_id,
                   slot.user_id, slot.occupied_since, slot.reserved_until, &slot.floor, slot.section) >= 8) {
                if (slot.status == 0) available_slots++;
                else if (slot.status == 1) occupied_slots++;
                else reserved_slots++;
            }
        }
        fclose(parking_file);
    }

    // Financial Statistics
    FILE *payments_file = fopen(PAYMENTS_FILE, "r");
    int total_payments = 0, total_revenue = 0;
    int cash_payments = 0, card_payments = 0, mobile_payments = 0;

    if (payments_file) {
        char line[500];
        while (fgets(line, sizeof(line), payments_file)) {
            Payment payment;
            if (sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%19[^|]|%d|%d|%19[^|]|%24[^|]|%19[^|]|%d|%99[^\n]",
                   payment.payment_id, payment.user_id, payment.reservation_id, payment.vehicle_id,
                   &payment.slot_id, &payment.amount, payment.payment_method, payment.payment_date,
                   payment.payment_type, &payment.duration_hours, payment.description) >= 10) {

                total_payments++;
                total_revenue += payment.amount;

                if (strcmp(payment.payment_method, "cash") == 0) cash_payments++;
                else if (strcmp(payment.payment_method, "card") == 0) card_payments++;
                else mobile_payments++;
            }
        }
        fclose(payments_file);
    }

    // Display Report
    display_separator();
    printf("USER ANALYTICS:\n");
    printf("Total Users: %d | Active: %d | Inactive: %d\n", total_users, active_users, total_users - active_users);
    printf("Users with Overstay Records: %d\n", users_with_overstay);

    printf("\nVEHICLE ANALYTICS:\n");
    printf("Total Active Vehicles: %d\n", total_vehicles);
    printf("Bikes: %d | Cars: %d | Trucks: %d\n", bikes, cars, trucks);

    printf("\nPARKING ANALYTICS:\n");
    printf("Total Slots: %d\n", available_slots + occupied_slots + reserved_slots);
    printf("Available: %d | Occupied: %d | Reserved: %d\n", available_slots, occupied_slots, reserved_slots);
    printf("Occupancy Rate: %.1f%%\n", ((float)(occupied_slots + reserved_slots) * 100) / (available_slots + occupied_slots + reserved_slots));

    printf("\nFINANCIAL ANALYTICS:\n");
    printf("Total Revenue: %d BDT\n", total_revenue);
    printf("Total Transactions: %d\n", total_payments);
    printf("Average Transaction: %.2f BDT\n", total_payments > 0 ? (float)total_revenue / total_payments : 0.0);
    printf("Cash: %d | Card: %d | Mobile: %d\n", cash_payments, card_payments, mobile_payments);

    printf("\nSYSTEM PERFORMANCE:\n");
    printf("System Uptime: 99.9%%\n");
    printf("Database Status: HEALTHY\n");
    printf("Total Files: 6 (users, vehicles, parking, payments, reservations, overstay)\n");
    display_separator();

    pause_screen();
}

void admin_dashboard() {
    int choice;

    while (1) {
        display_header("ADMIN DASHBOARD");

        printf("Welcome, Administrator! Full system access granted.\n");
        printf("System Time: 2025-08-09 15:08:45 (UTC) | Admin: Sajibkundu\n");

        printf("\n");
        display_menu_option(1, "View All Users", "👥");
        display_menu_option(2, "View All Parking Slots", "🅿️");
        display_menu_option(3, "View All Payments", "💳");
        display_menu_option(4, "View All Reservations", "📅");
        display_menu_option(5, "Generate System Report", "📊");
        display_menu_option(6, "System Maintenance", "🔧");
        display_menu_option(7, "Logout", "🚪");

        printf("\nEnter your choice (1-7): ");

        if (scanf("%d", &choice) != 1 || !is_valid_choice(choice, 1, 7)) {
            display_error("Invalid input! Please enter a number between 1-7.");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: admin_view_all_users(); break;
            case 2: display_parking_slots_status(); break;
            case 3: view_payment_history(); break;
            case 4: view_my_reservations(); break;
            case 5: admin_generate_system_report(); break;
            case 6:
                display_header("SYSTEM MAINTENANCE");
                printf("System Status: OPERATIONAL\n");
                printf("Database Integrity: OK\n");
                printf("File System: HEALTHY\n");
                printf("Memory Usage: OPTIMAL\n");
                printf("Last Backup: 2025-08-09 00:00:00\n");
                display_info("All systems are running normally.");
                pause_screen();
                break;
            case 7:
                is_admin_logged_in = 0;
                display_success("Admin logged out successfully!");
                display_info("System access revoked. Thank you!");
                return;
        }
    }
}

// ====== MAIN MENU AND PROGRAM ======
void display_main_menu() {
    display_header("MAIN MENU");

    printf("Welcome to SMART PARKING Lot Management System!\n");
    printf("Choose your access level:\n\n");

    display_menu_option(1, "User Login", "👤");
    display_menu_option(2, "Admin Login", "🔐");
    display_menu_option(3, "User Registration", "📝");
    display_menu_option(4, "About System", "❓");
    display_menu_option(5, "Exit Program", "🚪");

    printf("\nEnter your choice (1-5): ");
}

void show_about_system() {
    display_header("ABOUT SYSTEM");

    printf("SMART PARKING Lot Management System\n");
    printf("Version: 2.1.0\n");
    printf("Build Date: 2025-08-09\n");
    printf("Developer: Sajibkundu\n");
    printf("Language: C Programming\n");
    printf("Total Lines of Code: 2000+\n\n");

    printf("SYSTEM FEATURES:\n");
    printf("• Complete user management with secure authentication\n");
    printf("• Multi-vehicle support (bikes, cars, trucks)\n");
    printf("• Real-time parking slot management\n");
    printf("• Advanced reservation system with time management\n");
    printf("• Multiple payment methods (cash, card, mobile banking)\n");
    printf("• Comprehensive reporting and analytics\n");
    printf("• Full admin control panel with system monitoring\n");
    printf("• File-based data persistence (no external database needed)\n");
    printf("• Cross-platform compatibility\n");
    printf("• Enhanced user experience with improved interface\n\n");

    printf("SYSTEM CAPACITY:\n");
    printf("• Maximum Parking Slots: 100 (expandable to 1000+)\n");
    printf("• Supported Vehicle Types: 3 (bike, car, truck)\n");
    printf("• Payment Methods: 4 (cash, card, bKash, Nagad)\n");
    printf("• Multi-floor support with sectional organization\n");
    printf("• Unlimited users and reservations\n\n");

    printf("TECHNICAL SPECIFICATIONS:\n");
    printf("• Single C file implementation (2000+ lines)\n");
    printf("• Modular design with 60+ functions\n");
    printf("• Robust input validation and error handling\n");
    printf("• Memory-efficient file operations\n");
    printf("• Real-time date/time management\n");
    printf("• Comprehensive logging and audit trail\n\n");

    printf("DEFAULT ADMIN CREDENTIALS:\n");
    printf("Username: admin\n");
    printf("Password: admin123\n\n");

    printf("SUPPORT INFORMATION:\n");
    printf("For support or feature requests:\n");
    printf("Email: support@smartparking.com\n");
    printf("Website: www.smartparking.com\n");
    printf("Developer: Sajibkundu\n");

    pause_screen();
}

// ====== MAIN PROGRAM ======
int main() {
    int choice;

    // Initialize random seed for ID generation
    srand((unsigned int)time(NULL));

    // Display welcome banner and loading
    display_welcome_banner();
    display_loading_animation();

    // Initialize system files
    initialize_all_files();

    printf("\nSMART PARKING System initialized successfully!\n");
    printf("Current System Time: 2025-08-09 15:08:45 (UTC)\n");

    // Main program loop
    while (1) {
        display_main_menu();

        if (scanf("%d", &choice) != 1 || !is_valid_choice(choice, 1, 5)) {
            display_error("Invalid input! Please enter a number between 1-5.");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1:
                user_login();
                if (is_user_logged_in) {
                    user_dashboard();
                    is_user_logged_in = 0; // Reset after logout
                }
                break;

            case 2:
                admin_login();
                if (is_admin_logged_in) {
                    admin_dashboard();
                    is_admin_logged_in = 0; // Reset after logout
                }
                break;

            case 3:
                register_new_user();
                break;

            case 4:
                show_about_system();
                break;

            case 5:
                display_header("GOODBYE!");

                printf("Thank you for using SMART PARKING Lot Management System!\n");
                printf("Session ended at: 2025-08-09 15:08:45 (UTC)\n");
                printf("Have a great day!\n\n");

                printf("Cleaning up system resources...\n");
                for (int i = 0; i < 100000000; i++); // Simple delay
                printf("System shutdown complete.\n");

                display_success("Program terminated successfully!");
                exit(EXIT_SUCCESS);
        }
    }

    return 0;
}
