#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define INPUT_SIZE 10
#define HIDDEN_SIZE 5
#define OUTPUT_SIZE 5  // 5 different intent types
#define MAX_INPUT_LENGTH 256
#define LEARNING_RATE 0.1
#define MAX_INPUT 256
#define MAX_COMMANDS 25

// Whitelist of allowed swarm commands
const char* allowed_commands[] = {
    "tu", "ls", "al", "cl", "oz",
    "zx", "dl", "zv",  "zp", "fs",
    "me", "ox", "pc", "up", "eo",
    "date", "whoami", "cls",
    "uname -a", "df -h", "clear",
    "free -h", "ps aux", "zs",
    "top -n 1", "echo 'Hello World'"
};

// Function to trim whitespace
void trim_whitespace(char* str) {
    char* end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    // All spaces?
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    end[1] = '\0';
}

// Function to validate and execute command
int execute_safe_command(const char* input) {
    // Check if input matches any allowed command
    for(int i = 0; i < MAX_COMMANDS; i++) {
        if(strcmp(input, allowed_commands[i]) == 0) {
            printf("Executing: %s\n", input);
            printf("Output:\n");
            return system(input);
        }
    }
    
    printf("Command '%s' is not in the allowed list.\n", input);
    printf("Allowed commands:\n");
    for(int i = 0; i < MAX_COMMANDS; i++) {
        printf("  %s\n", allowed_commands[i]);
    }
    return -1;
}

// Intent types for pattern recognition
typedef enum {
    INTENT_GREETING = 0,
    INTENT_QUESTION = 1,
    INTENT_MATH = 2,
    INTENT_GOODBYE = 3,
    INTENT_UNKNOWN = 4
} IntentType;

const char* intent_names[] = {
    "Greeting", "Question", "Math", "Goodbye", "Unknown"
};

typedef struct {
    int input_size;
    int hidden_size;
    int output_size;
    double *weights_input_hidden;
    double *weights_hidden_output;
    double *hidden_bias;
    double *output_bias;
} NeuralNetwork;

// Function prototypes
double sigmoid(double x);
double sigmoid_derivative(double x);
void extract_features(const char *input, double *features);
NeuralNetwork* create_neural_network(int input_size, int hidden_size, int output_size);
void forward_propagate(NeuralNetwork *nn, double *input, double *hidden, double *output);
IntentType classify_intent(NeuralNetwork *nn, const char *input);
void generate_response_by_intent(IntentType intent, const char *input, char *response);
double solve_simple_math(const char *input, char *explanation);
int create_database();
void store_input_with_intent(const char *user_input, const char *bot_response, IntentType intent);
void chat_with_user(NeuralNetwork *nn);
int search_database_for_response(const char *user_input, char *found_response, IntentType *found_intent);
int calculate_similarity(const char *str1, const char *str2);
void display_database_contents();
void enhanced_chat_with_user(NeuralNetwork *nn);
void learning_chat_with_user(NeuralNetwork *nn);
void database_only_chat(NeuralNetwork *nn);

// Sigmoid activation function
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

// Derivative of sigmoid
double sigmoid_derivative(double x) {
    return x * (1.0 - x);
}


// Create neural network with bias terms
NeuralNetwork* create_neural_network(int input_size, int hidden_size, int output_size) {
    NeuralNetwork *nn = malloc(sizeof(NeuralNetwork));
    if (nn == NULL) {
        fprintf(stderr, "OZ: Failed to allocate memory for neural network.\n");
        return NULL;
    }

    nn->input_size = input_size;
    nn->hidden_size = hidden_size;
    nn->output_size = output_size;

    // Allocate memory for weights and biases
    nn->weights_input_hidden = malloc(input_size * hidden_size * sizeof(double));
    nn->weights_hidden_output = malloc(hidden_size * output_size * sizeof(double));
    nn->hidden_bias = malloc(hidden_size * sizeof(double));
    nn->output_bias = malloc(output_size * sizeof(double));

    if (nn->weights_input_hidden == NULL || nn->weights_hidden_output == NULL ||
        nn->hidden_bias == NULL || nn->output_bias == NULL) {
        fprintf(stderr, "OZ: Failed to allocate memory for weights.\n");
        free(nn);
        return NULL;
    }

    // Initialize weights and biases randomly
    for (int i = 0; i < input_size * hidden_size; i++) {
        nn->weights_input_hidden[i] = ((double)rand() / RAND_MAX) * 2 - 1;
    }
    for (int i = 0; i < hidden_size * output_size; i++) {
        nn->weights_hidden_output[i] = ((double)rand() / RAND_MAX) * 2 - 1;
    }
    for (int i = 0; i < hidden_size; i++) {
        nn->hidden_bias[i] = ((double)rand() / RAND_MAX) * 2 - 1;
    }
    for (int i = 0; i < output_size; i++) {
        nn->output_bias[i] = ((double)rand() / RAND_MAX) * 2 - 1;
    }

    return nn;
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

// Forward propagation with bias
void forward_propagate(NeuralNetwork *nn, double *input, double *hidden, double *output) {
    // Calculate hidden layer activations
    for (int i = 0; i < nn->hidden_size; i++) {
        hidden[i] = nn->hidden_bias[i];
        for (int j = 0; j < nn->input_size; j++) {
            hidden[i] += input[j] * nn->weights_input_hidden[j * nn->hidden_size + i];
        }
        hidden[i] = sigmoid(hidden[i]);
    }

    // Calculate output layer activations
    for (int i = 0; i < nn->output_size; i++) {
        output[i] = nn->output_bias[i];
        for (int j = 0; j < nn->hidden_size; j++) {
            output[i] += hidden[j] * nn->weights_hidden_output[j * nn->output_size + i];
        }
        output[i] = sigmoid(output[i]);
    }
}

// Improved classify user intent using rule-based logic with neural network backup
IntentType classify_intent(NeuralNetwork *nn, const char *input) {
    double features[INPUT_SIZE];
    double hidden[HIDDEN_SIZE];
    double output[OUTPUT_SIZE];
    
    // Extract features from input text
    extract_features(input, features);
    
    // Print debug information
    printf("OZ Debug: Features extracted: ");
    for (int i = 0; i < INPUT_SIZE; i++) {
        printf("%.2f ", features[i]);
    }
    printf("\n");
    
    // Rule-based classification (more reliable than untrained network)
    IntentType detected_intent = INTENT_UNKNOWN;
    
    // 1. Check for MATH first (highest priority when clear)
    if (features[2] > 0 && features[3] > 0) {
        // Has both numbers AND math operators/words
        detected_intent = INTENT_MATH;
        printf("OZ Debug: Rule-based detection: MATH (numbers + operators)\n");
    }
    // 2. Check for strong math signals even without numbers visible
    else if (features[3] > 0 && (strstr(input, "calculate") || strstr(input, "solve") || 
                                 strstr(input, "plus") || strstr(input, "minus") ||
                                 strstr(input, "times") || strstr(input, "multiply") ||
                                 strstr(input, "divide") || strstr(input, "equals"))) {
        detected_intent = INTENT_MATH;
        printf("OZ Debug: Rule-based detection: MATH (strong math words)\n");
    }
    // 3. Check for GOODBYE
    else if (features[4] > 0) {
        detected_intent = INTENT_GOODBYE;
        printf("OZ Debug: Rule-based detection: GOODBYE\n");
    }
    // 4. Check for GREETING
    else if (features[0] > 0) {
        detected_intent = INTENT_GREETING;
        printf("OZ Debug: Rule-based detection: GREETING\n");
    }
    // 5. Check for QUESTION
    else if (features[1] > 0) {
        detected_intent = INTENT_QUESTION;
        printf("OZ Debug: Rule-based detection: QUESTION\n");
    }
    // 6. If no clear rule-based match, use neural network as backup
    else {
        // Run forward propagation
        forward_propagate(nn, features, hidden, output);
        
        // Find the output with highest activation
        int max_index = 0;
        for (int i = 1; i < OUTPUT_SIZE; i++) {
            if (output[i] > output[max_index]) {
                max_index = i;
            }
        }
        
        detected_intent = (IntentType)max_index;
        printf("OZ Debug: Neural network classification: %s\n", intent_names[detected_intent]);
        
        // Print network outputs for debugging
        printf("OZ Debug: Network outputs: ");
        for (int i = 0; i < OUTPUT_SIZE; i++) {
            printf("%s:%.3f ", intent_names[i], output[i]);
        }
        printf("\n");
    }
    
    return detected_intent;
}

// Enhanced feature extraction with better word boundary detection
void extract_features(const char *input, double *features) {
    // Initialize all features to 0
    for (int i = 0; i < INPUT_SIZE; i++) {
        features[i] = 0.0;
    }
    
    int len = strlen(input);
    if (len == 0) return;
    
    // Convert to lowercase for easier matching
    char lower_input[MAX_INPUT_LENGTH];
    for (int i = 0; i < len && i < MAX_INPUT_LENGTH - 1; i++) {
        lower_input[i] = tolower(input[i]);
    }
    lower_input[len] = '\0';
    
    // Feature 0: Contains greeting words
    if (strstr(lower_input, "HI") || strstr(lower_input, "Hi") ||
        strstr(lower_input, "hello") || strstr(lower_input, "hey") || 
        strstr(lower_input, "good morning") || strstr(lower_input, "good afternoon") ||
        strstr(lower_input, "good evening") || strstr(lower_input, "good day") ||
        // Check for "hi" as complete word
        (strstr(lower_input, " hi ") != NULL) ||
        (strncmp(lower_input, "hi ", 3) == 0) ||
        (strcmp(lower_input, "hi") == 0) ||
        (len >= 3 && strcmp(lower_input + len - 3, " hi") == 0)) {
        features[0] = 1.0;
    }
    
    // Feature 1: Contains question words
    if (strstr(lower_input, "ok") || strstr(lower_input, "OK") ||
        strstr(lower_input, "question") || strstr(lower_input, "Question") ||
        strstr(lower_input, "what") || strstr(lower_input, "how") || 
        strstr(lower_input, "why") || strstr(lower_input, "when") ||
        strstr(lower_input, "where") || strstr(lower_input, "who") ||
        strstr(lower_input, "if") || strstr(lower_input, "while") ||
        strstr(lower_input, "which") || strstr(lower_input, "?")) {
        features[1] = 1.0;
    }
    
    // Feature 2: Contains numbers
    for (int i = 0; i < len; i++) {
        if (isdigit(lower_input[i])) {
            features[2] = 1.0;
            break;
        }
    }
    
    // Feature 3: Contains math operators or math words
    if (strstr(lower_input, "+") || strstr(lower_input, "-") || 
        strstr(lower_input, "*") || strstr(lower_input, "/") ||
        strstr(lower_input, "plus") || strstr(lower_input, "minus") ||
        strstr(lower_input, "multiply") || strstr(lower_input, "divide") ||
        strstr(lower_input, "times") || strstr(lower_input, "equals") ||
        strstr(lower_input, "calculate") || strstr(lower_input, "solve") ||
        strstr(lower_input, "add") || strstr(lower_input, "subtract")) {
        features[3] = 1.0;
    }
    
    // Feature 4: Contains goodbye words - with proper word boundaries
    if (strstr(lower_input, "goodbye") || strstr(lower_input, "see you") ||
        strstr(lower_input, "farewell") || strstr(lower_input, "take care") ||
        strstr(lower_input, "good bye") || strstr(lower_input, "bye-bye!") ||
        // Check for "bye" as complete word only
        (strstr(lower_input, " bye ") != NULL) ||
        (strncmp(lower_input, "bye ", 4) == 0) ||
        (strcmp(lower_input, "bye") == 0) ||
        (len >= 4 && strcmp(lower_input + len - 4, " bye") == 0)) {
        features[4] = 1.0;
    }
    
    // Feature 5: Input length (normalized)
    features[5] = (double)len / 100.0;
    
    // Feature 6: Number of words
    int word_count = 1;
    for (int i = 0; i < len; i++) {
        if (lower_input[i] == ' ') word_count++;
    }
    features[6] = (double)word_count / 10.0;
    
    // Feature 7: Exclamation marks
    if (strstr(lower_input, "!")) {
        features[7] = 1.0;
    }
    
    // Feature 8: Contains "please", "thank" and/or welcome
    if (strstr(lower_input, "please") || strstr(lower_input, "thank") ||
        strstr(lower_input, "welcome") || strstr(lower_input, "Welcome")  ) {
        features[8] = 1.0;
    }
    
    // Feature 9: Bias feature (always 1.0)
    features[9] = 1.0;
}

// Generate response based on recognized intent
void generate_response_by_intent(IntentType intent, const char *input, char *response) {
    switch (intent) {
        case INTENT_GREETING:
            // Array of greetings
    const char *greetings[] = {
        "OZ: Hello, World!",
        "OZ: Hi there!",
        "OZ: Greetings!",
        "OZ: Welcome!",
        "OZ: How are you doing?",
        "OZ: Good day!",
        "OZ: Howdy!",
        "OZ: Salutations!",
        "OZ: What's up!"
    };
    
             // Get the number of greetings
             int numGreetings = sizeof(greetings) / sizeof(greetings[0]);
             // Seed the random number generator
             srand(time(NULL));
            // Generate a random index
            int randomIndex = rand() % numGreetings;
            strcpy(response, greetings[randomIndex]);
            break;
        case INTENT_QUESTION:
            // Array of questions
    const char *questions[] = {
        "That is one interesting question.",
        "How are you today?",
        "What was your question?",
        "Interesting..",
        "I am still learning..",
        "What?",
        "Questions?",
        "Weird question?"
    };
            // Get the number of questions
            int numQuestions = sizeof(questions) / sizeof(questions[0]);
            // Seed the random number generator
            srand(time(NULL));
            // Generate a random index
            int randomIndex2 = rand() % numQuestions;
            // Create a buffer to hold the selected question
            char selectedQuestion[100]; // Ensure this is large enough to hold the longest question
            // Copy the random question into the buffer
            strcpy(selectedQuestion, questions[randomIndex2]);
            strcpy(response, selectedQuestion);
            break;
        case INTENT_MATH:
            {
                // Try to solve the math problem
                char math_explanation[MAX_INPUT_LENGTH];
                double result = solve_simple_math(input, math_explanation);
                
                if (strlen(math_explanation) > 0 && strstr(math_explanation, "Error:") == NULL && strstr(math_explanation, "couldn't") == NULL) {
                    snprintf(response, MAX_INPUT_LENGTH, "Let me solve that for you: %s", math_explanation);
                } else {
                    // Fallback responses for different math operations
                    if (strstr(input, "+") || strstr(input, "plus")) {
                        snprintf(response, MAX_INPUT_LENGTH, "I can see this is addition! %s", math_explanation);
                    } else if (strstr(input, "-") || strstr(input, "minus")) {
                        snprintf(response, MAX_INPUT_LENGTH, "This looks like subtraction! %s", math_explanation);
                    } else if (strstr(input, "*") || strstr(input, "times") || strstr(input, "multiply")) {
                        snprintf(response, MAX_INPUT_LENGTH, "I detect multiplication here! %s", math_explanation);
                    } else if (strstr(input, "/") || strstr(input, "divide")) {
                        snprintf(response, MAX_INPUT_LENGTH, "This appears to be division! %s", math_explanation);
                    } else {
                        snprintf(response, MAX_INPUT_LENGTH, "I see numbers and math-related words! %s", math_explanation);
                    }
                }
            }
            break;
        case INTENT_GOODBYE:
            strcpy(response, "OZ: Goodbye! It was nice talking with you. Have a great day!");
            break;
        case INTENT_UNKNOWN:
        default:
            strcpy(response,"OZ: I'm not sure I understand. Could you rephrase that?");
            break;
    }
}

// Function to create the SQLite database
int create_database() {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open("neural_network.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: I Cannot open the database %s\n", sqlite3_errmsg(db));
        return rc;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS Responses (Id INTEGER PRIMARY KEY, UserInput TEXT UNIQUE, BotResponse TEXT, Intent INTEGER);";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Showing SQL debug info, \nSQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
    return 0;
}

// Function to store user input with detected intent
void store_input_with_intent(const char *user_input, const char *bot_response, IntentType intent) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int rc = sqlite3_open("neural_network.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Cannot open database %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "INSERT INTO Responses(UserInput, BotResponse, Intent) VALUES(?, ?, ?) ON CONFLICT(UserInput) DO UPDATE SET BotResponse=?, Intent=?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Failed to prepare statement %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bot_response, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, (int)intent);
    sqlite3_bind_text(stmt, 4, bot_response, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, (int)intent);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "OZ: Execution failed %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Enhanced chat function with pattern recognition
void chat_with_user(NeuralNetwork *nn) {
    char user_input[MAX_INPUT_LENGTH];
    char bot_response[MAX_INPUT_LENGTH];

    printf("OZ: Hello! I'm OZ, your AI assistant.\n");
    printf("OZ: I can recognize greetings, questions, math problems, and goodbyes.\n");
    printf("OZ: Try saying something like 'Hello', 'What is...?', '2 + 2', or 'goodbye'.\n\n");

    while (1) {
        printf(">> ");
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = 0; // Remove newline character
        // Trim whitespace
        trim_whitespace(user_input); 
        
        // Check for run command
        if(strncmp(user_input, "run ", 4) == 0) {
            char* command = user_input + 4; // Skip "run "
            trim_whitespace(command);
            
            if(strlen(command) == 0) {
                printf("Please specify a command after 'run'.\n");
                continue;
            }
            
            execute_safe_command(command);
        }
        else if(strlen(user_input) > 0) {
            printf("\n");
        } 
        // Check for exit commands first
        if (strcmp(user_input, "quit") == 0 || 
            strcmp(user_input, "exit") == 0 || 
            strcmp(user_input, "bye") == 0) {
            printf("OZ: Goodbye! Thanks for testing my pattern recognition!\n");
            break;
        }

        // Use neural network to classify the intent
        IntentType detected_intent = classify_intent(nn, user_input);
        
        printf("OZ: I detected this as a '%s' pattern.\n", intent_names[detected_intent]);
        
        // Generate response based on detected intent
        generate_response_by_intent(detected_intent, user_input, bot_response);
        
        // Store the interaction in database
        store_input_with_intent(user_input, bot_response, detected_intent);
        
        printf("OZ: %s\n\n", bot_response);
    }
}


// Function to search for similar user inputs in the database
int search_database_for_response(const char *user_input, char *found_response, IntentType *found_intent) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int found = 0;

    int rc = sqlite3_open("neural_network.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Cannot open database %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // First, try exact match
    const char *exact_sql = "SELECT BotResponse, Intent FROM Responses WHERE UserInput = ? LIMIT 1;";
    rc = sqlite3_prepare_v2(db, exact_sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *response = (const char*)sqlite3_column_text(stmt, 0);
            int intent = sqlite3_column_int(stmt, 1);
            
            strcpy(found_response, response);
            *found_intent = (IntentType)intent;
            found = 1;
            printf("OZ Debug: Found exact match in database!\n");
        }
        sqlite3_finalize(stmt);
    }

    // If no exact match, try fuzzy matching (similar inputs)
    if (!found) {
        const char *fuzzy_sql = "SELECT UserInput, BotResponse, Intent FROM Responses;";
        rc = sqlite3_prepare_v2(db, fuzzy_sql, -1, &stmt, 0);
        if (rc == SQLITE_OK) {
            int best_similarity = 0;
            char best_response[MAX_INPUT_LENGTH] = "";
            IntentType best_intent = INTENT_UNKNOWN;
            
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *stored_input = (const char*)sqlite3_column_text(stmt, 0);
                const char *stored_response = (const char*)sqlite3_column_text(stmt, 1);
                int stored_intent = sqlite3_column_int(stmt, 2);
                
                int similarity = calculate_similarity(user_input, stored_input);
                if (similarity > best_similarity && similarity > 70) { // 70% similarity threshold
                    best_similarity = similarity;
                    strcpy(best_response, stored_response);
                    best_intent = (IntentType)stored_intent;
                }
            }
            
            if (best_similarity > 70) {
                strcpy(found_response, best_response);
                *found_intent = best_intent;
                found = 1;
                printf("OZ Debug: Found similar match in database! (Similarity: %d%%)\n", best_similarity);
            }
            sqlite3_finalize(stmt);
        }
    }

    sqlite3_close(db);
    return found;
}

// Simple similarity calculation function
int calculate_similarity(const char *str1, const char *str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    
    if (len1 == 0 && len2 == 0) return 100;
    if (len1 == 0 || len2 == 0) return 0;
    
    // Convert to lowercase for comparison
    char lower1[MAX_INPUT_LENGTH], lower2[MAX_INPUT_LENGTH];
    for (int i = 0; i < len1 && i < MAX_INPUT_LENGTH - 1; i++) {
        lower1[i] = tolower(str1[i]);
    }
    lower1[len1] = '\0';
    
    for (int i = 0; i < len2 && i < MAX_INPUT_LENGTH - 1; i++) {
        lower2[i] = tolower(str2[i]);
    }
    lower2[len2] = '\0';
    
    // Count common words
    int total_words = 0;
    int common_words = 0;
    
    char *str1_copy = malloc(strlen(lower1) + 1);
    strcpy(str1_copy, lower1);
    
    char *token = strtok(str1_copy, " \t\n");
    while (token != NULL) {
        total_words++;
        if (strstr(lower2, token) != NULL) {
            common_words++;
        }
        token = strtok(NULL, " \t\n");
    }
    
    free(str1_copy);
    
    if (total_words == 0) return 0;
    return (common_words * 100) / total_words;
}

// Function to get all responses from database with enhanced formatting
void display_database_contents() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int rc = sqlite3_open("neural_network.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Cannot open database %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "SELECT Id, UserInput, BotResponse, Intent FROM Responses ORDER BY Id;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Failed to prepare statement %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *user_input = (const char*)sqlite3_column_text(stmt, 1);
        const char *bot_response = (const char*)sqlite3_column_text(stmt, 2);
        int intent = sqlite3_column_int(stmt, 3);        
        printf("\n[Entry #%d]\n", id);
        printf("USER INPUT:    \"%s\"\n", user_input);
        printf("DATABASE TEXT: \"%s\"\n", bot_response);
        printf("INTENT TYPE:   %s\n", intent_names[intent]);
        count++;
    }
    
    if (count == 0) {
        printf("OZ: No responses stored in database yet.\n");
    } else {
        printf("Total stored responses: %d\n", count);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// New function: Database-only chat mode (only uses stored responses)
void database_only_chat(NeuralNetwork *nn) {
    char user_input[MAX_INPUT_LENGTH];
    char db_response[MAX_INPUT_LENGTH];

    printf("OZ: DATABASE-ONLY MODE ACTIVATED\n");
    printf("OZ: I will ONLY respond with text stored in my database.\n");
    printf("OZ: If I don't have a stored response, I'll tell you.\n");
    printf("OZ: Commands: 'show database', 'quit'\n\n");

    while (1) {
        printf(">> ");
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = 0;
       // Trim whitespace
        trim_whitespace(user_input);
        
        // Check for run command
        if(strncmp(user_input, "run ", 4) == 0) {
            char* command = user_input + 4; // Skip "run "
            trim_whitespace(command);
            
            if(strlen(command) == 0) {
                printf("Please specify a command after 'run'.\n");
                continue;
            }
            
            execute_safe_command(command);
        }
        else if(strlen(user_input) > 0) {
            printf("Unknown command. Type 'run <command>' or 'quit'.\n");
        }
        
        if (strcmp(user_input, "quit") == 0 || strcmp(user_input, "exit") == 0) {
            printf("OZ: Goodbye from database-only mode!\n");
            break;
        }
        if (strcmp(user_input, "wake") == 0) { 
			system("zs"); 
		}
        if (strcmp(user_input, "show database") == 0) {
            display_database_contents();
            continue;
        }

        // Only check database, don't generate new responses
        IntentType db_intent;
        int found_in_db = search_database_for_response(user_input, db_response, &db_intent);
        
        if (found_in_db) {
            printf("RETRIEVED TEXT: \"%s\"\n", db_response);
            printf("ORIGINAL INTENT: %s\n", intent_names[db_intent]);
            printf("OZ: %s\n\n", db_response);
        } else {
            printf("OZ: I don't have any stored response for: \"%s\"\n", user_input);
            printf("OZ: Try asking something I've answered before, or switch to enhanced mode.\n\n");
        }
    }
}

// Enhanced chat function that shows database responses prominently
void enhanced_chat_with_user(NeuralNetwork *nn) {
    char user_input[MAX_INPUT_LENGTH];
    char bot_response[MAX_INPUT_LENGTH];
    char db_response[MAX_INPUT_LENGTH];
    printf("OZ: Hello! I'm OZ with database response capabilities.\n");
    printf("OZ: I will show you exactly what I retrieve from my memory database.\n");
    printf("OZ: Special commands: 'show database' to see all stored responses, 'quit' to exit.\n\n");
    while (1) {
        printf(">> ");
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = 0; // Remove newline
       // Trim whitespace
        trim_whitespace(user_input);
        
        // Check for run command
        if(strncmp(user_input, "run ", 4) == 0) {
            char* command = user_input + 4; // Skip "run "
            trim_whitespace(command);
            
            if(strlen(command) == 0) {
                printf("Please specify a command after 'run'.\n");
                continue;
            }
            
            execute_safe_command(command);
        }
        else if(strlen(user_input) > 0) {
            printf("Unknown command. Type 'run <command>' or 'quit'.\n");
        }
        
        // Check for special commands
        if (strcmp(user_input, "quit") == 0 || strcmp(user_input, "exit") == 0) {
            printf("OZ: Goodbye! Thanks for testing my database memory!\n");
            break;
        }
        if (strcmp(user_input, "wake") == 0 || strcmp(user_input, "WAKE") == 0 ) { 
			system("zs"); 
		} else if (strcmp(user_input, "sleep") == 0) {
			system("zv"); 
		}
		 
        if (strcmp(user_input, "show database") == 0) {
            display_database_contents();
            continue;
        }

        // First, check if we have a response in the database
        IntentType db_intent;
        int found_in_db = search_database_for_response(user_input, db_response, &db_intent);
        
        if (found_in_db) {
            printf("\n=== RETRIEVED FROM DATABASE ===\n");
            printf("Input Query: \"%s\"\n", user_input);
            printf("Found Intent: %s\n", intent_names[db_intent]);
            printf("Database Response: \"%s\"\n", db_response);
            printf("================================\n");
            printf("OZ: %s\n\n", db_response);
            
            // Update the database with the reused response
            store_input_with_intent(user_input, db_response, db_intent);
        } else {
            printf("\n=== NO DATABASE MATCH - GENERATING NEW ===\n");
            // Use neural network to classify and generate new response
            IntentType detected_intent = classify_intent(nn, user_input);
            printf("New Input: \"%s\"\n", user_input);
            printf("Detected Intent: %s\n", intent_names[detected_intent]);
            
            generate_response_by_intent(detected_intent, user_input, bot_response);
            printf("Generated Response: \"%s\"\n", bot_response);
            printf("(This response will be stored in database)\n");
            printf("==========================================\n");
            
            // Store the new interaction
            store_input_with_intent(user_input, bot_response, detected_intent);
            
            printf("OZ: %s\n\n", bot_response);
        }
    }
}

// Function to add learning capability - shows database retrieval process
void learning_chat_with_user(NeuralNetwork *nn) {
    char user_input[MAX_INPUT_LENGTH];
    char bot_response[MAX_INPUT_LENGTH];
    char db_response[MAX_INPUT_LENGTH];
    char user_feedback[MAX_INPUT_LENGTH];

    printf("OZ: Hello! I'm OZ with transparent database learning.\n");
    printf("OZ: I'll show you exactly what I retrieve from my database.\n");
    printf("OZ: After each response, you can:\n");
    printf("OZ: - Press Enter to continue\n");
    printf("OZ: - Type 'correct: [better response]' to teach me\n");
    printf("OZ: - Type 'show database' to see all stored responses\n\n");

    while (1) {
        printf(">> ");
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = 0;
       // Trim whitespace
        trim_whitespace(user_input);
        
        // Check for run command
        if(strncmp(user_input, "run ", 4) == 0) {
            char* command = user_input + 4; // Skip "run "
            trim_whitespace(command);
            
            if(strlen(command) == 0) {
                printf("Please specify a command after 'run'.\n");
                continue;
            }
            
            execute_safe_command(command);
        }
        else if(strlen(user_input) > 0) {
            printf("Unknown command. Type 'run <command>' or 'quit'.\n");
        }
           if (strcmp(user_input, "wake") == 0 || strcmp(user_input, "WAKE") == 0 ) { 
			system("zs"); 
		} else if (strcmp(user_input, "sleep") == 0) {
			system("zv"); 
		}
        if (strcmp(user_input, "quit") == 0 || strcmp(user_input, "exit") == 0) {
            printf("OZ: Goodbye! Thanks for teaching me!\n");
            break;
        }
        
        if (strcmp(user_input, "show database") == 0) {
            display_database_contents();
            continue;
        }

        // Check database first
        IntentType db_intent;
        int found_in_db = search_database_for_response(user_input, db_response, &db_intent);
        
        if (found_in_db) {
            printf("\n=== RETRIEVED FROM DATABASE ===\n");
            printf("Query: \"%s\"\n", user_input);
            printf("Found Intent: %s\n", intent_names[db_intent]);
            printf("Retrieved Text: \"%s\"\n", db_response);
            printf("================================\n");
            printf("OZ: %s\n", db_response);
        } else {
            printf("\n=== GENERATING NEW RESPONSE ===\n");
            IntentType detected_intent = classify_intent(nn, user_input);
            generate_response_by_intent(detected_intent, user_input, bot_response);
            printf("Input: \"%s\"\n", user_input);
            printf("Detected Intent: %s\n", intent_names[detected_intent]);
            printf("Generated Text: \"%s\"\n", bot_response);
            printf("(Storing in database...)\n");
            printf("===============================\n");
            printf("OZ: %s\n", bot_response);
            strcpy(db_response, bot_response);
            store_input_with_intent(user_input, bot_response, detected_intent);
        }
        
        // Ask for feedback
        printf("\nFeedback (Enter to continue, or 'correct: [better response]'): ");
        fgets(user_feedback, sizeof(user_feedback), stdin);
        user_feedback[strcspn(user_feedback, "\n")] = 0;
        
        // Check if user provided a correction
        if (strncmp(user_feedback, "correct:", 8) == 0) {
            char *corrected_response = user_feedback + 8;
            while (*corrected_response == ' ') corrected_response++; // Skip spaces
            
            if (strlen(corrected_response) > 0) {
                // Re-classify with the corrected response
                IntentType corrected_intent = classify_intent(nn, user_input);
                store_input_with_intent(user_input, corrected_response, corrected_intent);
                
                printf("\n=== LEARNING UPDATE ===\n");
                printf("Original Input: \"%s\"\n", user_input);
                printf("Old Response: \"%s\"\n", db_response);
                printf("New Response: \"%s\"\n", corrected_response);
                printf("Intent: %s\n", intent_names[corrected_intent]);
                printf("Status: Stored in database\n");
                printf("======================\n\n");
            }
        } else {
            printf("\n");
        }
    }
}


int main() {
	
    srand(time(NULL)); // Seed for random number generation

    printf("OZ: Initializing enhanced pattern recognition system...\n");

    // Create the SQLite database
    if (create_database() != 0) {
        fprintf(stderr, "OZ: Failed to create database.\n");
        return 1;
    }

    // Create the neural network
    NeuralNetwork *nn = create_neural_network(INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE);
    if (nn == NULL) {
        return 1;
    }

    printf("OZ: Neural network created with database learning capabilities!\n");
    printf("OZ: Choose your interaction mode:\n");
    printf("OZ: 1 - Basic chat (original)\n");
    printf("OZ: 2 - Enhanced chat (shows database retrieval)\n");
    printf("OZ: 3 - Learning chat (accepts corrections)\n");
    printf("OZ: 4 - Database-only chat (only stored responses)\n");
    printf("Enter choice (1-4): ");
    
    int choice;
    scanf("%d", &choice);
    getchar(); // Consume newline
    
    switch(choice) {
        case 1:
            chat_with_user(nn);
            break;
        case 2:
            enhanced_chat_with_user(nn);
            break;
        case 3:
            learning_chat_with_user(nn);
            break;
        case 4:
            database_only_chat(nn);
            break;
        default:
            printf("Invalid choice, using enhanced mode.\n");
            enhanced_chat_with_user(nn);
            break;
    }

    // Free allocated memory
    free(nn->weights_input_hidden);
    free(nn->weights_hidden_output);
    free(nn->hidden_bias);
    free(nn->output_bias);
    free(nn);

    return 0;
}
