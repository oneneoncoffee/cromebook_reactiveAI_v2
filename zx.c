#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sqlite3.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <time.h> 

// Define default structure for test story 
const char *story = "The sun was setting over the ocean, casting a warm golden light over the waves. The sound of the surf was soothing, and the smell of saltwater filled the air. A group of seagulls flew overhead, their cries echoing through the stillness. It was a peaceful scene, one that seemed to stretch on forever. As the sun dipped lower in the sky, the stars began to twinkle in the distance, like diamonds scattered across the velvet expanse.";
#define loops 2
// Define the structure for a neural network layer
typedef struct {
    int num_inputs;
    int num_outputs;
    double* weights;
    double* biases;
} Layer;

// Define the structure for a neural network
typedef struct {
    int num_layers;
    Layer* layers;
} NeuralNetwork;

// Function to create a new neural network layer
Layer* create_layer(int num_inputs, int num_outputs) {
    Layer* layer = (Layer*) malloc(sizeof(Layer));
    layer->num_inputs = num_inputs;
    layer->num_outputs = num_outputs;
    layer->weights = (double*) malloc(num_inputs * num_outputs * sizeof(double));
    layer->biases = (double*) malloc(num_outputs * sizeof(double));
    return layer;
}


// Function to insert a question and answer into the table
void insert_question_answer(sqlite3* db, const char* question, const char* answer) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO questions (question, answer) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, question, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, answer, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// Function to retrieve an answer based on a user's question
void get_answer(sqlite3* db, const char* question) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT answer FROM questions WHERE question = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, question, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* answer = sqlite3_column_text(stmt, 0);
        printf("OZ: %s\n", answer);
    } else {
        printf("No answer found for the given question.\n");
    }

    sqlite3_finalize(stmt);
}

// Function to create a new neural network
NeuralNetwork* create_neural_network(int num_layers, int* layer_sizes) {
    NeuralNetwork* network = (NeuralNetwork*) malloc(sizeof(NeuralNetwork));
    network->num_layers = num_layers;
    network->layers = (Layer*) malloc(num_layers * sizeof(Layer));
    for (int i = 0; i < num_layers; i++) {
        if (i == 0) {
            network->layers[i] = *create_layer(layer_sizes[0], layer_sizes[i + 1]);
        } else if (i == num_layers - 1) {
            network->layers[i] = *create_layer(layer_sizes[i], layer_sizes[i]);
        } else {
            network->layers[i] = *create_layer(layer_sizes[i], layer_sizes[i + 1]);
        }
    }
    return network;
}

// Function to train the neural network
void train_neural_network(NeuralNetwork* network, double* inputs, double* targets, int num_samples) {
    // Simple gradient descent algorithm
    for (int i = 0; i < num_samples; i++) {
        // Forward pass
        double* outputs = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            outputs[j] = 0;
            for (int k = 0; k < network->layers[0].num_inputs; k++) {
                outputs[j] += inputs[i * network->layers[0].num_inputs + k] * network->layers[0].weights[k * network->layers[0].num_outputs + j];
            }
            outputs[j] += network->layers[0].biases[j];
            outputs[j] = 1 / (1 + exp(-outputs[j])); // Sigmoid activation function
        }

        // Backward pass
        double* errors = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            errors[j] = targets[i * network->layers[0].num_outputs + j] - outputs[j];
        }

        // Weight update
        for (int j = 0; j < network->layers[0].num_inputs; j++) {
            for (int k = 0; k < network->layers[0].num_outputs; k++) {
                network->layers[0].weights[j * network->layers[0].num_outputs + k] += 0.1 * errors[k] * inputs[i * network->layers[0].num_inputs + j];
            }
        }

        free(outputs);
        free(errors);
    }
}
// Function to save the neural network to a SQLite database
void save_neural_network(NeuralNetwork* network, sqlite3* db) {
    // Create table to store neural network weights
    char* sql = "CREATE TABLE IF NOT EXISTS weights (layer INTEGER, input INTEGER, output INTEGER, weight REAL)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    // Insert weights into table
    for (int i = 0; i < network->num_layers; i++) {
        for (int j = 0; j < network->layers[i].num_inputs; j++) {
            for (int k = 0; k < network->layers[i].num_outputs; k++) {
                sql = "INSERT INTO weights VALUES (?, ?, ?, ?)";
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
                sqlite3_bind_int(stmt, 1, i);
                sqlite3_bind_int(stmt, 2, j);
                sqlite3_bind_int(stmt, 3, k);
                sqlite3_bind_double(stmt, 4, network->layers[i].weights[j * network->layers[i].num_outputs + k]);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

    // Create table to store neural network biases
    sql = "CREATE TABLE IF NOT EXISTS biases (layer INTEGER, output INTEGER, bias REAL)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    // Insert biases into table
    for (int i = 0; i < network->num_layers; i++) {
        for (int j = 0; j < network->layers[i].num_outputs; j++) {
            sql = "INSERT INTO biases VALUES (?, ?, ?)";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, i);
            sqlite3_bind_int(stmt, 2, j);
            sqlite3_bind_double(stmt, 3, network->layers[i].biases[j]);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // Create table to store questions and answers
    sql = "CREATE TABLE IF NOT EXISTS questions (question TEXT, answer TEXT)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

// Function to load the neural network from a SQLite database
void load_neural_network(NeuralNetwork* network, sqlite3* db) {
    // Load weights from table
    char* sql = "SELECT * FROM weights";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int layer = sqlite3_column_int(stmt, 0);
        int input = sqlite3_column_int(stmt, 1);
        int output = sqlite3_column_int(stmt, 2);
        double weight = sqlite3_column_double(stmt, 3);
        network->layers[layer].weights[input * network->layers[layer].num_outputs + output] = weight;
    }
    sqlite3_finalize(stmt);

    // Load biases from table
    sql = "SELECT * FROM biases";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int layer = sqlite3_column_int(stmt, 0);
        int output = sqlite3_column_int(stmt, 1);
        double bias = sqlite3_column_double(stmt, 2);
        network->layers[layer].biases[output] = bias;
    }
    sqlite3_finalize(stmt);
}

// Function to test the neural network
void test_neural_network(NeuralNetwork* network, double* inputs, double* expected_outputs, int num_samples) {
    int seed = time(NULL); 
    srand(seed); 
    printf("\nRunning neural network stats:\nRandom Seed:%ld\n", seed);
    for (int i = 0; i < num_samples; i++) {
        double* outputs = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            outputs[j] = 0;
            for (int k = 0; k < network->layers[0].num_inputs; k++) {
                outputs[j] += inputs[i * network->layers[0].num_inputs + k] * network->layers[0].weights[k * network->layers[0].num_outputs + j];
            }
            outputs[j] += network->layers[0].biases[j];
            outputs[j] = 1 / (1 + exp(-outputs[j])); // Sigmoid activation function
        }

        printf("Input: ");
        for (int j = 0; j < network->layers[0].num_inputs; j++) {
            printf("%f ", inputs[i * network->layers[0].num_inputs + j]);
        }
        printf("\n");

        printf("Expected Output: ");
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            printf("%f ", expected_outputs[i * network->layers[0].num_outputs + j]);
        }
        printf("\n");

        printf("Actual Output: ");
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            printf("%f ", outputs[j]);
        }
        printf("\n");

        free(outputs);
    }
    printf("Press enter to continue.."); 
    getchar(); 
}


// Function to generate a random number from /dev/urandom
uint64_t generateRandomNumber() {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    uint64_t randomNumber;
    if (read(fd, &randomNumber, sizeof(randomNumber)) != sizeof(randomNumber)) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd);
        // Remove the negative sign by taking the absolute value
    return (randomNumber & 0x7FFFFFFFFFFFFFFF);
}

// Callback function for SQLite queries
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0; // Suppress unused variable warning

    // Assuming we're selecting two numbers
    if (argc >= 2) {
        uint64_t num1 = strtoull(argv[0], NULL, 10);
        uint64_t num2 = strtoull(argv[1], NULL, 10);

        // Convert num1 and num2 to strings
        char num1Str[20];
        char num2Str[20];
        sprintf(num1Str, "%llu", num1);
        sprintf(num2Str, "%llu", num2);

        // Combine the two numbers as a string
        char combinedStr[41];
        sprintf(combinedStr, "%s%llu", num1Str, num2);

        printf("%s\n", combinedStr);
    }

    return 0;
}


void generateAndCombineRandomNumbers() {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    // Open the database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "OZ: Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Create table if it doesn't exist
    const char *sql = "CREATE TABLE IF NOT EXISTS numbers(id INTEGER PRIMARY KEY AUTOINCREMENT, number INTEGER)";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    // Check if the database has more than 250 numbers
    sql = "SELECT COUNT(*) FROM numbers";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        if (count > 250) {
            // Delete the oldest number
            sql = "DELETE FROM numbers WHERE id = (SELECT MIN(id) FROM numbers)";
            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "OZ: SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
    }

    sqlite3_finalize(stmt);

    // Generate and insert a new random number into the database
    uint64_t randomNumber = generateRandomNumber();

    sql = "INSERT INTO numbers (number) VALUES (?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int64(stmt, 1, randomNumber);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) { return; }

    sqlite3_finalize(stmt);

    // Select two random numbers from the database and display them as one big number
    sql = "SELECT number FROM numbers ORDER BY RANDOM() LIMIT 2";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Other error SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
}

void displayRandomCombinedNumber(sqlite3 *db) {
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT number FROM numbers ORDER BY RANDOM() LIMIT 2";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: SQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }

    uint64_t num1, num2;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (sqlite3_column_count(stmt) >= 1) {
            if (num1 == 0) {
                num1 = sqlite3_column_int64(stmt, 0);
            } else {
                num2 = sqlite3_column_int64(stmt, 0);
            }
        }
    }

    sqlite3_finalize(stmt);

    // Combine the two numbers
    char combinedStr[41];
    sprintf(combinedStr, "%llu%llu", num1, num2);

    printf("%s\n", combinedStr);
}

// Subfunction to retrieve and display a joke from the database
void tell_joke_2() {
    sqlite3 *db;
    int rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT joke FROM jokes ORDER BY RANDOM() LIMIT 1";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        printf("%s\n", sqlite3_column_text(stmt, 0));
    } else {
        printf("No jokes found\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Function to run the program
void run_program() {
    sqlite3* db;
    int rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

        // Function to create a table
    char* errorMessage;
    const char* sql = "CREATE TABLE IF NOT EXISTS questions("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "question TEXT NOT NULL,"
                       "answer TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
    }

    char question[256];
    char answer[256];
    int loopCount = 0;

    while (loopCount < 5) {
	    printf(">> ");
        fgets(question, 256, stdin);
        question[strcspn(question, "\n")] = 0; // Remove newline character
        get_answer(db, question);
        printf("\nOZ:Training mode active..");
        printf("Enter question: ");
        fgets(question, 256, stdin);
        question[strcspn(question, "\n")] = 0; // Remove newline character

        printf("Enter answer: ");
        fgets(answer, 256, stdin);
        answer[strcspn(answer, "\n")] = 0; // Remove newline character

        insert_question_answer(db, question, answer);
        loopCount++;
        printf("Loop %d completed. %d remaining.\n", loopCount, 5 - loopCount);
    }

    sqlite3_close(db);
}


 
// Function to respond to user questions
void respond_to_questions(NeuralNetwork* network, sqlite3* db) {
	system("zv"); 
    char question[256];
    printf(">> ");
    fgets(question, 256, stdin);
    question[strcspn(question, "\n")] = 0; // Remove newline character
    if (strcmp(question, "random number") == 0) {
	displayRandomCombinedNumber(db); 
	}
    if (strcmp(question, "exit") == 0 || strcmp(question, "quit") == 0) { exit(1); }
    if (strcmp(question, "tell a joke") == 0) {  tell_joke_2();  }
    if (strcmp(question, "joke?") == 0) {  tell_joke_2();  }
    if (strcmp(question, "joke please") == 0) {  tell_joke_2();  }
    if (strcmp(question, "Joke?") == 0) {  tell_joke_2();  }
    if (strncmp(question, "run ", 4) == 0) {
	char command[100];
	}  
    system("zv");	
    // Update the neural network with the new data
    double inputs[] = {0, 0, 0, 1, 1, 0, 1, 1};
    double targets[] = {0, 1, 1, 0};
    train_neural_network(network, inputs, targets, 4);
    // Save the updated neural network to the database
    save_neural_network(network, db);    
}

// Function to collect user data and update the neural network
void collect_data_and_update_neural_network(NeuralNetwork* network, sqlite3* db) {
    char question[256];
    printf(">> ");
    fgets(question, 256, stdin);
    question[strcspn(question, "\n")] = 0; // Remove newline character
	if (strcmp(question, "random number") == 0) {
	generateAndCombineRandomNumbers();
	displayRandomCombinedNumber(db); 
	}
    if (strcmp(question, "train") == 0) {
	char question2[256]; 
	printf("Enter a question: "); 
	fgets(question, 256, stdin); 
	question[strcspn(question, "\n")] = 0; 	
	char answer[256];
    printf("Enter an answer: ");
    fgets(answer, 256, stdin);
    answer[strcspn(answer, "\n")] = 0; // Remove newline character

    // Store user question and answer in the database
    char* sql = "INSERT INTO questions (question, answer) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, question, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, answer, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Update the neural network with the new data
    double inputs[] = {0, 0, 0, 1, 1, 0, 1, 1};
    double targets[] = {0, 1, 1, 0};
    train_neural_network(network, inputs, targets, 4);

    // Save the updated neural network to the database
    save_neural_network(network, db);
	}
        if (strncmp(question, "run ", 4) == 0) {
	char command[100];
	strcpy(command, question + 4); 
	system(command); 
	} else {
    if (strcmp(question, "exit") == 0 || strcmp(question, "Exit") == 0) { 
        return; 
        exit(1);
    }
    if (strcmp(question, "quit") == 0) { exit(1); }
    if (strcmp(question, "tell a joke") == 0) {  tell_joke_2();  }
    if (strcmp(question, "joke?") == 0) {  tell_joke_2();  }
    if (strcmp(question, "joke please") == 0) {  tell_joke_2();  }
    if (strcmp(question, "Joke?") == 0) {  tell_joke_2();  }
   }   

    char* sql = "SELECT answer FROM questions WHERE question = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, question, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char* answer = (char*) sqlite3_column_text(stmt, 0);
        printf("OZ: %s\n", answer);
    }

    // Update the neural network with the new data
    double inputs[] = {0, 0, 0, 1, 1, 0, 1, 1};
    double targets[] = {0, 1, 1, 0};
    train_neural_network(network, inputs, targets, 4);

    // Save the updated neural network to the database
    save_neural_network(network, db);
}

// Function to generate a truly random seed from a file
unsigned int generate_random_seed() {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    unsigned int seed;
    if (read(fd, &seed, sizeof(seed)) == sizeof(seed)) {
        // Success
        seed = (seed * 39 ^ seed) % 0xFFFFFFFF; 
    }

    close(fd);
    return seed;
}

// Function to create a table
void create_table(sqlite3* db) {
	
    char* sql = "CREATE TABLE IF NOT EXISTS jokes (id INTEGER PRIMARY KEY, joke TEXT)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);
    
}

void create_Table2(sqlite3* db) {
    // Function to create a table
    char* errorMessage;
    const char* sql = "CREATE TABLE IF NOT EXISTS questions("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "question TEXT NOT NULL,"
                       "answer TEXT NOT NULL);";

    int rc = sqlite3_exec(db, sql, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
    }
}

// Function to insert jokes into the training table
void insert_jokes(sqlite3* db) {
    char* jokes[] = {
        "Why did the cat join a band? Because it wanted to be the purr-cussionist!",
        "Why did the dog go to the vet? Because it was feeling ruff!",
        "What do you call a group of cats playing instruments? A mew-sical band!",
        "Why did the dog go to the gym? To get a paws-itive workout!",
        "What did the cat say when it was happy? I'm feline great!",
        "Why did the dog go to the beauty parlor? Because it wanted to get a paws-itively gorgeous haircut!",
        "What do you call a dog that does magic tricks? A labracadabrador!",
        "Why did the cat take a nap? Because it was paws-itively exhausted!",
        "What did the dog say when it was excited? I'm howlin' with excitement!",
        "Why did the cat go to the library? To read a paws-ome book!",
        "I told my wife she was drawing her eyebrows too high, she looked surprised.",
        "Why don't scientists trust atoms, because they make up everything.",
        "Why don't eggs tell jokes, they'd crack each other up.",
        "Why did the tomato turn red, because it saw the salad dressing.",
        "What do you call a fake noodle, an impasta.",
        "Why did the scarecrow win an award, because he was outstanding in his field.",
        "Why don't lobsters share, because they're shellfish.",
        "What do you call a can opener that doesn't work, a can't opener.",
        "I'm reading a book about anti-gravity, it's impossible to put down.",
        "I'm addicted to placebos, I could quit, but it wouldn't make a difference.",
        "I'm reading a book on the history of glue, I just can't seem to put it down.",
        "I went to a restaurant and the sign said, Breakfast Anytime. So I ordered French toast during the Renaissance.",
        "A man walked into a library and asked the librarian, Do you have any books on Pavlov's dogs and Schrödinger's cat? The librarian replied, It rings a bell, but I'm not sure if it's here or not.",
        "I'm a vegetarian because I love animals, but I'm also a hypocrite because I wear leather shoes.",
        "I'm not a morning person, I'm not a night person, I'm a whenever the coffee kicks in person.",
        "I went to a fire station and asked, Do you have any fires to put out? They said, No, we're all out.",
        "I'm on a diet where I only eat foods that are bad for me, it's called the I'm going to die anyway diet.",
        "I'm not arguing, I'm just explaining why I'm right, there's a difference."        
    };

    for (int i = 0; i < 10; i++) {
        char* sql = "INSERT INTO jokes (id, joke) VALUES (?, ?)";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_int(stmt, 1, i + 1);
        sqlite3_bind_text(stmt, 2, jokes[i], -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// Function to tell a random joke from the database
void tell_joke(sqlite3* db) {
    srand(time(NULL));
    int joke_id = rand() % 10 + 1;

    char* sql = "SELECT joke FROM jokes WHERE id = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, joke_id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char* joke = (char*) sqlite3_column_text(stmt, 0);
        printf("%s\n", joke);
    }
    sqlite3_finalize(stmt);
}

// Storyteller functions 
int checkStoryExists(sqlite3 *db, const char *story) {
    sqlite3_stmt *stmt;
    int rc;
    const char *sql = "SELECT * FROM stories WHERE story = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    rc = sqlite3_bind_text(stmt, 1, story, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 1; // story exists
    }
    sqlite3_finalize(stmt);
    return 0; // story does not exist
}

int insertStory(sqlite3 *db, const char *story) {
    int rc;
    const char *sql = "INSERT INTO stories (story) VALUES (?)";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    rc = sqlite3_bind_text(stmt, 1, story, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int getRandomStory(sqlite3 *db) {
    sqlite3_stmt *stmt;
    int rc;
    const char *sql = "SELECT story FROM stories ORDER BY RANDOM() LIMIT 1";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        printf("Random story: %s\n", sqlite3_column_text(stmt, 0));
    } else {
        printf("No stories in database.\n");
    }
    sqlite3_finalize(stmt);
    return 0;
}


int main() {
	// generate a truly more random seed value
	unsigned int seed = generate_random_seed();
	srand(seed);  
    // Create a new neural network
    int num_layers = 2;
    int layer_sizes[] = {2, 1};
    NeuralNetwork* network = create_neural_network(num_layers, layer_sizes);

    // Initialize the neural network weights and biases
    for (int i = 0; i < network->num_layers; i++) {
        for (int j = 0; j < network->layers[i].num_inputs; j++) {
            for (int k = 0; k < network->layers[i].num_outputs; k++) {
                network->layers[i].weights[j * network->layers[i].num_outputs + k] = (double) rand() / RAND_MAX;
            }
        }
        for (int j = 0; j < network->layers[i].num_outputs; j++) {
            network->layers[i].biases[j] = (double) rand() / RAND_MAX;
        }
    }

    // Create a SQLite database
    sqlite3* db;
    sqlite3_open("neural_network.db", &db);
    // tricky stuff here
    create_table(db);
    char* sql = "SELECT * FROM jokes";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        insert_jokes(db);
    }
    sqlite3_finalize(stmt);
    
    time_t start_time = time(NULL);
    time_t start_time_2 = time(NULL); 
    // Save the neural network to the database
    save_neural_network(network, db);

    // Load the neural network from the database
    load_neural_network(network, db);
    // start building number nodes    
    generateAndCombineRandomNumbers();
    // Test the neural network
    double inputs[] = {0, 0, 0, 1, 1, 0, 1, 1};
    double expected_outputs[] = {0, 1, 1, 0};
    test_neural_network(network, inputs, expected_outputs, 4);    
    printf("\033[2J\033[1;1H");
    // Respond to user questions
    while (1) {	
    char *errmsg = NULL; 
    int rc; 
    // Open database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
        // Create the tables
    const char *sql[] = {
        "CREATE TABLE IF NOT EXISTS answer (id INTEGER PRIMARY KEY);",
        "CREATE TABLE IF NOT EXISTS questions (id INTEGER PRIMARY KEY, response TEXT, answer_id INTEGER, FOREIGN KEY (answer_id) REFERENCES answer (id));",
        "CREATE TABLE IF NOT EXISTS question (id INTEGER PRIMARY KEY, questions_id INTEGER, FOREIGN KEY (questions_id) REFERENCES questions (id));"
};
    for (int i = 0; i < 3; i++) { 
		    sqlite3 *db;
        	int rc;	    
            char *errMsg = NULL;
            rc = sqlite3_open("neural_network.db", &db);
        rc = sqlite3_exec(db, sql[i], NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            //fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
        }
   
    }

    // Create table
    const char *sql2 = "CREATE TABLE IF NOT EXISTS stories(id INTEGER PRIMARY KEY, story TEXT)";
    rc = sqlite3_exec(db, sql2, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        return 1;
    }
        // Check if story exists
    if (checkStoryExists(db, story)) {
        // Define the array of sayings (This is our dirty trick) A gift to our user. 
    const char* sayings[] = {
        "OZ: Believe you can and you're halfway there.",
        "OZ: It does not matter how slowly you go as long as you do not stop.",
        "OZ: Success is not final, failure is not fatal: It is the courage to continue that counts.",
        "OZ: Don't watch the clock; do what it does. Keep going.",
        "OZ: You miss 100% of the shots you don't take.",
        "OZ: HI?",
        "OZ: PRO TIP: type 'tell me a story' maybe I will maybe not.",
        "OZ: PRO TIP: type 'exit' to exit and/or 'quit' to end the program and exiting the shell.",
        "OZ: Hello? ",
        ">>\n" 
    };
    int arrayLength = sizeof(sayings) / sizeof(sayings[0]); // Calculate the length of the array
    // Generate a random index into the array
    int randomIndex = rand() % arrayLength;
    // Print the random saying
    printf("%s\n", sayings[randomIndex]);      
    } else {
        // Insert story
        if (insertStory(db, story)) {
            printf("OZ: Failed to insert story into the database.\n");
        } else {
            printf("OZ: Story inserted into the database successfully.\n");
        }
    }
    char question[1024]; 
    printf(">> ");
    fgets(question, 256, stdin); 
	question[strcspn(question, "\n")] = 0; 	
	if (strcmp(question, "random number") == 0) {
	generateAndCombineRandomNumbers();
	displayRandomCombinedNumber(db); 
	}
	if (strcmp(question, "quit") == 0) { exit(1);  return 0; } 
	if (strcmp(question, "exit") == 0 || strcmp(question, "quit") == 0) { exit(1); return 0; }          
    if (strcmp(question, "tell a joke") == 0) {  tell_joke_2();  }
    if (strcmp(question, "joke?") == 0) {  tell_joke_2();  }
    if (strcmp(question, "joke please") == 0) {  tell_joke_2();  }
    if (strcmp(question, "Joke?") == 0) {  tell_joke_2();  }
       // Check if the command starts with "run "
    if (strncmp(question, "run ", 4) == 0) {
                // Execute the command after "run "
                int result = system(question + 4); // Skip "run "
                if (result == -1) {
                    perror("Error executing command");
                }
	}
        if (strcmp(question, "tell me a story") == 0) {
        getRandomStory(db);
    } else if (strcmp(question, "insert story") == 0) {
        char newStory[1024];
        printf("Enter a new story: ");
        scanf(" %[^\n]s", newStory);
        if (checkStoryExists(db, newStory)) {
            printf("Story already exists in the database.\n");
        } else {
            if (insertStory(db, newStory)) {
                printf("Failed to insert story into the database.\n");
            } else {
                printf("Story inserted into the database successfully.\n");
            }
        }
    } else {
      const char* sayings_2[] = {
        "OZ: Life is 10% what happens to you and 90% how you react to it.",
        "OZ: The biggest risk is not taking any risk. Don't you think?",
        "OZ: The future belongs to those who believe in the beauty of their dreams.",
        "OZ: The cake is a lie?",
        "OZ: PRO TIP: type 'random number' to display a number",
        "OZ: You don't have to be great to start, but you have to start to be great.",
        "OZ: PRO TIP: type 'tell me a story' maybe I will maybe not.",
        "OZ: PRO TIP: type 'exit' to exit and/or 'quit' to end the program and exiting the shell.",
        ">>\n" 
    };
    int arrayLength = sizeof(sayings_2) / sizeof(sayings_2[0]); // Calculate the length of the array
    // Generate a random index into the array
    int randomIndex = rand() % arrayLength;
    // Print the random saying
    printf("%s\n", sayings_2[randomIndex]); 
    }

		 // Check if 3 minutes have passed or 180 do triger event 
        time_t current_time = time(NULL);
        if (difftime(current_time, start_time) >= 180) {
            tell_joke(db);
            start_time = current_time;
        }
        // collect data, update netowrk & respond to user input
		collect_data_and_update_neural_network(network, db);
        respond_to_questions(network, db);
        // Respond by running zv program in our swarm.
        time_t diff_time = time(NULL);
        if (difftime(diff_time, start_time_2) >= 10) {
        system("zv");
        system("oz");
        start_time_2 = diff_time;
	    } else if (difftime(diff_time, start_time_2) >= 20) {
	    system("zx");
		system("oz");
		start_time_2 = diff_time;
		}
		
	    
    }

    // Close the database
    sqlite3_close(db);

    // Free the neural network memory
    for (int i = 0; i < network->num_layers; i++) {
        free(network->layers[i].weights);
        free(network->layers[i].biases);
    }
    free(network->layers);
    free(network);

    return 0;
}
