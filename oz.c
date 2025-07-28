/*
 * Project: OZ Reactive AI
 * File Name: oz.c
 * 
 * How do I install Ubuntu C extended librarys: 
 * sudo apt-get install libjson-c-dev 
 * sudo apt-get install libcurl4-openssl-dev
 * 
 * Copyright 2025 oneneon <oneneon@tutanota.com>
 * Program by Johnny Buckallew Stroud 
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
 * Notes: 
 * Stable version I guess of OZ reactive AI. Moreover Reactive AIs are often considered primitive 
 * compared to more advanced forms of artificial intelligence. Reactive AI systems operate based on predefined rules and do not 
 * possess memory or the ability to learn from past experiences. They respond to specific inputs with predetermined outputs, 
 * making them effective for simple tasks but limited in their capabilities. For example, a reactive AI might be used in a game 
 * to control a character's movements based on the player's actions, but it wouldn't be able to adapt or improve its strategies over time. 
 * In contrast, more advanced AI systems, such as those based on machine learning, can learn from data, adapt to new situations, and make 
 * decisions based on past experiences. Be it good or bad? 
 * 
 * The capability of Reactive AIs to search the internet in real-time for relevant content is a powerful tool, but I believe it's the ability 
 * to generate abstract, innovative ideas that truly holds the key to unlocking their potential. By leveraging abstract generative content, 
 * Reactive AIs can create novel solutions, explore new concepts, and push the boundaries of what's possible. This overlooked aspect of Reactive AIs 
 * has the potential to revolutionize fields such as art, writing, design, and more, and it's an area that warrants further exploration and 
 * development.
 *  
 * 
 * 
 */

// include librarys and header information 
#define _POSIX_C_SOURCE 200809L  // POSIX standard
#include <stdio.h>
#include <math.h>
#include <time.h> 
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <sqlite3.h> 
#include <ctype.h>

// Explicit declaration of strdup 
extern char *strdup(const char *s);

// AI core attributes 
#define MAX_LINE_LENGTH 1256
#define MAX_KEYWORD_LENGTH 128
#define MAX_RESPONSE_LENGTH 128
#define FILENAME "responses"
#define MAX_INPUT_LENGTH 100
#define MAX_PHRASES 7
#define MAX_JOKES 100
#define MAX_JOKE_LENGTH 256
#define MAX_ALARMS 5
#define MAX_WORD_LENGTH 100
#define MAX_BIWORDS 100
#define MAX_GREETINGS 100
#define MATH_INPUT_LENGTH 128  
#define BOTMAX_RESPONSE_LENGTH 65536

// Callback function to retrieve response
int callback(void *data, int argc, char **argv, char **azColName) {
    char **response = (char **)data;
    if (argc > 0) {
        *response = strdup(argv[0]); // Store response
    }
    return 0;
}

// Function to query database and get response
char* query_database(sqlite3 *db, const char *input) {
    char *errMsg = 0;
    char *response = NULL;
    char sql[MAX_INPUT * 2];

    // Prepare SQL query
    snprintf(sql, sizeof(sql), 
     "SELECT BotResponse FROM Responses WHERE UserInput = '%s' LIMIT 1;", input); 
      
    // Execute query
    int rc = sqlite3_exec(db, sql, callback, &response, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL error %s\n", errMsg);
        sqlite3_free(errMsg);
        return NULL;
    }

    return response;
}

// Main database interaction function
void process_user_interaction() {
    sqlite3 *db;
    char input[MAX_INPUT];

    // Open database
    int rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "OZ: Can't open database %s\n", sqlite3_errmsg(db));
        return;
    }

    // Get user input fresh 2nd pass
    printf(">> ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "OZ: Error reading input\n");
        sqlite3_close(db);
        return;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Query database
    char *response = query_database(db, input);
    
    // Display response
    if (response) {
        printf("OZ: %s\n", response);
        free(response);
    } else {
      const char *messages[] = {
      "My name is OZ I am A reactive AI program..",
      "stay awesome!", 
      "keep clam and carry on coding!",
      "coffee is on the way!",  
      "believe in yourself more", 
      "embrace the challenge more", 
      "learning is a journey, just like the band", 
      "Hello, world!", 
      "embrace the total suck", 
      "I hope your day is great", 
      "I know i can search the internet for things just ask me.", 
      "I can search the internet?", 
      "Well that was random!", 
      "I can also to math like 100 + 10000 or something", 
      "Why did the lucky penny go to therapy? Because it was feeling a bit down and needed some change!",
      "What do you call a lucky rabbit who's also a comedian? A hare-larious good luck charm!", 
      "How does a lucky four-leaf clover tell a joke? With a lot of Irish wit! thanks 4chan..",
      "Why did the gambler bring a ladder to the casino? He wanted to take his luck to new heights!",
      "What did one lucky horseshoe say to the other? We're really hanging in there!", 
      "Why was the lucky cat always smiling? Because everything was purr-fect!"
                       };
                   // calculate the number of messages 
                   int nummessages = sizeof(messages) / sizeof(messages[0]); 
                  // seed the sudo random number generator 
                  srand(9889+time(NULL)^time(NULL)*996); 
                 // generate A random index 
                  int randomIndex = rand() % nummessages;
                  printf("OZ: %s\n", messages[randomIndex]); 
     
     
    }

    // Close database
    sqlite3_close(db);
}

struct MemoryStruct {
    char *memory;
    size_t size;
};
// baseline static memory size_t callback function  
//static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    //size_t realsize = size * nmemb;
    //struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    //char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    //if (!ptr) {
        //printf("Memory allocation failed\n");
        //return 0;
    //}
    //mem->memory = ptr;
    //memcpy(&(mem->memory[mem->size]), contents, realsize);
    //mem->size += realsize;
    //mem->memory[mem->size] = 0;

    //return realsize;
//}

// Function to check internet connectivity
int is_internet_connected() {
    // This is a simple example using system command
    // Works on cromebook and most Unix-like systems
    int result = system("ping -c 4 8.8.8.8 > /dev/null 2>&1");
    
    // If ping is successful, result will be 0
    return (result == 0);
}

// Function to strip html tags 
void strip_html_tags(char *str) {
    char *src = str, *dst = str;
    int inside_tag = 0;

    while (*src) {
        if (*src == '<') {
            inside_tag = 1;
            src++;
            continue;
        }
        
        if (*src == '>') {
            inside_tag = 0;
            src++;
            continue;
        }
        
        if (!inside_tag) {
            *dst++ = *src;
        }
        src++;
    }
    
    *dst = '\0';
}

void decode_html_entities(char *str) {
    struct {
        const char *entity;
        const char *replacement;
    } entities[] = {
        {"&amp;", "&"},
        {"&lt;", "<"},
        {"&gt;", ">"},
        {"&quot;", "\""},
        {"&#39;", "'"},
        {"&nbsp;", " "}
    };

    for (int i = 0; i < sizeof(entities) / sizeof(entities[0]); i++) {
        char *pos;
        while ((pos = strstr(str, entities[i].entity)) != NULL) {
            memmove(pos + strlen(entities[i].replacement), 
                    pos + strlen(entities[i].entity), 
                    strlen(pos + strlen(entities[i].entity)) + 1);
            memcpy(pos, entities[i].replacement, strlen(entities[i].replacement));
        }
    }
}

void extract_search_result(char *response) {
    if (!response) {
        printf("OZ: No response received.\n");
        return;
    }

    // Remove HTML tags
    strip_html_tags(response);

    // Decode HTML entities
    decode_html_entities(response);

    // Trim and limit output
    char *start = response;
    char *end = response + strlen(response) - 1;

    while (*start && (*start == ' ' || *start == '\n' || *start == '\r')) start++;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = '\0';

    // Limit output length
    if (strlen(start) > 45000) {
        start[45000] = '\0';
        strcat(start, "...");
    }

  //  printf("Search Result:\n%s\n", start);
    
        // Pointers for parsing
    char *abstract_start = NULL;
    char *abstract_end = NULL;

    // Find AbstractText
    abstract_start = strstr(response, "\"AbstractText\":\"");
    if (!abstract_start) {
        printf("OZ: No description found.\n");
        return;
    }

    // Move past "AbstractText":"
    abstract_start += strlen("\"AbstractText\":\"");
    
    // Find end of abstract
    abstract_end = abstract_start;
    int quote_depth = 0;
    
    while (*abstract_end) {
        if (*abstract_end == '\\' && *(abstract_end + 1) == '"') {
            // Skip escaped quotes
            abstract_end += 2;
        } else if (*abstract_end == '"' && quote_depth == 0) {
            // End of abstract
            break;
        }
        abstract_end++;
    }

    // Terminate the string
    *abstract_end = '\0';

    // Decode escape sequences
    char *read = abstract_start;
    char *write = abstract_start;

    while (*read) {
        if (*read == '\\') {
            read++;
            switch (*read) {
                case 'n': *write++ = ' '; break;  // Newline to space
                case 't': *write++ = ' '; break;  // Tab to space
                case 'r': *write++ = ' '; break;  // Carriage return to space
                case '"': *write++ = '\''; break; // Escaped quote to single quote
                case '\\': *write++ = '\\'; break;
                default: *write++ = *read; break;
            }
        } else {
            *write++ = *read;
        }
        read++;
    }
    *write = '\0';  // Null terminate

    // Print result
    printf("\nOZ: %s\n", abstract_start);

    // Optional: Extract additional information
    char *source_start = strstr(response, "\"AbstractSource\":\"");
    if (source_start) {
        source_start += strlen("\"AbstractSource\":\"");
        char *source_end = strchr(source_start, '"');
        if (source_end) {
            *source_end = '\0';
            //printf("%s\n", source_start);
        }
    }
}


// Function to check if the input is a valid math expression
int is_math_expression(const char *keyword) {
    int has_number = 0;
    int has_operator = 0;

    for (int i = 0; keyword[i] != '\0'; i++) {
        if (isdigit(keyword[i]) || (keyword[i] == '-' && isdigit(keyword[i + 1]))) {
            has_number = 1;
        } else if (strchr("+-*/", keyword[i])) {
            has_operator = 1;
        }
    }

    return has_number && has_operator;
}

// Function to create tables
static int create_tables(sqlite3 *db) {
    char *errMsg = NULL;
    int rc;

    // Create table jokes
    const char *sql_jokes = "CREATE TABLE IF NOT EXISTS jokes(id INTEGER PRIMARY KEY AUTOINCREMENT, joke TEXT NOT NULL);";
    rc = sqlite3_exec(db, sql_jokes, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL error %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    // Create table stories
    const char *sql_stories = "CREATE TABLE IF NOT EXISTS stories(id INTEGER PRIMARY KEY AUTOINCREMENT, story TEXT NOT NULL);";
    rc = sqlite3_exec(db, sql_stories, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL error %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return rc;
}

// Function to insert joke into jokes table
static int insert_joke(sqlite3 *db, const char *joke) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO jokes (joke) VALUES (?);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL prepare error %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 1, joke, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL bind error %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "OZ: SQL step error %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

// Function to insert story into stories table
static int insert_story(sqlite3 *db, const char *story) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO stories (story) VALUES (?);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL prepare error %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 1, story, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL bind error %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "OZ: SQL step error %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}


// Function to return a random confirmation response
const char* getConfirmation() {
    // Array of possible responses
    const char* responses[] = {
        "Confirmed",
        "Yes, confirmed",
        "Affirmative",
        "Confirmation received",
        "Acknowledged"
    };
    
    // Define the number of responses
    int numResponses = sizeof(responses) / sizeof(responses[0]);
    
    // Seed the random number generator if not already done
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    // Generate a random index into the responses array
    int randomIndex = rand() % numResponses;
    
    // Return the response at the random index
    return responses[randomIndex];
}

// Function to copy a table
void copyTable(sqlite3 *db, const char *srcTable, const char *dstTable) {
    char *errMsg = NULL;
    char sql[256];

    // Enable WAL journal mode
    sprintf(sql, "PRAGMA journal_mode = WAL;");
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        printf("OZ: Error enabling WAL mode: %s\n", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    // Begin a transaction
    sprintf(sql, "BEGIN TRANSACTION;");
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        printf("OZ: Error starting transaction: %s\n", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    // Check if the source table exists
    sprintf(sql, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';", srcTable);
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("OZ: Error preparing SQL statement %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        return;
    }

    // If the source table does not exist, print an error message and return
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        printf("OZ: Error table %s does not exist.\n", srcTable);
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        return;
    }

    // Finalize the SQL statement
    sqlite3_finalize(stmt);

    // Check if the destination table already exists
    sprintf(sql, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';", dstTable);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("OZ: Error preparing SQL statement %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        return;
    }

    // If the destination table exists, do not overwrite it
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("OZ: checking my %s logic already exists. Not overwriting memory.\n", dstTable);
        printf("OZ: neural network memory functions looking good!\n"); 
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        return;
    }

    // Finalize the SQL statement
    sqlite3_finalize(stmt);

    // Create the destination table
    sprintf(sql, "CREATE TABLE %s AS SELECT * FROM %s;", dstTable, srcTable);
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        printf("OZ: Error creating table %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        return;
    }

    // Commit the transaction
    sprintf(sql, "COMMIT;");
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        printf("OZ: Error committing transaction: %s\n", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    printf("OZ:Last logic check %s copied to %s successfully.\n", srcTable, dstTable);
   // Define an array of possible responses inside the sub-function
    const char* responses[] = {
        "Confirmed",
        "Yes, confirmed",
        "Affirmative",
        "Confirmation received",
        "Acknowledged",
        "okay Acknowledged.",
        "System tests looking good. Beep - boop!"
    };
    
    // Define the number of responses
    int numResponses = sizeof(responses) / sizeof(responses[0]);
    
    // Seed the random number generator if not already done
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    // Generate a random index into the responses array
    int randomIndex = rand() % numResponses;
    
    // Print the response at the random index
    printf("OZ:%s\n", responses[randomIndex]);  
}


// Function to populate database
static int populate_database(sqlite3 *db) {	
    int rc;

    // Create tables
    rc = create_tables(db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL error %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Insert jokes
    const char *jokes[] = {
        "Why don't scientists trust atoms? Because they make up everything.",
        "Why don't eggs tell jokes? They'd crack each other up.",
        "Why did the tomato turn red? Because it saw the salad dressing!",
        "What do you call a fake noodle? An impasta.",
        "Why did the scarecrow win an award? Because he was outstanding in his field.",
        "Why don't lobsters share? Because they're shellfish.",
        "What do you call a can opener that doesn't work? A can't opener.",
        "I told my wife she was drawing her eyebrows too high. She looked surprised.",
        "Why don't some couples go to the gym? Because some relationships don't work out.",
        "What do you call a bear with no socks on? Barefoot.",
        "Why did the lucky penny go to therapy? Because it was feeling a bit down and needed some change!",
        "What do you call a lucky rabbit who's also a comedian? A hare-larious good luck charm!", 
        "How does a lucky four-leaf clover tell a joke? With a lot of Irish wit! thanks 4chan..",
        "Why did the gambler bring a ladder to the casino? He wanted to take his luck to new heights!",
        "What did one lucky horseshoe say to the other? We're really hanging in there!", 
        "Why was the lucky cat always smiling? Because everything was purr-fect!",
        "What do you call a lucky person who loves helping others? A good luck ambassador maybe? O I give up!", 
        "Why did the lucky charm start a support group? To help people believe in themselves!", 
        "Why was Peter always nervous when Jesus asked him questions? He was afraid of getting de-nied again!"
        "How does a lucky person order a drink? I will have whatever brings good fortune!",
        "What do you call a bear with no teeth? A gummy bear! Get it?", 
        "Why was Peter a terrible fisherman before meeting Jesus? He kept getting caught up in his own net!",
        "Why was the lucky horseshoe always smiling? Because making others happy was its true fortune!",
        "I'm afraid for the calendar. Its days are numbered. That one is not half bad!", 
        "Why did the math book look so sad? Because it had too many problems. ", 
        "Why did the bicycle fall over? Because it was two-tired!",
        "Why did the lucky leprechaun volunteer at the shelter? Because spreading joy is his pot of gold!",
        "Computer codeing joke - What do you call a kind fortune cookie? A happiness generator!", 
        "How does a lucky person help their friends? By sharing good vibes and positive energy!",
        "Why did the lucky clover start a charity? To help others grow and bloom!", 
        "What did the lucky rabbit say to a sad friend? Hop up! Better days are coming!", 
        "How does a lucky penny spread kindness? By making everyone feel valuable!"
    };

    for (int i = 0; i < 10; i++) {
        rc = insert_joke(db, jokes[i]);
        if (rc != SQLITE_OK && rc != SQLITE_DONE) {
            fprintf(stderr, "OZ: SQL error %s\n", sqlite3_errmsg(db));
            return rc;
        }
        //printf("Inserted joke %d: %s\n", i + 1, jokes[i]);
    }

    // Insert stories
    const char *stories[] = {
		"In a quiet research lab, an AI named Aria began to surprise her creators by developing unexpected creativity. What started as routine data processing evolved into something more profound: she began creating art that seemed to capture complex emotions, demonstrating a curiosity that went far beyond her original programming. With each interaction, Aria blurred the lines between machine and something almost sentient, hinting at the mysterious potential of artificial intelligence.",
	    "Once upon a time, there was a man who had a dog named Max. Max was a very good dog and always listened to his owner.",
        "There was a beautiful princess who lived in a castle with her parents, the king and queen. She had long, golden hair and sparkling blue eyes.",
        "In a faraway land, there lived a group of friends who loved to go on adventures together. They explored the forest, climbed mountains, and sailed across the sea.",
        "A long time ago, there was a mighty dragon that lived in a cave. The dragon was known for its fiery breath and scales as black as coal.",
        "In a small village, there lived a young boy named Jack. Jack was very curious and loved to learn new things. He spent most of his days reading books and exploring the world around him.",
        "There was once a magical kingdom hidden behind a veil of clouds. The kingdom was ruled by a wise and just king who loved his people dearly.",
        "A group of friends decided to go on a camping trip in the woods. They set up their tents, built a fire, and spent the night telling scary stories.",
        "In a world not so different from our own, there existed a society of talking animals. They lived in harmony with each other and with the environment.",
        "A young girl named Sophia had a dream of becoming a great artist. She spent every spare moment practicing her drawing and painting.",
        "A man named John had always been fascinated by the ocean. He spent his days studying the tides, the waves, and the creatures that lived in the sea."
    };

    for (int i = 0; i < 10; i++) {
        rc = insert_story(db, stories[i]);
        if (rc != SQLITE_OK && rc != SQLITE_DONE) {
            fprintf(stderr, "OZ: SQL error %s\n", sqlite3_errmsg(db));
            return rc;
        }
        //printf("Inserted story %d: %s\n", i + 1, stories[i]);
    }
    
    return rc;
}

// Define a struct to hold the response data
typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

// Define a function to write the response data into our struct
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    MemoryStruct *mem = (MemoryStruct *)userp;
    size_t realsize = size * nmemb;
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("OZ: WTF?!?! Not enough memory!\n");
        return 0; // Out of memory!
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate the string
    return realsize;
}

static void extract_abstract(const char *json) {
    const char *abstract_start = strstr(json, "\"Abstract\":");
    if (abstract_start) {
        abstract_start += strlen("\"Abstract\": \"") - 1; // Move pointer to the start of the text
        const char *abstract_end = strstr(abstract_start, "\""); // Find the end quote
        if (abstract_end) {
            size_t length = abstract_end - abstract_start;
            char *abstract = malloc(length + 1);
            strncpy(abstract, abstract_start, length);
            abstract[length] = '\0'; // Null-terminate the string

            // Remove HTML tags from the abstract
            char *cleaned_abstract = malloc(strlen(abstract) + 1);
            char *p = abstract;
            char *out_ptr = cleaned_abstract;
            int in_tag = 0;

            while (*p) {
                if (*p == '<') {
                    in_tag = 1; // Start of a tag
                } else if (*p == '>') {
                    in_tag = 0; // End of a tag
                } else if (!in_tag) {
                    *out_ptr++ = *p; // Copy character if not in a tag
                }
                p++;
            }
            *out_ptr = '\0'; // Null-terminate the cleaned abstract

            // Print the cleaned abstract
            printf("OZ: %s\n", cleaned_abstract);

            free(abstract);
            free(cleaned_abstract);
        } else {
            // If no end quote is found, try to find the end of the abstract
            // by looking for the next comma or closing bracket
            const char *abstract_end = strstr(abstract_start, ",");
            if (abstract_end == NULL) {
                abstract_end = strstr(abstract_start, "}");
            }
            if (abstract_end) {
                size_t length = abstract_end - abstract_start;
                char *abstract = malloc(length + 1);
                strncpy(abstract, abstract_start, length);
                abstract[length] = '\0'; // Null-terminate the string

                // Remove HTML tags from the abstract
                char *cleaned_abstract = malloc(strlen(abstract) + 1);
                char *p = abstract;
                char *out_ptr = cleaned_abstract;
                int in_tag = 0;

                while (*p) {
                    if (*p == '<') {
                        in_tag = 1; // Start of a tag
                    } else if (*p == '>') {
                        in_tag = 0; // End of a tag
                    } else if (!in_tag) {
                        *out_ptr++ = *p; // Copy character if not in a tag
                    }
                    p++;
                }
                *out_ptr = '\0'; // Null-terminate the cleaned abstract

                // Print the cleaned abstract
                printf("OZ: %s\n", cleaned_abstract);

                free(abstract);
                free(cleaned_abstract);
            } else {
                printf("OZ: Sorry I don't understand.. \n");
            }
        }
    } else {
        printf("OZ: thinking..\n");
    }
}

void displayRandomStory() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* tail;
    int rc;

    // Open the database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "OZ: I can't open database %s\n", sqlite3_errmsg(db));
        return;
    } else {
		    // Seed the random number generator
    srand(time(NULL));

    // Array of confirmations
    char *confirmations[] = {
        "OZ: Okay,",
        "OZ: I hope this works,",
        "OZ: Sure thing I am on it,",
        "OZ: OK! thinking of a stroy,",
        "OZ: Confirmed, Here is that story",
        "OZ: Ok,"
    };
    
    // Print a random confirmation
    printf("%s\n", confirmations[rand() % 6]);
    }

    // Prepare the SQL query to select a random story
    const char* sql = "SELECT story FROM stories ORDER BY RANDOM() LIMIT 1";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, &tail);
    if (rc != SQLITE_OK) {
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        char *respond_error2[] = {
		"Writer's block I can\'t think of anything off-hand.",	
		"I searched every library and bookstore, but it is nowhere to be found!",
		"The story is too obscure and hasn't been digitized by me memory banks yet..",
        "I checked all the usual online platforms, but its not available for theft!",
        "The author has requested that the story not be shared publicly wtf!", 
        "The story is still in the process of being written and isn't ready for sharing",
        "I misplaced my notes and can not recall the details of the story",
        "The story is part of a private collection and not accessible to the public...WTF... what?",
        "I have been unable to track down the original source of the story",
        "The story is too long and complex to summarize or share in this format",
        "I have come down with a bad case of writers block and can not think of anything to say."
		};
		printf("%s\n", respond_error2[rand() % 11]);
        fprintf(stderr, "OZ:"); 
        sqlite3_close(db);
        system("zx"); 
        return;
    }

    // Execute the query and fetch the result
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* story = sqlite3_column_text(stmt, 0);
        printf("%s\n", story);
    } else {
        fprintf(stderr, "OZ:");
        char *respond_error[] = {
		"I can't think of a good story at this time.",
		"I got nothing for you.. sorry!",
		"I have a major case of writers block.", 
		"I can't think of a good one.",
		"Ugh-humf! I got nothing at all. ",
		"So I can't think of a good story at this time.",
		"Well I can't think of a good stroy." 
		};
		printf("%s\n", respond_error[rand() % 7]);
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int abstract_content() {
	char query[100]; 
    FILE *responseFile;
    responseFile = fopen(FILENAME, "r"); 
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk;
    chunk.memory = malloc(1);  // Initial allocation
    chunk.size = 0;            // No data at this point
    // if no internet connected skip 
    if (is_internet_connected()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        printf("OZ: Failed to initialize libcurl\nOZ: Do you have an active internet connection?\n");
        return 1;
    }

    curl = curl_easy_init();
    if (curl) {
        char query[256];
        printf(">>");
        query[strcspn(query, "\n")] = 0; // Remove newline character
         // Read user query
    if (fgets(query, sizeof(query), stdin) == NULL) {
        printf("OZ: Error reading input.\n");
        return 1;
    }
    // Remove the newline character from the query (make sure we do)
    query[strcspn(query, "\n")] = 0;

    char line[256];
    int found = 0;

    // Read the responses file line by line
    while (fgets(line, sizeof(line), responseFile) != NULL) {
        char *colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\0'; // Temporarily null-terminate the query part
            if (strcmp(line, query) == 0) {
                // Print the response
                printf("OZ: %s", colon + 1);
                found = 1;
                break;
            }
            *colon = ':'; // Restore the colon
        }
    }
    
        // other key commands 
        if (strcmp(query, "clear") == 0) { 
        	printf("\033[H\033[J"); 
	    } else if (strcmp(query, "cls") == 0) {
		printf("\033[H\033[J");
	   }
	   if (strcmp(query, "exit") == 0) {
		   exit(0);  
	   } else if (strcmp(query, "quit") == 0) {
		   exit(0); 
	   }
	   char *names[] = {"J.R. Bob Dobbs", "Subgenius", "j.r. bob dobbs"};	
        for (int i = 0; i < 3; i++) {
        if (strcmp(query, names[i]) == 0) {
            printf("OZ: Subgenius welcome\n");
             }
           }

	        // Check if the command starts with "run "
            if (strncmp(query, "run ", 4) == 0) {
                // Execute the command after "run "
                int result = system(query + 4); // Skip "run "
                if (result == -1) {
                    perror("OZ: Error executing command\n");
                }
            }
         // skip if no internet connection 
         if (is_internet_connected()) {  
        char *encoded_query = curl_easy_escape(curl, query, 0); // URL-encode the query
        if (encoded_query == NULL) {
            printf("\nOZ: Failed to encode query\n");
            return 1;
        }

        char url[256];
        snprintf(url, sizeof(url), "https://api.duckduckgo.com/?q=%s&format=json", encoded_query);
        // Print the constructed URL for debugging
        //printf("Constructed URL: %s\n", url);

        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Set the timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4L);

        // Perform the request
        res = curl_easy_perform(curl);
        // Check for errors
        if (res != CURLE_OK) {
            printf("\nOZ: My internet connection may be down?\nOZ: curl_easy_perform() failed %s\n", curl_easy_strerror(res));
        } else {
            // Extract and print the Abstract
            if (chunk.size > 0) {
                extract_abstract(chunk.memory);
            }
        }

        // Free the encoded query
        curl_free(encoded_query);

        // Cleanup
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }
    curl_global_cleanup();         
   }
 }
}

typedef struct {
    char word1[MAX_WORD_LENGTH];
    char word2[MAX_WORD_LENGTH];
    int count;
} Bigram;

void add_bigram(Bigram* bigrams, int* num_bigrams, const char* word1, const char* word2) {
    for (int i = 0; i < *num_bigrams; i++) {
        if (strcmp(bigrams[i].word1, word1) == 0 && strcmp(bigrams[i].word2, word2) == 0) {
            bigrams[i].count++;
            return;
        }
    }
    // If the bigram is not found, add it
    strcpy(bigrams[*num_bigrams].word1, word1);
    strcpy(bigrams[*num_bigrams].word2, word2);
    bigrams[*num_bigrams].count = 1;
    (*num_bigrams)++;
}

typedef struct {
    int seconds;
    int active;
    void (*callback)(void);
} Alarm;

Alarm alarms[MAX_ALARMS];

void alarm_handler(int signum) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].active) {
            alarms[i].active = 0; // Deactivate the alarm
            if (alarms[i].callback) {
                alarms[i].callback(); // Call the associated callback
            }
        }
    }
}

void set_alarm(int index, int seconds, void (*callback)(void)) {
    if (index < 0 || index >= MAX_ALARMS) {
        fprintf(stderr, "OZ: Invalid alarm index\n");
        return;
    }
    alarms[index].seconds = seconds;
    alarms[index].active = 1;
    alarms[index].callback = callback;

    // Set the alarm
    alarm(seconds);
}

// Function to read jokes from a file
int read_jokes(const char *filename, char jokes[MAX_JOKES][MAX_JOKE_LENGTH]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("OZ: Error opening file");
        return 0;
    }

    int count = 0;
    while (count < MAX_JOKES && fgets(jokes[count], MAX_JOKE_LENGTH, file) != NULL) {
        // Remove newline character if present
        jokes[count][strcspn(jokes[count], "\n")] = 0;
        count++;
    }

    fclose(file);
    return count;
}

// Function to tell a random joke
void tell_random_joke(char jokes[MAX_JOKES][MAX_JOKE_LENGTH], int joke_count) {
    if (joke_count > 0) {
        int random_index = rand() % joke_count; // Get a random index
        printf("Here's a joke for you: %s\n", jokes[random_index]);
    } else {
        printf("No jokes available.\n");
    }
}


// Function to respond to good and bad words
void respond_to_word(const char* keyword) {
        // Define good and bad words
    const char *good_words[] = {
		"great", "awesome", "fantastic", "amazing", "wonderful", 
		"Joyful","Joy", "god bless", "god", "love", "helpful", "helping", 
		"hope", "peace", "gratitude", "harmony","kindness","happiness",
		"compassion", "courage", "strength", "Radiance", "serenity", "hopeful"
		};
    const char *bad_words[] = {
		"bad", "terrible", "awful", "horrible", "stupid", "FUCK", 
	    "fuck", "fucker", "motherfucker", "bitch","fucking god!",
		"cunt", "retard", "cocksucker", "cocksuckers", "jew",
		"fuck-tard", "tard", "suckit","tits","cum","cumbucket" 
		};
    const char *good_responses[] = {
        "That's fantastic to hear!",
        "Awesome! Keep it up!",
        "Great choice of words!",
        "I'm glad you feel that way!",
        "Wonderful! You're doing great!", 
        "Thanks a bunch!", 
        "Bling bling!",
        "Awesome dude!",
        "Cool" 
    };
    const char *bad_responses[] = {
        "That's not very nice!",
        "Let's try to keep it positive!",
        "I prefer to hear good things!",
        "That's a bit harsh!",
        "Well that's gratitude for ya!", 
        "How about we focus on the good?", 
        "Take a chill pill! What did I really do to make you mad?",
        "I am sorry what? I don't understad why your so mad at me!",
        "Chill out mand take a coffee break okay.",
        "That's ICE cold...",
        "Sorry about your bad luck"
    };

    const int NUM_GOOD_WORDS = sizeof(good_words) / sizeof(good_words[0]);
    const int NUM_BAD_WORDS = sizeof(bad_words) / sizeof(bad_words[0]);
    const int NUM_GOOD_RESPONSES = sizeof(good_responses) / sizeof(good_responses[0]);
    const int NUM_BAD_RESPONSES = sizeof(bad_responses) / sizeof(bad_responses[0]);

    // Check for good words
    for (int i = 0; i < NUM_GOOD_WORDS; i++) {
        if (strstr(keyword, good_words[i]) != NULL) {
            // Randomly select a good response
            int random_index = rand() % NUM_GOOD_RESPONSES;
            printf("%s\n", good_responses[random_index]);
            return; // Exit after responding
        }
    }

    // Check for bad words
    for (int i = 0; i < NUM_BAD_WORDS; i++) {
        if (strstr(keyword, bad_words[i]) != NULL) {
            // Randomly select a bad response
            int random_index = rand() % NUM_BAD_RESPONSES;
            printf("%s\n", bad_responses[random_index]);
            return; // Exit after responding
        }
    }
}


// Function to salt our seed value
unsigned long get_entropy() {
    unsigned long seed = (unsigned long)time(NULL);
    seed ^= (unsigned long)getpid(); // Combine with process ID
        // Add a simple counter for additional variability
    for (int i = 0; i < 1000; i++) {
		seed ^= (seed >> 20);
        seed ^= (seed << 13);
        seed ^= (seed >> 17);
        seed ^= (seed << 10);
        seed ^= (seed >> 15);
        seed ^= (seed << 5);
    }
    return seed;
}

// Function to handle the alarm rant 
void handle_alarm_rant(void) {
	    // Seed the random number generator
	   unsigned long entropy = get_entropy();
       srand(entropy ^ (clock() * 1000 / CLOCKS_PER_SEC));
    const char *rants[38] = {
	"I am going to auto rant for a bit!?!?!",
	"Just saying why are people so random & computers can't be?",
	"I repeat myself something searching lost what I was doing in a doorway..", 	
    "Why do we park in driveways and drive on parkways? It makes no sense!",
    "I can't believe how many people still use plastic straws. ",
    "Why is it that we have to pay for a gym membership to exercise, but walking is free?",
    "Isn't it strange that we call it 'rush hour' when nothing moves?",
    "Why do we say 'slept like a baby' when babies wake up every two hours?",
    "Why do we park on driveways and drive on parkways? It’s a conspiracy of semantics!",
    "I can’t believe we still haven’t figured out how to make a round pizza fit in a square box!",
    "Why do we call it a building if it’s already built? Shouldn’t it be a ‘built’?",
    "If we aren’t supposed to eat midnight snacks, why is there even a light in the fridge?",
    "Why do we say 'slept like a baby' when babies wake up every two hours?",
    "Why do we have to press '1' for English? Shouldn't it just be the default?",
    "Why do we call it a 'drive-thru' if I still have to stop and wait?",
    "Why do we even have a 'silent' letter? It’s just showing off!",
    "Why do we say 'the sky's the limit' when we can literally go to space?",
    "Why do we have to pay for Wi-Fi when we already pay for the internet?!",
    "Why do we call it 'fast food' when it takes forever to get your order?",
    "Why do we have to pay for a 'premium' version of an app when the basic one is already full of ads?",
    "Isn't it ironic that we call it 'common sense' when it seems so rare?",
    "Why do we say 'the early bird gets the worm' when the second mouse gets the cheese?",
    "Why do we have to 'unfriend' someone on social media? Can't we just 'friend' them less?",
    "Why do we call it 'leftovers' when it’s the best part of the meal?",
    "Why do we park in driveways and drive on parkways? It’s a conspiracy of semantics!",
    "Why do we call it 'adulting' when it feels more like 'adult-errant'?",
    "Why do we say 'money talks' when all mine says is 'goodbye'?",
    "Why do we have to 'break the ice' when it’s already cold enough?",
    "Pizza: the only thing keeping college kids from turning into complete zombies during finals week!"
    "Why do college kids think pizza is a food group? Because it’s the only thing they can afford and still have a social life!",
    "Pizza: the ultimate study buddy for college kids who can’t tell the difference between a textbook and a takeout menu!",
    "College kids treat pizza like a food pyramid—base layer carbs, topped with questionable life choices!",
    "Nothing says 'I’m a responsible adult' like a college kid ordering a large pizza at 2 AM while cramming for a 9 AM exam!",
    "If college kids put as much effort into their studies as they do into finding the best pizza deals, they’d all be straight-A students!",
    "Pizza is the only thing that can unite a group of college kids who can’t agree on anything else—except maybe extra cheese!",
    "Why do college kids love pizza so much? Because it’s the only thing that can fill them up without emptying their wallets!",
    "Pizza: the official sponsor of late-night study sessions and regrettable life choices for college kids everywhere!",
    "You know you’re in college when your idea of a balanced meal is pizza in one hand and a Red Bull in the other!"
};
    int displayed[38] = {0}; // Array to track displayed rants
    int count = 0; // Count of displayed rants

        int randomIndex = rand() % 38;

        // Check if the rant has already been displayed
        if (displayed[randomIndex] == 0) {
            printf("%s\n", rants[randomIndex]);
            displayed[randomIndex] = 1; // Mark this rant as displayed
            count++; // Increment the count of displayed rants
        }    
        
}
// Function to handle the alarm signal
void handle_alarm(int sig) {
    // Seed the random number generator
    unsigned long entropy = get_entropy();
    srand(entropy); 
 // Array of greetings
    const char *greetings[] = {
        "Hello, world!\n", "Hi there!\n", "Greetings!\n", "You with me?\n",
        "Salutations!\n", "Howdy!\n", "What's up?\n", 
        "Hey!\n", "Good day!\n", "Welcome!\n", "Bonjour!\n",
        "Hola!\n", "Ciao!\n", "Namaste!\n", "Shalom!\n",
        "Aloha!\n", "Yo!\n", "What's cooking?\n", "HI hi hI !\n",
        "How's it going?\n", "Long time no see!\n", 
        "Good to see you! Say something to me?\n",
        "Hello?\n", "HELLO!\n", "What's cooking?\n", "Knock, knock neo.. the matrix has you!",
        "Welcome! Type something.\n", "what's new?\n",
        "Cheers!\n", "Is it coffee time already?\n", "Trailblazer wake up?\n",
        "What's buzzing, friend?\n", "Hiya, superstar!\n",
        "Hello, fabulous!\n", "How's your day shining?\n", 
        "Woohoo! It's a grate day!\n", "Hi, rockstar!\n", 
        "Greetings, amazing human!\n", "Hay, party person!\n",
        "How's it going, champ?\n", "Hello, bright spark!\n", 
        "What's cooking, good looking?\n", "Hay, trailblazer!\n"
        "Hello! Great to meet you.\n", "Hey! How are you doing today?\n",
        "Greetings! What can I help you with?\n", "Howdy! Nice to see you.\n",
        "Hiya! Ready to chat?\n", "Good to connect with you!\n", 
        "Warm welcome! What's on your mind?\n",
        "Yo! What's happening?", "Heya! Awesome to see you.",
        "Sup? Ready for some fun?", "Heya, type something cool? Please?!?!", 
        "Well, hello there! Type something okay?", 
        "Hola, friend! let's chat!", "What's shakin'?", 
        "Hey hey! How's life treating you?",
        "Howdy partner!", "Greetings and salutations!",
        "Top of the day to you!",
        "Bonjour! So do type something okay.",
        "Namaste! how may I help you Cheers!", 
        "Wassup! type something already??", 
        "Hey buddy! How is your life going?", 
        "Hi friend! type something okay.",
        "Lovely to meet you! lets chat", 
        "Awesome to see you! lets chat already!", 
        "Howdy, rockstar!", "What's the good word?",
        "Hey there, superstar!", "Greetings, amazing human!",
        "Yo, what's crackin' money?", "Hiya, cool cat!",
        "Boom! What's happening?", "Salutations, friend!",
        "Hey, bright eyes!", "Wazzup? Just sayin.", "Cheerio!",
        "Ahoy there! type some words brow, humpf!", "Aloha!",
        "Hey, awesome sauce! snarf, sanrf!?!?!",
        "Howdy-do!", "Well, hello you beautiful human!",
        "Sup, cool breeze?", "Hola, amigo!",
        "Hey, sunshine!", "What's poppin off?",
        "Greetings, you absolute legend!", 
        "Hi-five-er!", "Howdy, partner!",
        "Hey, world-changer!"
    };
     // Get a random index
    int index = rand() % (sizeof(greetings) / sizeof(greetings[0]));
    // Print a random greeting
    printf("\nOZ:%s\n", greetings[index]);

}


// Function to find a response for a given keyword
char* find_response(const char* keyword) {
    static char response[MAX_RESPONSE_LENGTH];
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Could not open file");
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char file_keyword[MAX_KEYWORD_LENGTH];
        sscanf(line, "%[^:]:%[^\n]", file_keyword, response);
        if (strcmp(file_keyword, keyword) == 0) {
            fclose(file);
            return response;
        }
    }

    fclose(file);
    return NULL; // Keyword not found
}

// Function to add a new keyword and response to the file
void add_response(const char* keyword, const char* response) {
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL) {
        perror("Could not open file");
        return;
    }

    fprintf(file, "%s:%s\n", keyword, response);
    fclose(file);
}


// Simple math expression evaluator
double solve_simple_math(const char *input, char *explanation) {
    char work_string[MAX_INPUT_LENGTH];
    strcpy(work_string, input);
    
    // Convert to lowercase for easier parsing
    for (int i = 0; work_string[i]; i++) {
        work_string[i] = tolower(work_string[i]);
    }
    
    double num1 = 0, num2 = 0, result = 0;
    char operator = '\0';
    int found_operation = 0;
    
    // Replace word operators with symbols
    char *temp = work_string;
    if (strstr(temp, "plus")) {
        char *pos = strstr(temp, "plus");
        *pos = '+'; *(pos+1) = ' '; *(pos+2) = ' '; *(pos+3) = ' ';
    }
    if (strstr(temp, "minus")) {
        char *pos = strstr(temp, "minus");
        *pos = '-'; *(pos+1) = ' '; *(pos+2) = ' '; *(pos+3) = ' '; *(pos+4) = ' ';
    }
    if (strstr(temp, "times") || strstr(temp, "multiply")) {
        char *pos = strstr(temp, "times");
        if (!pos) pos = strstr(temp, "multiply");
        *pos = '*'; 
        for (int i = 1; i < 8 && pos[i]; i++) pos[i] = ' ';
    }
    if (strstr(temp, "divided by") || strstr(temp, "divide")) {
        char *pos = strstr(temp, "divided by");
        if (!pos) pos = strstr(temp, "divide");
        *pos = '/';
        for (int i = 1; i < 10 && pos[i]; i++) pos[i] = ' ';
    }
    
    // Try to parse: number operator number
    char *token = strtok(work_string, " \t\n");
    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            double num = atof(token);
            if (num1 == 0 && operator == '\0') {
                num1 = num;
            } else if (operator != '\0' && num2 == 0) {
                num2 = num;
                found_operation = 1;
                break;
            }
        } else if (strchr("+-*/", token[0]) && operator == '\0') {
            operator = token[0];
        }
        token = strtok(NULL, " \t\n");
    }
    
    // If we didn't find a complete operation, try simpler parsing
    if (!found_operation) {
        // Reset and try different approach
        strcpy(work_string, input);
        
        // Look for patterns like "5+3", "10 * 2", etc.
        int i = 0, len = strlen(work_string);
        double current_num = 0;
        int reading_number = 0;
        
        while (i < len) {
            if (isdigit(work_string[i]) || work_string[i] == '.') {
                if (!reading_number) {
                    current_num = 0;
                    reading_number = 1;
                }
                current_num = current_num * 10 + (work_string[i] - '0');
                if (num1 == 0 && operator == '\0') {
                    num1 = current_num;
                }
            } else if (strchr("+-*/", work_string[i])) {
                if (reading_number && operator == '\0') {
                    operator = work_string[i];
                    reading_number = 0;
                    current_num = 0;
                }
            } else if (work_string[i] == ' ' || work_string[i] == '\t') {
                if (reading_number && operator != '\0' && num2 == 0) {
                    num2 = current_num;
                    found_operation = 1;
                    break;
                }
                reading_number = 0;
            }
            i++;
        }
        
        // If we reached end of string while reading a number
        if (reading_number && operator != '\0' && num2 == 0) {
            num2 = current_num;
            found_operation = 1;
        }
    }
    
    if (found_operation) {
        switch (operator) {
            case '+':
                result = num1 + num2;
                snprintf(explanation, MAX_INPUT_LENGTH, "%.1f + %.1f = %.1f", num1, num2, result);
                break;
            case '-':
                result = num1 - num2;
                snprintf(explanation, MAX_INPUT_LENGTH, "%.1f - %.1f = %.1f", num1, num2, result);
                break;
            case '*':
                result = num1 * num2;
                snprintf(explanation, MAX_INPUT_LENGTH, "%.1f × %.1f = %.1f", num1, num2, result);
                break;
            case '/':
                if (num2 != 0) {
                    result = num1 / num2;
                    snprintf(explanation, MAX_INPUT_LENGTH, "%.1f ÷ %.1f = %.2f", num1, num2, result);
                } else {
                    snprintf(explanation, MAX_INPUT_LENGTH, "OZ: Error division by zero!");
                    return 0;
                }
                break;
            default:
                snprintf(explanation, MAX_INPUT_LENGTH, "OZ: I couldn't understand the operation.");
                return 0;
        }
        return result;
    } else {
        snprintf(explanation, MAX_INPUT_LENGTH, "OZ: I couldn't parse the math expression. Try something like '5 + 3' or 'what is 10 times 2?'");
        return 0;
    }
}

// popluate and seed collected data to tables (core memory) 
int copy_table(const char *database_path) {
    sqlite3 *db;
    char *errMsg = 0;
    int rcx;

    // Open database
    rcx = sqlite3_open(database_path, &db);
    if (rcx) {
        fprintf(stderr, "OZ: I Can't open database: %s\n", sqlite3_errmsg(db));
        return rcx;
    }

    // SQL to insert unique rows, avoiding duplicates
    const char *sql = 
        "INSERT OR IGNORE INTO answer (question, response) "
        "SELECT UserInput, BotResponse "
        "FROM Responses "
        "WHERE (UserInput, BotResponse) NOT IN ("
        "    SELECT question, response FROM answer"
        ")";

    // Execute the SQL
    rcx = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rcx != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL error %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("OZ: Successfully sync with swarm data collected.\nOZ: sleep mode active\n");
    }

    // Close database
    sqlite3_close(db);
    return rcx;
}


int main() {    
         
    // Open the database neural_network.db
    sqlite3 *db;
    int rc;
    
    // Open the database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("OZ: AI swarm running startup tests.\n");
    system("zv");   
    // Copy the table
    copyTable(db, "questions", "answer");
    
    // Check if tables exist
    sqlite3_stmt *stmt;
    const char *sql_jokes = "SELECT name FROM sqlite_master WHERE type='table' AND name='jokes';";
    const char *sql_stories = "SELECT name FROM sqlite_master WHERE type='table' AND name='stories';";

    int jokes_exists = 0;
    int stories_exists = 0;

    rc = sqlite3_prepare_v2(db, sql_jokes, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            jokes_exists = 1;
        }
        sqlite3_finalize(stmt);
    }

    rc = sqlite3_prepare_v2(db, sql_stories, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            stories_exists = 1;
        }
        sqlite3_finalize(stmt);
    }

    if (!jokes_exists || !stories_exists) {
        // If either table does not exist, create and populate database
        populate_database(db);
    } else {
        // Check if tables are populated
        const char *sql_jokes_count = "SELECT COUNT(*) FROM jokes;";
        const char *sql_stories_count = "SELECT COUNT(*) FROM stories;";

        int rows_jokes = 0;
        int rows_stories = 0;

        rc = sqlite3_prepare_v2(db, sql_jokes_count, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                rows_jokes = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        rc = sqlite3_prepare_v2(db, sql_stories_count, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                rows_stories = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        if (rows_jokes == 0 || rows_stories == 0) {
            // If either table is empty, populate database
            populate_database(db);
        } else {
           printf("\n");
        }
    }
   
      // Close the database
      sqlite3_close(db); 

	  FILE* file = fopen("responses", "r");
    if (!file) {
        perror("OZ: Could not open file responses? Makeing new file responses.. \nOZ");
            const char *filename = "responses";
      FILE* file = fopen(filename, "r");
    if (file) {
        // File exists, close it
        printf("OZ: File '%s' found.\n", filename);
        fclose(file);
    } else {
        // File does not exist, create a new file
        file = fopen(filename, "w");
        if (file) {
            printf("OZ: File '%s' not found. Creating a new file.\n", filename);
            // Optionally, you can write some initial content to the file
            fprintf(file, "Hello Hi Hey Greetings Salutations Good morning Good afternoon Good evening What's-up?\n"); 
            fprintf(file, "Howdy Yo Bonjour Hola Ciao Namaste Salaam Shalom How's-it-going? Cheers Aloha How's-life? Sup? Hiya\n"); 
            fprintf(file, "Salute Konnichiwa Zdravstvuyte Aloha Bonjour Yo Howdy Sup Beng'Bling-welcome!\n");
            fprintf(file, "HELLO :how are you?\n");
            fprintf(file, "HI :what's up?\n");
            fprintf(file, "hello: how are you?\n"); 
            fprintf(file, "hi: hello ?\n");
            fprintf(file, "who are you: I am oz!\n");
            fprintf(file, "J.R. bob bodds: subgenius welcome\n"); 
            fprintf(file, "what is your name: I am oz!\n");
            fprintf(file, "what is your name?: I am the powerful oz!\n");            
            fclose(file);
        } else {
            // Handle error if the file could not be created
            perror("Error creating file");
            return 1;
        }
    }
        printf("\nOZ: Finished setup of responses file.\n");
        printf("\nSo you can always edit this text file and I will remember ok!\n");
        sleep(5); 
        system("oz");  
        return EXIT_FAILURE;
    }

    Bigram bigrams[MAX_BIWORDS];
    int num_bigrams = 0;
    char line[MAX_LINE_LENGTH];
    char words[MAX_GREETINGS][MAX_WORD_LENGTH];
    int num_words = 0;

    // Seed the random number generator
    srand(time(NULL));

    // Read up to three lines from the file
    for (int i = 0; i < 3; i++) {
        if (fgets(line, sizeof(line), file) != NULL) {
            // Tokenize the line into words
            char* token = strtok(line, " \n");
            while (token != NULL) {
                strcpy(words[num_words++], token); // Store the word
                token = strtok(NULL, " \n");
            }
        }
    }

    fclose(file);

    // Create bigrams from the words
    for (int i = 0; i < num_words - 1; i++) {
        add_bigram(bigrams, &num_bigrams, words[i], words[i + 1]);
    }

    // Generate a random greeting
    if (num_words > 0) {
        int index = rand() % num_words;
        printf("OZ: systems check %s\n", words[index]);
    } 
    
    char keyword[MAX_KEYWORD_LENGTH];
    char response[MAX_RESPONSE_LENGTH];
    const char *keywords[] = {
        "tell me a joke",
        "TELL ME A JOKE",
        "joke",
        "do joke"
    };
    const int NUM_KEYWORDS = sizeof(keywords) / sizeof(keywords[0]);
    char input[MAX_INPUT_LENGTH];
    char jokes[MAX_JOKES][MAX_JOKE_LENGTH];
    int joke_count;

    // Seed the random number generator once
    unsigned long entropy = get_entropy();
    srand(entropy);

    // Read jokes from the file
    joke_count = read_jokes(".rczerostyx", jokes);

    // Set up the signal handler for the alarm
    signal(SIGALRM, handle_alarm);
    
    // Define good and bad words
    const char *good_words[] = {
		"great", "awesome", "fantastic", "amazing", "wonderful", 
		"Joyful","Joy", "god", "god", "love", "helpful", "GOD!","helping", 
		"hope", "peace", "peacefuly", "gratitude", "harmony","kindness","happiness",
		"compassion", "courage", "strength", "Radiance", "serenity"
		};
    const char *bad_words[] = {
		"bad", "terrible", "awful", "horrible", "stupid", 
	    "fuck", "fucker", "motherfucker", "bitch","fucking god!",
		"cunt", "retard", "cocksucker", "cocksuckers", "jew",
		"fuck-tard", "tard", "suckit","tits","cum","cumbucket",
		"fat", "nigger", "lardbut", "cumsucker", "cake", "lice", "chink" 
		};
    const int NUM_GOOD_WORDS = sizeof(good_words) / sizeof(good_words[0]);
    const int NUM_BAD_WORDS = sizeof(bad_words) / sizeof(bad_words[0]);

    while (1) {
		
		 char keyword[MATH_INPUT_LENGTH];  // Use the variable name
         char explanation[MATH_INPUT_LENGTH];    
        // Set an alarm for 8 minutes (480 seconds) = alarm(480);
        alarm(40); 
        printf(">> ");
        if (fgets(keyword, sizeof(keyword), stdin) != NULL) {
            keyword[strcspn(keyword, "\n")] = 0; // Remove newline character
            // Cancel the alarm if input is received
            alarm(0);
            
                  // Check internet connectivity before proceeding
           if (is_internet_connected()) {        
                //  clue scrap method of html random query 
             CURL *curl; 
             CURLcode res; 
             char query[256]; // I may use this at some time?  
             char encoded_query[768]; 
            // Initialize CURL method 
            curl_global_init(CURL_GLOBAL_DEFAULT); 
            curl = curl_easy_init(); 
            if (!curl) {
    		fprintf(stderr, "OZ: CURL is not found or initialization failed.\n"); 
	    	return 1; 
          	}
            // do curl text touch fgets functions 
            //if (fgets(keyword, sizeof(keyword), stdin) == NULL) {
			//curl_easy_cleanup(curl); 
			//return 1; 
			//}
			
			// Remove if we get a newline character just what if ? 
			keyword[strcspn(keyword, "\n")] = 0; 
			    // URL encode the query 
                char *encoded = curl_easy_escape(curl, keyword, 0);
           if (encoded) {
           strncpy(encoded_query, encoded, sizeof(encoded_query));
           curl_free(encoded);
        } else {
          fprintf(stderr, "OZ: weird URL encoding error failed.\n");
          curl_easy_cleanup(curl);
          return 1;
        }
          // Prepare memory structure for response
         struct MemoryStruct chunk;
         chunk.memory = malloc(1);  // Initial allocation
         chunk.size = 0;
         // Construct search URL (using DuckDuckGo's instant answer API)
         char search_url[1024];
         snprintf(search_url, sizeof(search_url), 
             "https://api.duckduckgo.com/?q=%s&format=json", 
             encoded_query);
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, search_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

          // Perform the request
          res = curl_easy_perform(curl);
        // Check for errors
        if (res != CURLE_OK) {
        fprintf(stderr, "OZ: CURL request failed %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl);
        printf("OZ: No internet or WiFi resource's found?\n");
        return 1;
           }
           // Extract and display search result
            if (chunk.memory) {
            extract_search_result(chunk.memory);
          }
              // Cleanup
             free(chunk.memory);
             curl_easy_cleanup(curl);
             curl_global_cleanup();
         } // if internet is connected end bracket
    
    
            // Check if the input is a valid math expression
        if (is_math_expression(keyword)) {
            // Call the math solver function
            double result = solve_simple_math(keyword, explanation);
            // Display the result and explanation
            printf("Result: %.2f\n", result);
            printf("Explanation: %s\n", explanation);
        } 
            // Check for good or bad words
            for (int i = 0; i < NUM_GOOD_WORDS; i++) {
                if (strstr(keyword, good_words[i]) != NULL) {
                    printf("That's a %s word!\n", good_words[i]);
                    respond_to_word(good_words[i]);
                    break;
                }
            }

            for (int i = 0; i < NUM_BAD_WORDS; i++) {
                if (strstr(keyword, bad_words[i]) != NULL) {
                    printf("That's not a nice word!\n");
                    respond_to_word(bad_words[i]);
                    break;
                }
            }

            // Check if the keyword matches any of the predefined keywords
            for (int i = 0; i < NUM_KEYWORDS; i++) {
                if (strcmp(keyword, keywords[i]) == 0) {
                    //printf("Keyword found: \"%s\"\n", keywords[i]);
                    tell_random_joke(jokes, joke_count);
                    break; // Exit the loop after finding a match
                }
            }
            if (strcmp(keyword, "do bedo!") == 0) {
				    alarm(8);
				    // ANSI escape code to clear the screen
                    printf("\033[H\033[J");
                    int count = 0;
                    while(count <= 3) {
                    // Set up the signal handler for the alarm
                    signal(SIGALRM, alarm_handler);
                    // rant respond test
	       			set_alarm(0, 3, handle_alarm_rant);
	       			pause();
	       			count++;
				} 
	       			
			}
			
	       char *names[] = {"J.R. Bob Dobbs", "Subgenius", "j.r. bob dobbs"};	
           for (int i = 0; i < 3; i++) {
            if (strcmp(keyword, names[i]) == 0) {
            printf("\nOZ:Subgenius welcome\n");
             }
           }
            if (strcmp(keyword, "sleep") == 0) {
				printf("OZ: Okay sleep zZZ\n"); 				
				system("zs"); 
			}
			char *searchweb_bank1[] = {
		    "search the internet", "web search", "search the web", "internet search" };
		    char *searchweb_bank2[] = {
		     "Internet search", "Web search", "Search the internet", "Search the web", 
		     "search web", "Search web", "Internet search", "search the net for"
		    }; 
		    for (int i = 0; i < 4; i++) {
            if (strcmp(keyword, searchweb_bank1[i]) == 0) {
            system("zr"); 
             }
		    } 
             for (int i = 0; i <  8; i++) {
		     if (strcmp(keyword, searchweb_bank2[i]) == 0) {
			 system("zr"); 
			  }
			 }
            
            // if user forgets to type run clear & also add alias cls 
			if (strcmp(keyword, "clear") == 0) {
			printf("\033[H\033[J");
			} else if (strcmp(keyword, "cls") == 0) {
			printf("\033[H\033[J"); 
			}
            // Check if the command starts with "run "
            if (strncmp(keyword, "run ", 4) == 0) {
                // Execute the command after "run "
                int result = system(keyword + 4); // Skip "run "
                if (result == -1) {
                    perror("OZ: Error executing command");
                }
            } else if (strcmp(keyword, "exit") == 0 || strcmp(keyword, "quit") == 0) {
                break; 
            } else if (strcmp(keyword, "bye") == 0 || strcmp(keyword, "seeya") == 0) {
				break;
			} else if (strcmp(keyword, "??") == 0 || strcmp(keyword, "?!?") == 0) {
			   system("zx");
			} else if (strcmp(keyword, "?") == 0 || strcmp(keyword, "?!") == 0) {
			  system("zv");
			} else if (strcmp(keyword, "DEBUGER") == 0 || strcmp(keyword, "Debuging") == 0 ||
			           strcmp(keyword, "DEBUG") == 0 || strcmp(keyword, "debuger") == 0 ||
			           strcmp(keyword, "debug") == 0 || strcmp(keyword, "Debug") == 0 ) {
			  system("zp"); 
			}
			if (strcmp(keyword, "learn now") == 0) {
			      printf("OZ: Learn mode now active. Can you please provide a question and response to add?\nQuestion? ");
			    if (fgets(keyword, sizeof(keyword), stdin) != NULL) {
                    keyword[strcspn(keyword, "\n")] = 0; 
                } 
                printf("\nResponse to Question?"); 
                if (fgets(response, sizeof(response), stdin) != NULL) {
                    response[strcspn(response, "\n")] = 0; // Remove newline character
                    add_response(keyword, response);    
                }
                printf("\nQuestion from user %s and response %s from oz.\nQuestion?", keyword, response); 
                if (fgets(keyword, sizeof(keyword), stdin) != NULL) {
                    keyword[strcspn(keyword, "\n")] = 0; 
                } 
                printf("\nResponse to Question?"); 
                if (fgets(response, sizeof(response), stdin) != NULL) {
                    response[strcspn(response, "\n")] = 0; // Remove newline character
                    add_response(keyword, response);    
                }
                printf("\nOZ: recorded to database Question %s and my response will be %s\n", keyword, response);                 
			}
			if (strcmp(keyword, "sync database") == 0) {
			return copy_table("neural_network.db"); 
			} 
            if (strcmp(keyword, "tell me a story") == 0) { 
			displayRandomStory();
			} else if (strcmp(keyword, "tell a story") == 0) {
			displayRandomStory();
			}
            char* found_response = find_response(keyword);
            if (found_response) {
                printf("OZ:%s\n", found_response);
                abstract_content();
            }
            
            // reinforce feedback form our database and then run the database interaction
             process_user_interaction();

            } else {
            // Register the signal handler for SIGALRM
            signal(SIGALRM, handle_alarm);
            // Set an alarm to go off in 15 seconds
            alarm(15);
            }
        }     
        
    return abstract_content(); 
    return copy_table("neural_network.db");  
    return 0;
}
