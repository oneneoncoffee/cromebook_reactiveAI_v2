#include <stdio.h>
#include <sys/sysinfo.h>

#define BAR_WIDTH 50  // Width of the bar graph

void print_bar_graph(double percentage, const char* label) {
    int bar_length = (int)(percentage / 100 * BAR_WIDTH);
    printf("%s:\033[33m [\033[0m", label);
    for (int i = 0; i < bar_length; i++) {
        printf("\033[31m#\033[0m");
    }
    for (int i = bar_length; i < BAR_WIDTH; i++) {
        printf(" ");
    }
    printf("\033[33m ]\033[0m %.2f%%\n", percentage);
}

void print_memory_stats() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return;
    }

    long total_memory = info.totalram;
    long free_memory = info.freeram;
    long used_memory = total_memory - free_memory;

    double used_percentage = (double)used_memory / total_memory * 100;
    double free_percentage = (double)free_memory / total_memory * 100;

    // Print memory statistics
    printf("Memory Statistics:\n");
    printf("\033[33mTotal Memory:\033[34m%ld MB\033[0m ", total_memory / (1024 * 1024));
    printf("\033[33mFree Memory:\033[34m%ld MB\033[0m ", free_memory / (1024 * 1024));
    printf("\033[33mUsed Memory:\033[34m%ld MB\033[0m\n", used_memory / (1024 * 1024));
    // Print bar graphs for used and free memory
    print_bar_graph(used_percentage, "Used Memory");
    print_bar_graph(free_percentage, "Free Memory");
}

int main() {
    print_memory_stats();
    return 0;
}
