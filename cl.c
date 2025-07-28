#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("\033[J2\033[H");
    int result = system("clear");
    if (result == -1) {
        perror("system call failed");
        return 1;
    } else {
        if (WIFEXITED(result) && WEXITSTATUS(result) != 0) {
            fprintf(stderr, "The 'clear' command failed with status: %d\n", WEXITSTATUS(result));
            return 1;
        }
    }
return 0;
}
