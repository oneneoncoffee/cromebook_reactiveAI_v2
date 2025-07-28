#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h> 

#define MAX_CMDLINE_LENGTH 50 // Maximum length for process name display
#define SLEEP_DURATION 500000  // Sleep duration in microseconds (500ms)

// Color escape codes
#define COLOR_RESET "\033[0m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_WHITE "\033[37m"

void display_memory_usage() {
    FILE *fp;
    char line[256];
    long total_memory = 0;
    long free_memory = 0;

    // Open the /proc/meminfo file
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    // Read the memory information
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %ld kB", &total_memory) == 1) {
            // Found total memory
        } else if (sscanf(line, "MemFree: %ld kB", &free_memory) == 1) {
            // Found free memory
        }
    }
    fclose(fp);

    // Calculate used memory
    long used_memory = total_memory - free_memory;
    long used_percentage = (used_memory * 100) / total_memory;

    // Display memory usage
    printf("Memory Usage: [%s", COLOR_YELLOW);
    int bar_length = 50; // Length of the visual aid
    int used_length = (used_percentage * bar_length) / 100;

    for (int i = 0; i < used_length; i++) {
        printf("%s#", COLOR_RED);
    }
    for (int i = used_length; i < bar_length; i++) {
        printf(" ");
    }
    printf("%s] %s%ld%% \n", COLOR_WHITE , COLOR_YELLOW, used_percentage);
    printf(COLOR_RESET); // Reset color
}

int main() {
    struct dirent *entry;
    DIR *dp;
    int process_count = 0;

    // Open the /proc directory
    dp = opendir("/proc");
    if (dp == NULL) {
        perror("opendir");
        return 1;
    }

    printf("%sPID\tProcess Name\n", COLOR_WHITE);
    printf("---------------------\n");

    // Read entries in the /proc directory
    while ((entry = readdir(dp)) != NULL) {
        // Check if the entry is a directory and is a number (process ID)
        if (isdigit(entry->d_name[0])) {
            char path[256];
            char cmdline[256];
            FILE *fp;

            // Construct the path to the cmdline file
            snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);

            // Open the cmdline file to read the process name
            fp = fopen(path, "r");
            if (fp != NULL) {
                // Read the command line (process name)
                if (fgets(cmdline, sizeof(cmdline), fp) != NULL) {
                    // Replace null characters with spaces for better readability
                    for (int i = 0; i < sizeof(cmdline); i++) {
                        if (cmdline[i] == '\0') {
                            cmdline[i] = ' ';
                        }
                    }

                    // Truncate the command line to fit within MAX_CMDLINE_LENGTH
                    char truncated_cmdline[MAX_CMDLINE_LENGTH + 1];
                    snprintf(truncated_cmdline, sizeof(truncated_cmdline), "%.*s", MAX_CMDLINE_LENGTH, cmdline);

                    // Print the PID and truncated command line
                    printf("%s%s\t%s\n", COLOR_GREEN, entry->d_name, truncated_cmdline);
                    process_count++;  // Increment the process count

                    // Sleep for a short duration to slow down the output
                    usleep(SLEEP_DURATION);
                }
                fclose(fp);
            }
        }
    }

    // Close the directory
    closedir(dp);

    // Print the total number of processes
    printf("%s---------------------\n", COLOR_WHITE);
    printf("Total number of processes: %s%d\n", COLOR_WHITE, process_count);

    // Display memory usage
    display_memory_usage();

    return 0;
}
