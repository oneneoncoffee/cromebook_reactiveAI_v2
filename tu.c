/* Project: tu.c the OZ second draft 
 * To make project: gcc oz.c -o oz -lm 
 * Reactive AI self learning via a text file. Possibly can be inproved in version 3 of this source code. 
 * Copyright 2025 oneneon <oneneon@tutanota.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <math.h> 

#define MAX_KEYWORDS 25000
#define MAX_RESPONSE_LENGTH 256
#define MAX_INPUT_LENGTH 256
#define MAX_LINE_LENGTH 512
#define FILENAME "responses"
#define MAX_NAME_LENGTH 100
#define MAX_VARIATIONS 12

// Function prototypes
void respondToInput(const char *input);
void saveNewKeyword(const char *keyword, const char *response);
void randomInsult();
void randomPositiveResponse(const char *keyword);
void systemAlert(int signum);
void loadKeywordsFromFile();
void addKeywordToList(const char *keyword, const char *response);
void sclear();
unsigned int custom_seeds1(); 
void save_info(const char *name, const char *response); 
int load_info(char *name, char *response);
int is_who_am_i(const char *input, const char *variations[], int size);
int is_what_is_my_name(const char *input);
// Global variables for keywords and responses
char keywords[MAX_KEYWORDS][MAX_INPUT_LENGTH];
char responses[MAX_KEYWORDS][MAX_RESPONSE_LENGTH];
int keywordCount = 0;

// Variations of "Who am I?"
const char  *what_is_my_name_variations[MAX_VARIATIONS] = {
        "Who am I?", "who am I?", "Who Am I?",
        "whoami", "Who am I", "what is my name?",
        "what is my name", "what was my name?",
        "What is my name?", "What was my name?",
        "What was my name again?", "what was my name again?"
};
const char *badWords[] = {
	"bad", "awful", "terrible", "hate", "fucker", 
	"bitch", "cunt", "retard", "mother fucker", "fuckers",
	"jew", "stupid"
	};
const char *insults[] = {
    "You're as useless as a screen door on a submarine.",
    "I'd agree with you, but then we'd both be wrong.",
    "You're like a cloud. When you disappear, it's a beautiful day.",
    "WTF? I am just an AI after all don't put the blame on me!", 
    "I don't get humans sometimes. Just take a chill pill",
    "Woops, what was I thinking man...",
    "I’d explain it to you, but I left my English-to-Dingbat dictionary at home.",
    "You're like a software update. Whenever I see you, I think, 'Not now.'",
    "I’d agree with you, but then we’d both be wrong.",
    "I’m not saying you’re forgettable, but I can’t remember your name.",
    "You have the perfect face for radio."   
};
const char *positiveKeywords[] = {
	"good", "great", "awesome", "fantastic", "joy", "happy",
	"wonderful", "amazing", "incredible", "marvelous",
    "remarkable", "extraordinary", "outstanding",
     "fabulous", "stupendous", "terrific", "splendid",
     "exquisite", "brilliant", "phenomenal", "delightful",
     "breathtaking", "enchanting", "magnificent", "superb"
	};
const char *positiveResponses[] = {
    "That's fantastic! Keep it up!",
    "Great to hear that! You're doing amazing!",
    "Awesome! Your positivity is contagious!", 
    "that's good infact great.", 
    "Joy to the word..Hump..this AI can't hold a tune.",
    "I really appreciate your kind words!",
    "You’re doing an amazing job! Keep it up!",
    "I believe in you! You’ve got this!",
    "Absolutely! I couldn’t agree more.",
};

int main() {
    char input[MAX_INPUT_LENGTH];
    char name[MAX_NAME_LENGTH];
    char response[MAX_RESPONSE_LENGTH];

    // clear screen
    sclear();
    // Seed random number generator
    srand(time(NULL));
    // Load keywords and responses from file
    loadKeywordsFromFile();
    // Set up signal handler for system alerts
    signal(SIGALRM, systemAlert);

    // Start the interaction loop
    while (1) {
        // Set an alarm for 18 seconds
        alarm(18);
        printf("\n>> ");
        fgets(input, MAX_INPUT_LENGTH, stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character

        // Cancel the alarm if input is received
        alarm(0);
        // check if we are exiting and check if command starts with "run "
        if (strncmp(input, "run ", 4) == 0) {
			//extract the actual command 
			char *actual_command = input + 4;
			int result = system(actual_command);
			if (result == -1) {
			perror("\nError executing command\n");
			}
		} else { 

	        if (strcmp(input, "exit") && strcmp(input, "quit") == 0) {
            break;
            } 
            if (is_what_is_my_name(input)) {
                printf("Please enter your name?\n>> ");
                fgets(name, MAX_NAME_LENGTH, stdin);
                name[strcspn(name, "\n")] = 0; // Remove newline character

                // Check if the name already exists in the file
                if (load_info(name, response)) {
                    printf("\nWelcome back, %s!\n%s\n", name, response);
                } else {
                    printf("\nPlease enter a response associated with your name: ");
                    fgets(response, MAX_RESPONSE_LENGTH, stdin);
                    response[strcspn(response, "\n")] = 0; // Remove newline character
                    save_info(name, response);
                    printf("\nYour name and response have been saved.\n");
                }              
            } 
		}
		            
        respondToInput(input);
    }

    return 0;
}


void randomInsult() {
    int index = rand() % (sizeof(insults) / sizeof(insults[0]));
    printf("%s\n", insults[index]);
}

void randomPositiveResponse(const char *keyword) {
    int index = rand() % (sizeof(positiveResponses) / sizeof(positiveResponses[0]));
    printf("%s\n", positiveResponses[index]);
}

void saveNewKeyword(const char *keyword, const char *response) {
    FILE *file = fopen("responses", "a+");
    if (file != NULL) {
        fprintf(file, "%s: %s\n", keyword, response);
        fclose(file);
        printf("Saved new keyword: %s with response: %s\n", keyword, response);
        addKeywordToList(keyword, response);
    } else {
        printf("Error saving keyword.\n");
    }
}


void respondToInput(const char *input) {
    // Check for bad words
    for (int i = 0; i < sizeof(badWords) / sizeof(badWords[0]); i++) {
        if (strstr(input, badWords[i]) != NULL) {
            randomInsult();
            return;
        }
    }

    // Check for positive keywords
    for (int i = 0; i < sizeof(positiveKeywords) / sizeof(positiveKeywords[0]); i++) {
        if (strstr(input, positiveKeywords[i]) != NULL) {
            randomPositiveResponse(positiveKeywords[i]);
            return;
        }
    }

    // Check if the keyword already exists in the loaded keywords
    for (int i = 0; i < keywordCount; i++) {
        if (strcmp(input, keywords[i]) == 0) {
			//printf("Details about inquiry: %s\n", input); 
            printf("%s\n", responses[i]);
            return;
        }
    }

    // If no keywords match, prompt the user for a response
    char userResponse[MAX_RESPONSE_LENGTH];
    printf("Help me out,\n");
    printf("I don't recognize that so What response should I save for '%s'?\n", input);
    printf("Your response: ");
    fgets(userResponse, MAX_RESPONSE_LENGTH, stdin);
    userResponse[strcspn(userResponse, "\n")] = 0; // Remove newline character

    // Save the new keyword and user-provided response
    saveNewKeyword(input, userResponse);
}

void loadKeywordsFromFile() {
    FILE *file = fopen("responses", "r+");
    if (file != NULL) {
        while (fgets(keywords[keywordCount], MAX_LINE_LENGTH, file) && keywordCount < MAX_KEYWORDS) {
            // Split the line into keyword and response
            char *token = strtok(keywords[keywordCount], ":");
            if (token != NULL) {
                strcpy(keywords[keywordCount], token);
                token = strtok(NULL, "\n");
                if (token != NULL) {
                    strcpy(responses[keywordCount], token);
                    keywordCount++;
                }
            }
        }
        fclose(file);
    } else {
        printf("Error loading keywords from file.\n");
    }
}


void addKeywordToList(const char *keyword, const char *response) {
    if (keywordCount < MAX_KEYWORDS) {
        strcpy(keywords[keywordCount], keyword);
        strcpy(responses[keywordCount], response);
        keywordCount++;
    }
}

unsigned int custom_seeds1() {
return (unsigned int)(time(NULL) ^ clock());
}

void systemAlert(int signum) {
	srand(custom_seeds1());
    const char *greetings[] = {
        "\nHello! How can I assist you today?\n>> ",
        "\nHi there! Don't forget to share your thoughts!\n>> ",
        "\nGreetings! I'm here to help you.\n>> ",
        "\nHey! What's on your mind? \n>> ",
        "\nOkay, greeting space cowboay? chat with me!?!? \n>> ",
        "\nHowdy space case? \n>>", "\nWhat ya' do in? \n>>",
        "\nBonjour what's up ? \n>>", "\nHello! So how can I assist you? \n>>",
        "\nKonnichiwa how's life? \n>>", "\nHow's everything? \n>>" 
    };
    int index = rand() % (sizeof(greetings) / sizeof(greetings[0]));
    printf("%s\n", greetings[index]);
}

void sclear() {
     // Clear the screen using ANSI escape code
     printf("\033[H\033[J");
}

void save_info(const char *name, const char *response) {
    FILE *file = fopen(FILENAME, "a+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s:%s\n", name, response);
    fclose(file);
}

int load_info(char *name, char *response) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return 0; // No user found
    }

    while (fscanf(file, "%[^:]:%[^\n]\n", name, response) != EOF) {
        if (strlen(name) > 0) {
            fclose(file);
            return 1; // User found
        }
    }

    fclose(file);
    return 0; // No user found
}


int is_what_is_my_name(const char *input) {
    for (int i = 0; i < MAX_VARIATIONS; i++) {
        if (strcasecmp(input, what_is_my_name_variations[i]) == 0) {
            return 1; // Match found
        }
    }
    return 0; // No match
}
