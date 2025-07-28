#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

void countdown(int microseconds){
for (int i = microseconds; i > 0; i--) {
printf("\r\033[32mTime remaining:\033[31m [%d] seconds \033[0m", i);
fflush(stdout);
sleep(1);
usleep(microseconds);
}
printf("\r\n");
}

void disable_input_echo() {
    struct termios new_settings;
    tcgetattr(0, &new_settings);
    new_settings.c_lflag &= ~ECHO; // Disable echo
    tcsetattr(0, TCSANOW, &new_settings);
}

void enable_input_echo() {
    struct termios new_settings;
    tcgetattr(0, &new_settings);
    new_settings.c_lflag |= ECHO; // Enable echo
    tcsetattr(0, TCSANOW, &new_settings);
}

int main(int argc, char *argv[], int delay) {
delay = 1;
if (argc > 1) {
delay = atoi(argv[1]);
int delay_in_microseconds = delay * delay;
disable_input_echo();
countdown(delay_in_microseconds);
enable_input_echo();
return EXIT_SUCCESS;
} else {
int delay_in_microseconds = delay * 100;
disable_input_echo();
countdown(delay_in_microseconds);
enable_input_echo();
return EXIT_SUCCESS;
}
return 0;
}

