#include <stdio.h>
#include <stdlib.h> // For strtod()
#include <unistd.h> // For usleep()
#include <string.h> // For strlen()

// Function to get user input for delay
double get_delay_input() {
    char input[20]; // Buffer to hold user input
    double seconds;

    while (1) {
        printf("Enter delay in seconds (can be a fraction, e.g., 0.25) or type 'skip' to skip the delay: ");
        fgets(input, sizeof(input), stdin); // Read user input

        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;

        // Check if the user wants to skip the delay
        if (strcmp(input, "skip") == 0) {
            return 0.0; // Return 0.0 to indicate no delay
        }

        // Check if the input is a valid number
        if (strlen(input) == 0) {
            printf("Input cannot be empty. Please enter a number or 'skip'.\n");
            continue;
        }

        // Convert input to double and check for errors
        char *endptr;
        seconds = strtod(input, &endptr);

        // Check if the entire input was converted and if it's a valid positive number
        if (*endptr != '\0' || endptr == input || seconds < 0) {
            printf("Invalid input. Please enter a valid positive number or 'skip'.\n");
            continue;
        }

        // If we reach here, the input is valid
        return seconds; // Return the valid input
    }
}

// Function to perform the delay
void perform_delay(double seconds) {
    if (seconds > 0) {
        printf("Delaying for %.2f seconds...\n", seconds);
        usleep((unsigned int)(seconds * 1000000)); // Delay for the specified number of microseconds
        printf("Delay complete!\n");
    } else {
        printf("No delay will be applied.\n");
    }
}

int main() {
    // Get user input for delay
    double delay = get_delay_input();

    // Call the delay function
    perform_delay(delay);

    return 0; // Exit successfully
}
