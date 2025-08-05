/* 
 * domx an example of a smart bash like subsystem. 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h> 

#define MAX_INPUT 256
#define MAX_RESPONSES 18
#define MAX_KEYWORDS 13
#include <sqlite3.h>
#define NUM_RESPONSES 5
#define MAX_RESPONSE_LENGTH 100

// Function to get a random response from the array
char* getRandomResponse(char responses[][MAX_RESPONSE_LENGTH]) {
    // Seed the random number generator
    srand(time(NULL) % 24000);
       // Multiple sources of randomness
    unsigned int seed = time(NULL) + clock();
    
    // Use multiple randomization techniques
    srand(seed);
    
    // Additional randomization step
    for (int i = 0; i < rand() % 10; i++) {
        rand(); // Burn some random numbers
    }
    
    // Generate a random index
    int randomIndex = rand() % NUM_RESPONSES;
    
    // Return the randomly selected response
    return responses[randomIndex];
}

// Database Function Prototypes
sqlite3* initialize_database();

// Add this function prototype before it's used
int get_or_create_user(
    sqlite3 *db,           // Database connection
    const char *username   // User's username
);

// Existing prototypes
void save_conversation_context(
    sqlite3 *db, 
    int user_id, 
    const char *user_input, 
    const char *bot_response
);

char* retrieve_previous_context(
    sqlite3 *db, 
    int user_id, 
    int context_depth
);

// Conversation Management Prototypes
char* generate_adaptive_response(
    sqlite3 *db, 
    const char *input, 
    int user_id
);

void adapt_response_database(
    sqlite3 *db, 
    const char *user_input, 
    const char *bot_response
);

//void start_conversation(sqlite3 *db);

// Logging Levels
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

// Conversation Topic Structure
typedef struct {
    char *keywords[MAX_KEYWORDS];
    char *responses[MAX_RESPONSES];
    int keyword_count;
    int response_count;
} ConversationTopic;

// User Profile Structure
typedef struct {
    int user_id;
    char username[50];
    int conversation_count;
    time_t last_interaction;
    char *preferences[10];  // Potential user preferences
} UserProfile;

// Global Configuration
//typedef struct {
    //int max_context_depth;
    //int learning_enabled;
    //LogLevel current_log_level;
//} BotConfiguration;

// Utility Function Prototypes
void log_message(LogLevel level, const char *message);
void to_lowercase(char *str);
void clean_input(char *input);


// User Profile Structure
//typedef struct {
    //int user_id;
    //char username[50];
    //int conversation_count;
    //time_t last_interaction;
//} UserProfile;

// Logging Utility Function
void log_message(LogLevel level, const char *message) {
    // File for logging
    FILE *log_file;
    
    // Timestamp buffer
    char timestamp[64];
    time_t now;
    struct tm *t;

    // Get current time
    time(&now);
    t = localtime(&now);
    
    // Format timestamp
    strftime(timestamp, sizeof(timestamp), 
             "%Y-%m-%d %H:%M:%S", t);

    // Log level strings
    const char *level_strings[] = {
        "DEBUG", 
        "INFO", 
        "WARNING", 
        "ERROR"
    };

    // Open log file in append mode
    log_file = fopen("chatbot.log", "a");
    
    if (log_file == NULL) {
        // Fallback to stderr if file can't be opened
        fprintf(stderr, "Failed to open log file\n");
        return;
    }

    // Log format: [TIMESTAMP] [LEVEL] Message
    fprintf(log_file, "[%s] [%s] %s\n", 
            timestamp, 
            level_strings[level], 
            message);

    // Also print to console for critical levels
    if (level >= LOG_WARNING) {
        fprintf(stderr, "[%s] [%s] %s\n", 
                timestamp, 
                level_strings[level], 
                message);
    }

    // Close log file
    fclose(log_file);
}


// Use existing table with a flag to mark training data
const char *add_training_flag_sql = 
    "ALTER TABLE response_patterns ADD COLUMN is_training INTEGER DEFAULT 0;";

// Create separate training table
const char *create_training_table_sql = 
    "CREATE TABLE IF NOT EXISTS training_data ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "input_pattern TEXT NOT NULL, "
    "bot_response TEXT NOT NULL, "
    "category TEXT, "
    "confidence REAL DEFAULT 1.0, "
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    ");";

// Function to insert training data
int insert_training_data(sqlite3 *db, const char *input, const char *response, const char *category) {
    const char *sql = "INSERT INTO training_data (input_pattern, bot_response, category) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;
    
    sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, category, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

// Function to load basic training data
void load_basic_training_data(sqlite3 *db) {
    // Greetings
    insert_training_data(db, "hello", "Hello! How can I help you?", "greeting");
    insert_training_data(db, "hi", "Hi there! What can I do for you?", "greeting");
    insert_training_data(db, "hey", "Hey! How's it going?", "greeting");
    insert_training_data(db, "good morning", "Good morning! Hope you're having a great day!", "greeting");
    insert_training_data(db, "good day", "Top of the morning to ya!", "greeting"); 
    
    // Farewells
    insert_training_data(db, "bye", "Goodbye! Have a great day!", "farewell");
    insert_training_data(db, "goodbye", "Take care! See you later!", "farewell");
    insert_training_data(db, "see you", "See you later! Thanks for chatting!", "farewell");
    insert_training_data(db, "exit", "See you next time, Thanks for chatting with me.", "farewell"); 
    
    // Simple questions
    insert_training_data(db, "how are you", "I'm doing well, thank you for asking! How are you?", "status");
    insert_training_data(db, "what's your name", "I'm your helpful assistant bot!", "identity");
    insert_training_data(db, "who are you", "I'm an AI assistant designed to help answer your questions.", "identity");
    insert_training_data(db, "your name is", "I'm an AI assistant call me domx", "identity"); 
    insert_training_data(db, "i am good", "That is wonderful", "identity"); 
    insert_training_data(db, "I am very good", "Wonderful", "identity"); 
    // Help requests
    insert_training_data(db, "help", "I'm here to help! What do you need assistance with?", "help");
    insert_training_data(db, "what can you do", "I can answer questions, have conversations, and help with various tasks. What would you like to know?", "help");
    
    // Thanks
    insert_training_data(db, "thank you", "You're welcome! Happy to help!", "gratitude");
    insert_training_data(db, "thanks", "No problem! Glad I could assist!", "gratitude");
    insert_training_data(db, "your welcome", "Thanks glad i culd assist you", "gratitude"); 
    
}

// Function to find matching response (simple keyword matching)
char* find_response(sqlite3 *db, const char *user_input) {
    const char *sql = "SELECT bot_response FROM training_data WHERE LOWER(?) LIKE '%' || LOWER(input_pattern) || '%' ORDER BY LENGTH(input_pattern) DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    char *response = NULL;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *result = (const char*)sqlite3_column_text(stmt, 0);
            if (result) {
                response = malloc(strlen(result) + 1);
                strcpy(response, result);
            }
        }
        sqlite3_finalize(stmt);
    }
    
    return response; // Remember to free() this when done
}

// Setup function to call once
void setup_training_database(sqlite3 *db) {
    char *err_msg = 0;
    
    // Create training table
    sqlite3_exec(db, create_training_table_sql, 0, 0, &err_msg);
    
    // Load basic training data (only if table is empty)
    const char *count_sql = "SELECT COUNT(*) FROM training_data;";
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    if (count == 0) {
        load_basic_training_data(db);
        printf("Loaded basic training data\n");
    }
}

// Enhanced logging with format support
void log_message_format(LogLevel level, const char *format, ...) {
    char message[512];
    va_list args;

    // Use variadic arguments to support formatted logging
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // Call standard log_message
    log_message(level, message);
}
// Database Connection and Initialization
sqlite3* initialize_database() {
    sqlite3 *db;
    char *err_msg = 0;
    
    // Open database
    int rc = sqlite3_open("chatbot_context.db", &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    // Create tables if not exist
    const char *create_user_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE,"
        "conversation_count INTEGER DEFAULT 0,"
        "last_interaction DATETIME"
        ");";

    const char *create_conversation_table = 
        "CREATE TABLE IF NOT EXISTS conversations ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER,"
        "user_input TEXT,"
        "bot_response TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(user_id)"
        ");";

    rc = sqlite3_exec(db, create_user_table, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    rc = sqlite3_exec(db, create_conversation_table, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    return db;
}
void save_conversation_context(
    sqlite3 *db, 
    int user_id, 
    const char *user_input, 
    const char *bot_response
) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT INTO conversations (user_id, user_input, bot_response) "
        "VALUES (?, ?, ?);";

    // Prepare SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare conversation save statement: %s\n", 
                sqlite3_errmsg(db));
        return;
    }

    // Bind parameters safely
    if (sqlite3_bind_int(stmt, 1, user_id) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, user_input, -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, bot_response, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameters for conversation save\n");
        sqlite3_finalize(stmt);
        return;
    }

    // Execute the statement
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Conversation save failed: %s\n", sqlite3_errmsg(db));
    }

    // Update user conversation count
    const char *update_sql = 
        "UPDATE users SET conversation_count = conversation_count + 1, "
        "last_interaction = datetime('now') WHERE user_id = ?;";
    
    sqlite3_stmt *update_stmt;
    if (sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(update_stmt, 1, user_id);
        sqlite3_step(update_stmt);
        sqlite3_finalize(update_stmt);
    }

    // Finalize the original statement
    sqlite3_finalize(stmt);
}

// Advanced Context Retrieval Function
char* retrieve_previous_context(sqlite3 *db, int user_id, int context_depth) {
    sqlite3_stmt *stmt;
    char *context = malloc(1024 * sizeof(char));
    context[0] = '\0';

    const char *sql = 
        "SELECT user_input, bot_response FROM conversations "
        "WHERE user_id = ? ORDER BY timestamp DESC LIMIT ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare context retrieval: %s\n", sqlite3_errmsg(db));
        free(context);
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, context_depth);

    // Accumulate context
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *user_input = (const char*)sqlite3_column_text(stmt, 0);
        const char *bot_response = (const char*)sqlite3_column_text(stmt, 1);
        
        char temp_context[512];
        snprintf(temp_context, sizeof(temp_context), 
                 "User: %s\nBot: %s\n", 
                 user_input ? user_input : "N/A", 
                 bot_response ? bot_response : "N/A");
        
        // Prepend to maintain chronological order
        memmove(context + strlen(temp_context), context, strlen(context) + 1);
        strcpy(context, temp_context);
    }

    sqlite3_finalize(stmt);
    return context;
}
// Machine Learning-like Context Adaptation
void adapt_response_database(
    sqlite3 *db, 
    const char *user_input, 
    const char *bot_response
) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT OR REPLACE INTO response_patterns "
        "(input_pattern, response_count, last_used) VALUES (?, "
        "COALESCE((SELECT response_count FROM response_patterns WHERE input_pattern = ?) + 1, 1), "
        "datetime('now'));";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare adaptation statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_input, -1, SQLITE_STATIC);

    // Execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to adapt response pattern\n");
    }

    sqlite3_finalize(stmt);
}
void debug_table_structure(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *pragma_sql = "PRAGMA table_info(response_patterns);";
    
    if (sqlite3_prepare_v2(db, pragma_sql, -1, &stmt, 0) == SQLITE_OK) {
        printf("Current table structure:\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Column: %s, Type: %s\n", 
                   sqlite3_column_text(stmt, 1),  // column name
                   sqlite3_column_text(stmt, 2)); // column type
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Table 'response_patterns' does not exist\n");
    }
}
int create_response_patterns_table(sqlite3 *db) {

const char *create_table_sql = 
    "CREATE TABLE IF NOT EXISTS response_patterns ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "input_pattern TEXT NOT NULL, "
    "bot_response TEXT NOT NULL, "
    "response_count INTEGER DEFAULT 1, "
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    ");";


    char *err_msg = 0;
    int rc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create table: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    printf("Table created successfully? runing verify, \n");
    
//const char *create_table_sql = 
    //"CREATE TABLE IF NOT EXISTS response_patterns ("
    //"id INTEGER PRIMARY KEY AUTOINCREMENT, "
    //"input_pattern TEXT NOT NULL, "
    //"bot_response TEXT NOT NULL, "
    //"response_count INTEGER DEFAULT 1, "
    //"created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    //");";

// To verify the table structure was created correctly, you can use:
const char *check_table_sql = "PRAGMA table_info(response_patterns);";

// Example of how to execute and check for errors:
int xrc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);
if (xrc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
}

// Check if bot_response column exists before adding it
const char *check_column_sql = 
    "SELECT COUNT(*) FROM pragma_table_info('response_patterns') WHERE name='bot_response';";

const char *add_column_sql = 
    "ALTER TABLE response_patterns ADD COLUMN bot_response TEXT;";

sqlite3_stmt *stmt;
int column_exists = 0;

// Check if column already exists
int crc = sqlite3_prepare_v2(db, check_column_sql, -1, &stmt, NULL);
if (crc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        column_exists = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
}

// Only add column if it doesn't exist
if (!column_exists) {
    crc = sqlite3_exec(db, add_column_sql, 0, 0, &err_msg);
    if (crc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Column added successfully\n");
    }
} else {
    printf("Column already exists\n");
}
    return 0;
}

char* generate_adaptive_response(sqlite3 *db, const char *input, int user_id) {
    sqlite3_stmt *stmt;
    char *response = calloc(512, sizeof(char));
    
    if (!db || !input) {
        free(response);
        return NULL;
    }
    
    // Debug: Check if table exists and has correct structure
    debug_table_structure(db);
    
    // First, ensure table exists
    if (create_response_patterns_table(db) != 0) {
        free(response);
        return NULL;
    }
    
    char *previous_context = retrieve_previous_context(db, user_id, 3);
    
    // Updated query - make sure column names match your actual table
    const char *pattern_sql = 
        "SELECT bot_response FROM response_patterns "
        "WHERE input_pattern LIKE ? "
        "ORDER BY response_count DESC LIMIT 1;";
        
    if (sqlite3_prepare_v2(db, pattern_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        fprintf(stderr, "SQL query was: %s\n", pattern_sql);
        free(response);
        if (previous_context) free(previous_context);
        return NULL;
    }
    
    // Rest of your code...
    char pattern_match[256];
    snprintf(pattern_match, sizeof(pattern_match), "%%%s%%", input);
    
    if (sqlite3_bind_text(stmt, 1, pattern_match, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        fprintf(stderr, "Bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        free(response);
        if (previous_context) free(previous_context);
        return NULL;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *learned_response = (const char*)sqlite3_column_text(stmt, 0);
        snprintf(response, 512, "%s (Based on previous learning)", 
                learned_response ? learned_response : "");
    } else {
		char *response_2 = find_response(db, input); 
		if (response) {
        snprintf(response, 512, "bot: %s", response_2);
        free(response_2); 
	}        
    }
    
    sqlite3_finalize(stmt);
    
    if (previous_context) {
        size_t current_len = strlen(response);
        size_t available = 511 - current_len;
        
        if (available > 20) {
            strncat(response, "\n[Context Hint: ", available);
            current_len = strlen(response);
            available = 511 - current_len;
            
            if (available > 2) {
                strncat(response, previous_context, available - 2);
                strcat(response, "]");
            }
        }
        free(previous_context);
    }
    
    return response;
}

// Optional: Enhanced error recovery function
void handle_conversation_error(
    sqlite3 *db, 
    int user_id, 
    const char *error_context
) {
    char error_log[512];
    snprintf(error_log, sizeof(error_log), 
             "Conversation Error for User ID %d: %s", 
             user_id, error_context);
    
    log_message(LOG_ERROR, error_log);

    // Optional: Log error to database
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT INTO error_logs "
        "(user_id, error_message, timestamp) "
        "VALUES (?, ?, datetime('now'));";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, error_context, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// Conversation context recovery
void recover_conversation_context(
    sqlite3 *db, 
    int user_id
) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "SELECT user_input, bot_response, timestamp "
        "FROM conversations "
        "WHERE user_id = ? "
        "ORDER BY timestamp DESC "
        "LIMIT 5;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);

        printf("\n--- Recent Conversation Context ---\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *user_input = (const char*)sqlite3_column_text(stmt, 0);
            const char *bot_response = (const char*)sqlite3_column_text(stmt, 1);
            const char *timestamp = (const char*)sqlite3_column_text(stmt, 2);

            printf("[%s]\n", timestamp);
            printf("User: %s\n", user_input ? user_input : "N/A");
            printf("Bot: %s\n\n", bot_response ? bot_response : "N/A");
        }
        printf("--- End of Context ---\n");

        sqlite3_finalize(stmt);
    }

}
void start_conversation(sqlite3 *db) {
    char input[MAX_INPUT];
    char username[50];
    int login_attempts = 0;
    const int MAX_LOGIN_ATTEMPTS = 3;
    // Set up once when your program starts
    setup_training_database(db);
   // Notes: 
   // find responses function options here 
   //char *response = find_response(db, "hello there");
   //if (response) {
    //printf("Bot: %s\n", response);
    //free(response);
   //} else {
    //printf("Bot: I'm not sure how to respond to that.\n");
   //}

    // Validate database connection
    if (!db) {
        log_message(LOG_ERROR, "Invalid database connection in start_conversation");
        return;
    }

    // User Authentication Loop
    while (login_attempts < MAX_LOGIN_ATTEMPTS) {
        // Clear previous input
        memset(username, 0, sizeof(username));
        
        // Prompt for username
        printf("Enter your username (3-20 characters, alphanumeric): ");
        
        // Safe input reading
        if (fgets(username, sizeof(username), stdin) == NULL) {
            log_message(LOG_ERROR, "Input reading failed");
            break;
        }

        // Remove newline
        username[strcspn(username, "\n")] = 0;

        // Validate username
        if (strlen(username) < 3 || strlen(username) > 20) {
            printf("Username must be between 3-20 characters.\n");
            login_attempts++;
            continue;
        }

        // Sanitize username (remove special characters)
        for (int i = 0; username[i]; i++) {
            if (!isalnum(username[i]) && username[i] != '_') {
                username[i] = '_';
            }
        }

        // Get or create user
        int user_id = get_or_create_user(db, username);
        
        if (user_id != -1) {
            // Successful user retrieval/creation
            char welcome_msg[128];
            snprintf(welcome_msg, sizeof(welcome_msg), 
                     "Welcome, %s! (User ID: %d)", username, user_id);
            log_message(LOG_INFO, welcome_msg);

            // Conversation Loop
            while (1) {
                printf("\n[%s] You: ", username);
                
                // Reset input
                memset(input, 0, sizeof(input));
                
                // Safe input reading
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    log_message(LOG_WARNING, "Input reading terminated");
                    break;
                }

                // Remove newline
                input[strcspn(input, "\n")] = 0;

                // Trim whitespace
                char *start = input;
                char *end = input + strlen(input) - 1;
                while (isspace(*start)) start++;
                while (end > start && isspace(*end)) end--;
                *(end + 1) = '\0';
                memmove(input, start, end - start + 2);
                char *response = find_response(db, input);
                 if (response) {
                 printf("Bot: %s\n", response);
                 free(response);
                } else {
                 printf("Bot: thinks to self about that response?\n");
                     // Define an array of responses
    char responses[NUM_RESPONSES][MAX_RESPONSE_LENGTH] = {
        "I am just not sure how to resond to that.",
        "I can't find anything about that.",
        "Nice to meet you by the way. thanks for chatting with me.",
        "What's up? I can not find A respones about your input. Sorry!",
        "I can't find anything for a response to that."
    };
    
              // Demonstrate getting random response 
              for (int i = 0; i < 1; i++) {
             // Small delay to ensure different clock cycles
            for(int j = 0; j < 1000000; j++) {} // Simple busy wait        
                char* randomResponse = getRandomResponse(responses);
                 //printf("bot: %d: %s\n", i + 1, randomResponse);
                 printf("bot: %s\n", randomResponse); 
                 }
                }
                // Exit conditions
                if (strcmp(input, "quit") == 0 || 
                    strcmp(input, "exit") == 0 || 
                    strcmp(input, "bye") == 0) {
                    
                    printf("Goodbye, %s!\n", username);
                    log_message(LOG_INFO, "User initiated exit");
                    break;
                }

                // Skip empty inputs
                if (strlen(input) == 0) {
                    printf("Please enter a message.\n");
                    continue;
                }

                // Generate adaptive response
                char *bot_response = generate_adaptive_response(db, input, user_id);
                
                if (bot_response) {
                    printf("Bot: %s\n", bot_response);

                    // Save conversation context
                    save_conversation_context(db, user_id, input, bot_response);

                    // Adapt response database
                    adapt_response_database(db, input, bot_response);

                    // Free dynamically allocated response
                    free(bot_response);
                } else {
                    printf("Bot: I'm not sure how to respond.\n");
                    log_message(LOG_WARNING, "Failed to generate response");
                }
            }

            // Exit conversation successfully
            return;
         } else {
            // User creation/retrieval failed
            printf("Failed to create/retrieve user profile.\n");
            login_attempts++;

            // Provide helpful feedback based on attempts
            switch (login_attempts) {
                case 1:
                    printf("Tip: Use only letters, numbers, and underscores.\n");
                    break;
                case 2:
                    printf("Please check your username and try again.\n");
                    break;
                case 3:
                    printf("Maximum login attempts reached.\n");
                    log_message(LOG_ERROR, "Exceeded maximum login attempts");
                    break;
            }

            // Optional: Short delay to prevent rapid repeated attempts
            if (login_attempts < MAX_LOGIN_ATTEMPTS) {
                sleep(1);  // Requires #include <unistd.h>
            }
        }
    }

    // Final error handling if all attempts fail
    if (login_attempts >= MAX_LOGIN_ATTEMPTS) {
        log_message(LOG_ERROR, "User authentication completely failed");
        printf("Authentication failed. Please restart the application.\n");
    }
}
           
int get_or_create_user(sqlite3 *db, const char *username) {
    sqlite3_stmt *stmt;
    int user_id = -1;

    // Validate input
    if (!db || !username || strlen(username) == 0) {
        log_message(LOG_ERROR, "Invalid username or database connection");
        return -1;
    }

    // Sanitize username (remove special characters)
    char sanitized_username[64];
    strncpy(sanitized_username, username, sizeof(sanitized_username) - 1);
    sanitized_username[sizeof(sanitized_username) - 1] = '\0';
    
    // Remove non-alphanumeric characters
    for (int i = 0; sanitized_username[i]; i++) {
        if (!isalnum(sanitized_username[i]) && sanitized_username[i] != '_') {
            sanitized_username[i] = '_';
        }
    }

    // Prepare SQL for user retrieval or creation
    const char *sql = 
        "INSERT OR IGNORE INTO users (username, conversation_count, last_interaction) "
        "VALUES (?, 0, datetime('now'));"
        "SELECT user_id FROM users WHERE username = ?;";
    // Prepare statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Failed to prepare user retrieval statement: %s", 
                 sqlite3_errmsg(db));
        log_message(LOG_ERROR, error_msg);
        return -1;
    }
// Print out binding details
printf("Sanitized Username: %s\n", sanitized_username);
printf("Username Length: %zu\n", strlen(sanitized_username));

    // Bind username parameters
    // Change SQLITE_STATIC to SQLITE_TRANSIENT to create a copy
if (sqlite3_bind_text(stmt, 1, sanitized_username, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    log_message(LOG_ERROR, "Failed to bind parameters");
    sqlite3_finalize(stmt);
    return -1;
}
// Always check binding results
if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    // Handle binding failure
    sqlite3_finalize(stmt);
   
}

// Reset statement after use
sqlite3_reset(stmt);
sqlite3_clear_bindings(stmt);
 
    // Execute statement
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // User exists, retrieve user_id
        user_id = sqlite3_column_int(stmt, 0);
        
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), 
                 "Retrieved existing user: %s (ID: %d)", 
                 sanitized_username, user_id);
        log_message(LOG_INFO, log_msg);
    } else if (rc == SQLITE_DONE) {
        // New user created
        user_id = sqlite3_last_insert_rowid(db);
        
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), 
                 "Created new user: %s (ID: %d)", 
                 sanitized_username, user_id);
        log_message(LOG_INFO, log_msg);
    } else {
        // Error occurred
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Failed to get/create user: %s", 
                 sqlite3_errmsg(db));
        log_message(LOG_ERROR, error_msg);
        user_id = -1;
    }

    // Finalize statement
    sqlite3_finalize(stmt);

    return user_id;
}

// Struct for more complex response handling
//typedef struct {
    //char *keywords[MAX_KEYWORDS];
    //char *responses[MAX_RESPONSES];
    //int keyword_count;
    //int response_count;
//} ConversationTopic;

// Dynamic response database
ConversationTopic topics[] = {
    {
        // Greeting topic
        .keywords = {"hello", "hi", "hey", "greetings"},
        .responses = {
            "Hi there! How are you doing?", 
            "Hello! Nice to meet you.", 
            "Greetings! What can I help you with today?"
        },
        .keyword_count = 4,
        .response_count = 3
    },
    {
        // Mood topic
        .keywords = {"how are you", "feeling", "mood"},
        .responses = {
            "I'm doing great, thanks for asking!", 
            "Feeling fantastic today!", 
            "Always good, how about you?"
        },
        .keyword_count = 3,
        .response_count = 3
    },
    {
        // Weather topic
        .keywords = {"weather", "temperature", "sunny", "rainy"},
        .responses = {
            "I can't check the weather, but I hope it's nice!", 
            "Weather talk is always interesting.", 
            "Climate can be quite fascinating."
        },
        .keyword_count = 4,
        .response_count = 3
    },
    {
		// who are you / your name 
		.keywords = { "who are you", "what is your name", "your name is", "domx", "your name" },
		.responses = {
			"My name is domx.", 
			"My screen name is domx.",
			"domx is my name.",
			"My name is domx that what people call me.", 
			"Well then my name is domx.", 
		},
		.keyword_count = 5,
		.response_count = 5 
	},
	{
	// what are you 	
	.keywords = { "what are you", "are you real", "tell me about yourself"},
	.responses = {
		  "I am domx A reactive scripted smart terminal bot",
		  "I am a reactive form of A.I. or none human synththic generative content bot", 
		  "My name is domx I am A helpful A.I. terminal bot"  
		},
		.keyword_count = 3,
		.response_count = 3	
    },
    
};

// Utility function to convert string to lowercase
void to_lowercase(char *str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Dynamic response generator
char* generate_response(char *input) {
    // Convert input to lowercase for matching
    to_lowercase(input);

    // Random seed for varied responses
    srand(time(NULL));

    // Iterate through topics
    for (int i = 0; i < sizeof(topics) / sizeof(topics[0]); i++) {
        ConversationTopic *topic = &topics[i];

        // Check for keyword matches
        for (int j = 0; j < topic->keyword_count; j++) {
            if (strstr(input, topic->keywords[j]) != NULL) {
                // Return a random response from the matched topic
                int response_index = rand() % topic->response_count;
                return topic->responses[response_index];
            }
        }
    }

    // Fallback responses for unrecognized input
    char *default_responses[] = {
        "I'm not sure I understand. Could you elaborate?",
        "That's interesting. Tell me more.",
        "I'm still learning. Can you rephrase that?",
        "Hmm, could you explain that differently?"
    };

    return default_responses[rand() % 4];
}

// Advanced input cleaning
void clean_input(char *input) {
    // Remove leading and trailing whitespaces
    char *start = input;
    char *end = input + strlen(input) - 1;

    while (isspace(*start)) start++;
    while (end > start && isspace(*end)) end--;

    *(end + 1) = '\0';
    memmove(input, start, end - start + 2);

    // Remove punctuation if needed
    for (int i = 0; input[i]; i++) {
        if (ispunct(input[i])) {
            memmove(&input[i], &input[i+1], strlen(&input[i+1]) + 1);
            i--;
        }
    }
}

// Conversation loop with enhanced features
//void start_conversation() {
    //char input[MAX_INPUT];
    //int conversation_count = 0;
    
    //printf("Dynamic Conversation Bot\n");
    //printf("Type 'quit' or 'exit' to end the conversation\n");

    //while (1) {
        //// Increment conversation counter
        //conversation_count++;

        //// Dynamic prompt with conversation context
        //printf("\n[Conversation #%d] You: ", conversation_count);
        
        //// Get user input
        //if (fgets(input, sizeof(input), stdin) == NULL) {
            //break;
        //}

        //// Remove newline character
        //input[strcspn(input, "\n")] = 0;

        //// Clean input
        //clean_input(input);

        //// Check for exit conditions
        //if (strcmp(input, "quit") == 0 || 
            //strcmp(input, "exit") == 0 || 
            //strcmp(input, "bye") == 0) {
            
            //// Farewell messages with randomness
            //char *farewell_messages[] = {
                //"Goodbye! Hope to chat again soon.",
                //"See you later! Take care.",
                //"It was nice talking to you.",
                //"Farewell! Have a great day!"
            //};
            
            //printf("%s\n", farewell_messages[rand() % 4]);
            //break;
        //}

        //// Skip empty inputs
        //if (strlen(input) == 0) {
            //printf("Please type something.\n");
            //continue;
        //}

        //// Generate and print response
        //char *response = generate_response(input);
        //printf("Bot: %s\n", response);

        //// Optional: Add some conversation context tracking
        //if (conversation_count % 5 == 0) {
            //printf("\n[Bot Note: We've been chatting for a while!]\n");
        //}
    //}

    //// Conversation summary
    //printf("\nConversation ended. Total exchanges: %d\n", conversation_count);
//}
// Notes: 
// Context tracking and learning (placeholder for advanced implementation)
    // In a more advanced version, this could:
    // 1. Save conversation logs
    // 2. Learn from user interactions
    // 3. Improve response generation

// Cleanup and close database
void cleanup_database(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

// Main function
int main() {
    // Initialize random seed
    srand(time(NULL)*2);
    // Welcome message with some randomness
    char *welcome_messages[] = {
        "Welcome to the Dynamic Conversation Bot!",
        "Hi there! Ready to chat?",
        "Hello! I'm your conversational companion.",
        "Greetings! Let's have an interesting chat."
    };
    printf("%s\n", welcome_messages[rand() % 4]);

    // Start the conversation
    // start_conversation();

   // Initialize SQLite database
    sqlite3 *db = initialize_database();
    if (!db) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }

    // Create additional learning tables if not exists
    const char *learning_table_sql = 
        "CREATE TABLE IF NOT EXISTS response_patterns ("
        "input_pattern TEXT PRIMARY KEY,"
        "response_count INTEGER DEFAULT 1,"
        "last_used DATETIME"
        ");";

    char *err_msg = 0;
    int rc = sqlite3_exec(db, learning_table_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    // Welcome message
    printf("=== Adaptive Conversation Bot ===\n");
    printf("A conversational AI that learns from interactions\n");
    printf("Type 'quit' or 'exit' to end the conversation\n\n");

    // Start conversation
    start_conversation(db);

    // Cleanup
    cleanup_database(db);

    return 0;
}

