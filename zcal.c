/* zcal.c */
/* A pure C program to print a calendar, much like a typical POSIX cal(1) */

/*
 * Copyright (C) 2026, Alexios Zavras
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * With no arguments, it prints the current month.
 * With a -y argument, it prints the complete year.
 * With one numeric argument (e.g. 2025), it prints that year.
 * With two numeric arguments (e.g. 12 2025), it prints the specific month of the specific year.
 * Weeks always start on Mondays!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DAYS_PER_WEEK   7
#define MONTHS_PER_YEAR 12

#define MONTHS_PER_ROW  3
#define ROWS_PER_YEAR   4
#define MAX_WEEKS       6

#define VALID_MONTH(m) ((m) >= 1 && (m) <= MONTHS_PER_YEAR)
const char *month_names[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
const int   days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define MIN_YEAR 1753      // Gregorian calendar adoption
#define MAX_YEAR 9999

int
validate_year(int year)
{
    return year >= MIN_YEAR && year <= MAX_YEAR;
}

int
is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int
get_days_in_month(int month, int year)
{
    if (!VALID_MONTH(month))
	return 0;
    if (month == 2 && is_leap_year(year)) {
	return 29;
    }
    return days_in_month[month - 1];
}

// Calculate day of week (0=Monday, 1=Tuesday, ..., 6=Sunday)
// Using Zeller's congruence adapted for Monday start
int
day_of_week(int day, int month, int year)
{
    if (month < 3) {
	month += 12;
	year--;
    }
    int q = day;
    int m = month;
    int k = year % 100;
    int j = year / 100;
    int h = (q + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    // Convert to Monday = 0, handle negative modulo
    return ((h + 5) % 7 + 7) % 7;
}

void
print_month(int month, int year)
{
    printf("     %s %d\n", month_names[month - 1], year);
    printf("Mo Tu We Th Fr Sa Su\n");

    int first_day = day_of_week(1, month, year);
    int days = get_days_in_month(month, year);

    for (int i = 0; i < first_day; i++) {
	printf("   ");
    }

    for (int day = 1; day <= days; day++) {
	printf("%2d ", day);
	if ((first_day + day) % DAYS_PER_WEEK == 0) {
	    printf("\n");
	}
    }
    if ((first_day + days) % DAYS_PER_WEEK != 0) {
	printf("\n");
    }
}

void
print_year(int year)
{
    // Calculate centering (20 chars per month + 2 spaces between = 64 total)
    char year_str[16];
    snprintf(year_str, sizeof(year_str), "%d", year);
    int year_len = strlen(year_str);
    int total_width = (20 * MONTHS_PER_ROW) + (2 * (MONTHS_PER_ROW - 1));
    int padding = (total_width - year_len) / 2;
    printf("%*s%d\n\n", padding, "", year);


    for (int row = 0; row < ROWS_PER_YEAR; row++) {
	// Print month names
	for (int col = 0; col < MONTHS_PER_ROW; col++) {
	    int m = row * MONTHS_PER_ROW + col + 1;
#define	print_centered(s, n)	printf("%*s%*s", (n+strlen(s))/2, s, (n-strlen(s))/2, "")
	    print_centered(month_names[m - 1], 20);
	    if (col < MONTHS_PER_ROW - 1)
		printf("    ");
	}
	printf("\n");

	// Print day headers
	for (int col = 0; col < MONTHS_PER_ROW; col++) {
	    printf("Mo Tu We Th Fr Sa Su");
	    if (col < MONTHS_PER_ROW - 1)
		printf("   ");
	}
	printf("\n");

	// Print days for all three months side by side
	int first_days[MONTHS_PER_ROW];
	int total_days[MONTHS_PER_ROW];

	for (int col = 0; col < MONTHS_PER_ROW; col++) {
	    int m = row * MONTHS_PER_ROW + col + 1;
	    first_days[col] = day_of_week(1, m, year);
	    total_days[col] = get_days_in_month(m, year);
	}

	// Print up to 6 weeks
	for (int week = 0; week < MAX_WEEKS; week++) {
	    int any_output = 0;
	    for (int col = 0; col < MONTHS_PER_ROW; col++) {
		for (int dow = 0; dow < DAYS_PER_WEEK; dow++) {
		    int day_num = week * DAYS_PER_WEEK + dow - first_days[col] + 1;
		    if (day_num > 0 && day_num <= total_days[col]) {
			printf("%2d ", day_num);
			any_output = 1;
		    } else {
			printf("   ");
		    }
		}
		if (col < MONTHS_PER_ROW - 1)
		    printf("  ");
	    }
	    if (any_output) {
		printf("\n");
	    }
	}
	printf("\n");
    }
}

void
print_usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] [args]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  (no args)        Display current month\n");
    fprintf(stderr, "  -y               Display current year\n");
    fprintf(stderr, "  YEAR             Display entire year\n");
    fprintf(stderr, "  MONTH YEAR       Display specific month (order flexible)\n");
    fprintf(stderr, "  YEAR MONTH       Display specific month (order flexible)\n");
    fprintf(stderr, "  -h, --help       Display this help\n");
}

int
main(int argc, char **argv)
{
    time_t     now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    if (!timeinfo) {
	fprintf(stderr, "Error getting current time\n");
	return 1;
    }

    int current_year = timeinfo->tm_year + 1900;
    int current_month = timeinfo->tm_mon + 1;

    if (argc == 1) {
	// No arguments: print current month
	print_month(current_month, current_year);
    } else if (argc == 2) {
	if (strcmp(argv[1], "-y") == 0) {
	    // -y flag: print current year
	    print_year(current_year);
	} else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
	    // -h flag: print help and exit
	    print_usage(argv[0]);
	    return 0;
	} else {
	    // One number: print that year
	    char *endptr;
	    long  year = strtol(argv[1], &endptr, 10);
	    if (*endptr != '\0' || year < MIN_YEAR || year > MAX_YEAR) {
		fprintf(stderr, "Invalid year: %s\n", argv[1]);
		return 1;
	    }
	    print_year(year);
	}
    } else if (argc == 3) {
	// Two numbers: month and year
	char *endptr;
	long  arg1 = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
	    fprintf(stderr, "Invalid argument: %s\n", argv[1]);
	    return 1;
	}
	long arg2 = strtol(argv[2], &endptr, 10);
	if (*endptr != '\0') {
	    fprintf(stderr, "Invalid argument: %s\n", argv[2]);
	    return 1;
	}

	// Detect which is month and which is year
	int month, year;
	int arg1_valid_month = VALID_MONTH(arg1);
	int arg2_valid_month = VALID_MONTH(arg2);
	int arg1_valid_year = validate_year(arg1);
	int arg2_valid_year = validate_year(arg2);

	if (arg1_valid_month && arg2_valid_year) {
	    // Traditional order: MONTH YEAR
	    month = arg1;
	    year = arg2;
	} else if (arg1_valid_year && arg2_valid_month) {
	    // Swapped order: YEAR MONTH
	    month = arg2;
	    year = arg1;
	} else {
	    fprintf(stderr, "Invalid month/year combination: %ld %ld\n", arg1, arg2);
	    fprintf(stderr, "Expected: MONTH YEAR or YEAR MONTH\n");
	    return 1;
	}

	print_month(month, year);
    } else {
	print_usage(argv[0]);
	return 1;
    }

    return 0;
}

