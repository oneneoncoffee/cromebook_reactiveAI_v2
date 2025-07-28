#include <stdio.h>
#include <sys/sysinfo.h>

#define RED "\033[31m"    // Red color
#define BLUE "\033[34m"   // Blue color
#define RESET "\033[0m"   // Reset to default color

int main() {
    struct sysinfo info;

    if (sysinfo(&info) == 0) {
        long uptime_seconds = info.uptime;
        long hours = uptime_seconds / 3600;
        long minutes = (uptime_seconds % 3600) / 60;
        long seconds = uptime_seconds % 60;
        printf(RED);
        printf("Uptime:");
        printf(RESET);
        printf(BLUE);
        printf(" %ld hours, %ld minutes, %ld seconds\n", hours, minutes, seconds);
        printf(RESET);
        printf("Total RAM: %ld bytes / ", info.totalram);
        printf("Free RAM: %ld bytes\n", info.freeram);
        printf(RED);
        printf("Number of processes: %d\n", info.procs);
        printf(RESET);
    } else {
        perror("sysinfo");
    }

    return 0;
}
