#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "validation.h"

int validate_email(const char *email) {
    const char *at = strchr(email, '@');
    if (!at) return 0;
    const char *dot = strchr(at, '.');
    if (!dot || dot < at) return 0;
    for (const char *p = email; *p; ++p) {
        if (isspace((unsigned char)*p)) return 0;
    }
    return 1;
}

int validate_phone(const char *phone) {
    if (strlen(phone) != 11) return 0;
    if (phone[0] != '0' || phone[1] != '1') return 0;
    for (int i = 0; i < 11; ++i) {
        if (!isdigit((unsigned char)phone[i])) return 0;
    }
    return 1;
}

int validate_nid(const char *nid) {
    size_t len = strlen(nid);
    if (len != 10 && len != 17) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)nid[i])) return 0;
    }
    return 1;
}

int validate_datetime(const char *dt) {
    int year, month, day, hour, min;
    if (sscanf(dt, "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &min) != 5)
        return 0;
    if (year < 2020 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || min < 0 || min > 59)
        return 0;
    return 1;
}

int validate_plate(const char *plate) {
    int len = strlen(plate);
    if (len < 5 || len > 15) return 0;
    int has_letter = 0, has_digit = 0;
    for (int i = 0; i < len; ++i) {
        if (isalpha((unsigned char)plate[i])) has_letter = 1;
        if (isdigit((unsigned char)plate[i])) has_digit = 1;
    }
    return has_letter && has_digit;
}

int validate_password(const char *pw) {
    int len = strlen(pw);
    if (len < 6) return 0;
    int has_digit = 0;
    for (int i = 0; i < len; ++i) {
        if (isdigit((unsigned char)pw[i])) has_digit = 1;
    }
    return has_digit;
}
