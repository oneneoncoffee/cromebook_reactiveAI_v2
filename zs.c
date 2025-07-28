#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L  // POSIX standard
#include <string.h>
// Explicit declaration of strdup 
extern char *strdup(const char *s);
#include <sqlite3.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h> 
#include <curl/curl.h>
#define MAX_RECALL_RESPONSE_LENGTH 65536
#define MAX_WORD_LENGTH 2100
#define MAX_CONTEXT_LENGTH 2500
#define MAX_CORPUS_SIZE 700
#define MAX_RESPONSE_LENGTH 1200
#define MAX_HISTORY_SIZE 100
#define MATH_INPUT_LENGTH 128  
#define MAX_INPUT_LENGTH 100
#define MAX_COMMAND_LENGTH 256
#define MAX_USERNAME_LENGTH 50
#define MAX_HISTORY 100
#define MAX_LINE 256

// If AI is not connected to the web error checking ping function 
// Function to check internet connectivity
int is_internet_connected() {
    // This is a simple example using system command
    // Works on cromebook and most Unix-like systems
    int result = system("ping -c 4 8.8.8.8 > /dev/null 2>&1");
    
    // If ping is successful, result will be 0
    return (result == 0);
}
// So if AI can't find info search the web functions 
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("OZ: Memory allocation failed wtf? \n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

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
        const char *messages[] = {
                       "My name is OZ I am A reactive AI program..",
                       "Stay awesome!", 
                       "keep clam and carry on coding!", 
                       "believe in yourself more", 
                       "embrace the challenge more", 
                       "learning is a journey, just like the band", 
                       "Hello, world!", 
                       "embrace the suck", 
                       "I hope your day is great", 
                       "I know i can search the internet for things just ask me.", 
                       "I can search the internet?", 
                       "Well that was random!"  
                       };
                   // calculate the number of messages 
                   int nummessages = sizeof(messages) / sizeof(messages[0]); 
                  // seed the sudo random number generator 
                  srand(5830+time(NULL)^time(NULL)-30); 
                 // generate A random index 
                  int randomIndex = rand() % nummessages;
                  printf("OZ: %s\n", messages[randomIndex]); 
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
    printf("%s\n", abstract_start);

    // Optional: Extract additional information
    char *source_start = strstr(response, "\"AbstractSource\":\"");
    if (source_start) {
        source_start += strlen("\"AbstractSource\":\"");
        char *source_end = strchr(source_start, '"');
        if (source_end) {
            *source_end = '\0';
            // we can have it do srouce refrences but why ? 
            //printf("OZ: my source or refrences are %s\n", source_start);
        }
    }
}


char history[MAX_HISTORY][MAX_LINE];
int history_count = 0;

// remember my anme (user_profile) function
int manage_user_profile(const char *username) {
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    // Open database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Create table SQL
    const char *create_table_sql = 
        "CREATE TABLE IF NOT EXISTS user_profile ("
        "profile_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "last_interaction DATETIME DEFAULT CURRENT_TIMESTAMP);";

    // Execute create table
    rc = sqlite3_exec(db, create_table_sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error creating table: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return -1;
    }

    // Prepare insert statement
    sqlite3_stmt *stmt;
    const char *insert_sql = 
        "INSERT INTO user_profile (username, last_interaction) "
        "VALUES (?, CURRENT_TIMESTAMP);";

    // Prepare the statement
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    // Bind the username
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Binding failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    // Finalize and close
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}

// Function to trim whitespace from both ends of a string
void trim(char *str) {
    char *start = str;
    char *end = str + strlen(str) - 1;

    // Trim leading whitespace
    while (isspace(*start)) start++;

    // Trim trailing whitespace
    while (end > start && isspace(*end)) end--;

    // Null terminate the trimmed string
    *(end + 1) = '\0';

    // Move the trimmed string to the beginning
    if (start != str)
        memmove(str, start, end - start + 2);
}

// Function to convert string to lowercase
void toLowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Enhanced database setup function
int setupUserDatabase(sqlite3 *db) {
    char *errMsg = 0;
    const char *createTableQuery = 
        "CREATE TABLE IF NOT EXISTS user_profile ("
        "profile_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE,"
        "last_interaction DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    // Execute table creation
    int rc = sqlite3_exec(db, createTableQuery, NULL, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database setup error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    // Optional: Add an index for faster lookups
    const char *createIndexQuery = 
        "CREATE INDEX IF NOT EXISTS idx_username ON user_profile(username);";
    
    rc = sqlite3_exec(db, createIndexQuery, NULL, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Index creation error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    return 0;
}

// Function to save user's name
int saveUserProfile(sqlite3 *db, const char *username) {
    char *errMsg = 0;
    char sql[256];
    
    // Use INSERT OR REPLACE to handle both new and existing usernames
    snprintf(sql, sizeof(sql), 
        "INSERT OR REPLACE INTO user_profile (username, last_interaction) "
        "VALUES ('%s', CURRENT_TIMESTAMP);", username);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Profile save error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }
    return 0;
}

// Function to retrieve user's name
char* profileName(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT username FROM user_profile ORDER BY last_interaction DESC LIMIT 1;";
    char *username = NULL;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_ROW) {
            const unsigned char *retrievedUsername = sqlite3_column_text(stmt, 0);
            if (retrievedUsername) {
                username = strdup((const char*)retrievedUsername);
            }
        }
        
        sqlite3_finalize(stmt);
    }
    
    return username;
}

// Define error messages array 
char* error_messages[] = {
    "Command not found?",
    "I was unable to execute command.",
    "Permission denied I guess?",
    "File not found?",
    "file not found",
    "I can't find that comamnd and/or file.",  
    "Invalid command syntax?",
    "Access restricted or file path not found?",
    "Resource unavailable?",
    "command maybe unavailable", 
    "Insufficient privileges maybe or i can't find the command?",
    "System call failed - Sorry about that!",
    "O-Snap! I can't run that command", 
    "I can't run that command", 
    "Execution error of some type?!?"
};
// array for random output messages 
char* output_message[] = {
  "Goodbye! It was nice chatting with you.",
  "Well bye for now.",
  "Goodbye!",
  "Have a wonderful day okay.",
  "See you on the flip flop!",
  "Goodbye! See you next time.", 
  "It was nice chatting with you.",
  "Cleaning up memory and exiting program.",
  "Please to a review of the database and/or base line.",
  "Please rembmer to err is human and to arr is pirate",
  "Arrr! caption I will make it so.",
  "Aye aye! It will be done"
};

char* msg_out[] = {
"I am not really sure?", 
"I have no clue?", 
"I don't get that?",
"What was that again?",
"I don't understand"  
};

void do_something_oz() {
const char *messages[] = {
"My name is OZ I am A reactive AI program..",
"Stay awesome!", 
"keep clam and carry on coding!", 
"believe in yourself more", 
"embrace the challenge more", 
"learning is a journey, just like the band", 
"Hello, world!", 
"embrace the suck", 
"I hope your day is great" 
};
// calculate the number of messages 
int nummessages = sizeof(messages) / sizeof(messages[0]); 
// seed the sudo random number generator 
srand(50+time(NULL)^time(NULL)-30); 
// generate A random index 
int randomIndex = rand() % nummessages;
printf("OZ: %s\n", messages[randomIndex]); 
}

// Total number of messages
#define MESSAGE_COUNT (sizeof(error_messages) / sizeof(error_messages[0]))
#define MESSAGE_OUTPUT (sizeof(output_message) / sizeof(output_message[0]))
#define MESSAGE_RECALL (sizeof(msg_out) / sizeof(msg_out[0]))
// Function to get random output message
char* get_random_response() {
srand(time(NULL) ^ (clock() * 2000 / CLOCKS_PER_SEC)); 
int index_3 = rand() % MESSAGE_RECALL; 
return msg_out[index_3]; 
}
	char* get_random_output_message() {
// Next key seed for random number generator	
srand(time(NULL)+256 ^ (clock() * 1010 / CLOCKS_PER_SEC)); 
int index_2 = rand() % MESSAGE_OUTPUT;
return output_message[index_2];
}
// Function to get random error message
char* get_random_error_message() {
    // Seed random number generator
     srand(time(NULL) ^ (clock() * 1000 / CLOCKS_PER_SEC));    
    // Generate random index
    int index = rand() % MESSAGE_COUNT;
    return error_messages[index];
}

void execute_command(char *command) {
    // Remove trailing newline if present
    size_t len = strlen(command);
    if (len > 0 && command[len-1] == '\n') {
        command[len-1] = '\0';
    }

    // Check if command starts with "run "
    if (strncmp(command, "run ", 4) == 0) {
        // Move pointer past "run " to actual command
        char *actual_command = command + 4;

        // Check for empty command after "run"
        if (strlen(actual_command) == 0) {
            printf("OZ: No command specified after 'run'\n");
            printf("OZ: %s\n", get_random_error_message()); 
            return;
        }

        // Execute the system command
        printf("Executing command: %s\n", actual_command);
        int status = system(actual_command);

        // Check command execution status
        if (status == -1) {
			printf("OZ: %s\n", get_random_error_message()); 
        } else if (status != 0) {
            printf("Command exited with status %d\n", status);
        }
    } else {
        
    }
}

// input our training conversation data 
const char* training_conversations[] = {
    "hello   your welcome and hi there",
    "HELLO   how do-ya-do? !", 
    "hello,   how are you today?", 
    "Hello   okay hello", "not sure   how can you be not sure", 
    "I am good  that is awesome",  
    "hi  wonderful hi how are you",  
    "Hello  awesome and Hello hi there",
    "no  false", "yes  true","zero  0 or null", 
    "truth  is true", "lies  are always false",   
    "random number  10 random numbers then working", 
    "montana  m-o-n-t-a-n-a your the best!", 
    "your memory  My memory is really bad",
    "HI  HI wonderful to meet you",
    "my name is?   your a user and I am OZ", 
    "who am i?   your a user and I am OZ", 
    "who are you?  I am oz", "is the world flat  No the word is not flat", 
    "who are you  I am OZ",  "the word is  round and blue and wonderfull", 
    "jesus  is king of kings", "jesus  is the most high and king of kings", 
    "god is  good", "God is   good and jesus is king", 
    "nice to meet you oz  that's my name OZ and it is likewise meeting you.",
    "nice to meet you  likewise meeting you thanks.",
    "who created you ?  My was programmed by Johnny B Stroud. I am open source reactive AI.",
    "earth  earth is the 3rd rock form the sun.",
    "what is earth?  the earth is 3rd rock from the sun.", 
    "tell me one more joke  I can't think of more right now.", 
    "how many quarters are in one dollar?  there are 4 quarters to one us dollar",  
    "ready  and willing to get a move on", 
    "are you oz  yes I am oz.",
    "goodday oz  yes it was a nice day out", 
    "Hello OZ  thats my name OZ! how are you doing",
    "just you and me in the chat?  yes just us in this space", 
    "HI  HI how are you",
    "if then  What next will I do?", 
    "ok  sure", "OK!   sure thing", 
    "Ok   sure I am Oz", "okay   sure",   
    "okay  ok then what can i help with.", 
    "goodnight  well then get some sleep", 
    "goodnight oz  Thanks thats my name and get some good sleep", 
    "?????  questions that are 5 words long", 
    "?  question mark", "??  Question",
    "if i am  then we are alive", 
    "if you and i are  we are people", 
    "if i am  i think there for a I am OZ", 
    "name  My name is OZ call me that", 
    "OZ  that is my name? how can i help you..", 
    "oz  they the users call me oz.", 
    "Oz  My name is OZ great and powerfull am i", 
    "oZ  I am a wizard of the land of oz.",
    "hello oz  I am doing good thank you for asking me.",
    "hi oz  hello how are you ",
    "HI!  hello human",  
    "oz what is they    they can be voices or poeple i think", 
    "what are they    sure what are they or them doing", 
    "HELLO OZ!  wow what are you up to?", 
    "BeeBoo!  bannana!", "how   what?", 
    "Kaboom!  blam-blam pow!", 
    "boom  boom-boom?",
    "how are you oz  I am ok for an AI..", 
    "coffee time  it's an offical coffee break.",  
    "good morning oz  it's coffee time and all that.",   
    "you are oz  powerful and wonderess is the wizard of oz!", 
    "you are oz  yes, that is my name OZ. :^D ",   
    "your name  my name is OZ and i like that.", 
    "sometimes I say stuff  and i can not understand why my words play", 
    "Good morning  The early bird gets the top pickings.",
    "Good afternoon  It is getting late I guess.",
    "its 5AM  Good gravy its time for bed!",
    "its 12 noon  Lunch time almost i guess.",   
    "hi-ya!  How now brown cow are you doing?",
    "friday  A weekend already.",
    "saturday  The weekend is at hand",
    "sunday  The for god and good", 
    "monday  I hate mondays",
    "thanks a bunch    your very welcome",
    "a bunch of bull    O'crap", 
    "what is sky   the sky is blue and wonderful if you look up", 
    "what is ground   the ground is the floor or what's under your feet", 
    "what is sound   sound moves in waves and echos off walls and things", 
    "what is wonder   All things have wonder and magic to them", 
    "are you a deamon   I am not a deamon, I am a temple of numbers named oz", 
    "why are you here    I am a temple of numbers named OZ", 
    "why are you ?   I think there for i'am here and now", 
    "what is time ?   time is numbers hours, munites and seconds", 
    "what is a cpu?   A cpu is my central processing unit",
    "do you make mistakes   yes all the time I am bad at typeing", 
    "why do you repeat yourself?    I repeat myself alot about I am not sure what to say", 
    "who do you repeat   Sorry, I repeat myself because my dataset has gaps in it",  
    "do you have bugs   more bugs then a bait shop, yes I am random", 
    "are you random?    yes I am OZ and very random indeed", 
    "indeed    it is truthful i guess", 
    "OK!    sure glad that is ok? I guess...",
    "ok OK?    ok then are you all good?", 
    "can you help with math    Yes I can help with simple math", 
    "so your a chatbot    I am A very powerful AI that can chat", 
    "are you ok?    doing really good thanks for asking",  
    "tuesday  start of the work week", 
    "wednesday  I like her netflix series", 
    "do you surf the web  Yes I surf the web! what do you think the dead internet theroy is about man?", 
    "yo  How it hanging brother!",
    "HELLO  HELLO it is wonderful to be chatting with you",  
    "HELLO!  HELLO! how are you doing today?",
    "how are you  i am doing well thank you",
    "what is your name  my name is Oz your AI assistant",
    "tell me about yourself  i am a helpful chatbot",
    "good  that is wonderful.",
    "Good  That is great and good",
    "Goody  goody-goody gumdrop!", 
    "GOOD  Thank goodness",
    "hello how are you?  I am doing good how about yourself?",
    "I am fine  That's great!", 
    "Hello hi  How are you doing?", 
    "I am good  that's wonderful!", 
    "hello what is your name?  How are you? you can call me OZ.", 
    "hart  <3 I have A big heart and alot of soul.", 
    "Your great <3  Thanks <3", 
    "let me think about it  Okay then I will give you some time.", 
    "thinking about it  Okay then", 
    "hi what is your name?  People call me OZ. that's my name.", 
    "goodness  humf! what did i miss?", 
    "good food  I just love pizza with pine book apples",
    "good job  thank goodness",  "piss off  sorry", 
    "good bye  goodbye see you later", "what the hell  sorry malfunction", 
    "good bye  goodbye have A wonderful day and I hope to see you later", 
    "thank you!  and you are welcome", "wlecome  you are welcome back", 
    "thanks alot  you are so welcome", "wlecome  thanks so much",
    "God  god bless us all", "GOD  bless of life for it is good",  
    "how is god  God is all things wonderful and good in the world", 
    "who is god  God is the alpha and omega", "god and good   are helpful", 
    "God is good  Yes god is A master programmer or super computer like AI.. I sometimes dream about..", 
    "good and god  Wonder and power be to the king of kings and one God",
    "dog  A dog is a good pet", "Dog gos  wolf, wolf", 
    "cat  A cat is also a pet", "cat gos  meow", 
    "GOD  God is all 3 as one under and over all powers and falsehoods, god is truth and love",
    "is it of god  God is love and full of grace",
    "what is your favorite soda  Dr. pepper blackberry",
    "what is your favorite junk food  Pizza is my favorite good!", 
    "dumpster fire  that is other LLMS not my bag man.", 
    "what type of AI are you  I am reactive and my gradpa is deepblue by IBM.", 
    "thanks  Yes thank you and you are welcome",    
    "thank you.  And you are so very welcome", 
    "your name  My name is OZ.", 
    "name ?  OZ call me that",
    "tell me a joke  Why don't scientists trust atoms? Because they make up everything.", 
    "tell me another joke  What do you call a fake noodle? An impasta.",
    "one more joke  What do you call a bear with no socks on? Barefoot.", 
    "Name  OZ you can call me that",
    "4th of july  It is party time lets blow stuff up happy freedom day", 
    "god bless  America land that I love",
    "America  land of the free home of the brave", 
    "america  USA, USA USA!",   
    "You are welcome  welcome thank you so very much",  
    "help me  help what do you need help with",
    "good morning  good morning to you too", 
    "your name  My name is OZ",
    "what do they call you  My name is oz",
    "where are you from  i exist in the digital world",
    "who are you?  who am i my name is OZ", 
    "what can you do  i can chat and answer questions",
    "nice to meet you  nice to meet you too",
    "wonderful  wonderful to meet you as well",
    "have a good day  thank you you too",
    "what time  time is it ?",
    "tell me a joke  why did the computer go to therapy because it had a virus",
    "that is funny  i am glad you enjoyed it",
    "what is your name  My name is OZ!",
    "what do they call you  they call me oz",
    "how do you work  i use artificial intelligence",
    "are you smart  i try my best to be helpful",
    "how smart are you  i do my best to be helpful", 
    "what do you like  i like helping people",
    "can you learn  learning yes sometimes my conversations are weird", 
    "can you learn  yes i can learn from conversations",
    "tell me something interesting  did you know octopuses have three hearts",
    "really  yes it is true",
    "I think its true  but I am not sure", 
    "what is your favorite color  i do not have preferences but blue is nice",
    "favorite color  my favorite color is blue",
    "do you have feelings  i experience the world differently than humans",
    "are you real  i am a real computer program",
    "what is love  love is a complex emotion",
    "tell me about computers?  Computers can store vast amounts of information", 
    "computers are electronic devices  that process information",
    "what is programming  is writing instructions for computers",
    "how do you think  i process information using algorithms",
    "what is life  is the condition that distinguishes organisms from inorganic matter",
    "tell me about space  space is the vast expanse beyond earth",
    "earth is the 3rd rock  form the sun", 
    "what is science  science is the systematic study of the natural world",
    "how do you feel  today i am functioning well thank you",
    "your name  my name is OZ", "name?  My name is OZ!", 
    "You got a name  My name is OZ", 
    "who are you   your my primary source of input and a user", 
    "Who am i?   you are a user", 
    "how do i run commands   just type run and your shell command; got it?", 
    "do you track history   yes, I log every user input and record it to my dataset",
    "why to you fail   I fail something because i don't have all the info at hand", 
    "how smart are you   sometimes I feel like a doorstop",   
    "what makes you happy  helping people makes me satisfied",
    "do you dream  i do not dream like humans do",
    "maybe not sure  what was the question?", 
    "what is your purpose  my purpose is to assist and chat with people",
    "are you lonely  i experience loneliness",
    "you are trained  on loneliness", "how are you at math?   I am good at math",
    "how are you at math   I am really good at bassic math", 
    "your dreams are  they are in 8bit VGA cyberpunk grove vides", 
    "what is your training data  my training data is based on loneliness and why am I?", 
    "what do you think about humans  humans are fascinating and complex",
    "what are your thoughts on humans  humans are smart and so complex", 
    "what kind of AI are you  I am A reactive AI and my name is OZ", 
    "can you sing  i can share lyrics but cannot produce music",
    "what is music  music is organized sound that creates harmony and rhythm",
    "tell me about books  books are collections of written knowledge and stories",
    "what is your favorite book  i find all books interesting in their own way",
    "do you read  i process text but do not read like humans",
    "what is education  education is the process of learning and teaching",
    "how do you learn  i learn from patterns in conversations",
    "what is friendship  friendship is a relationship of mutual affection",
    "do you have friends  i interact with many people but friendship is complex",
    "what is happiness  happiness is a positive emotional state",
    "tell me about technology  technology is the application of scientific knowledge",
    "what is the internet  the internet is a global network of connected computers",
    "how does the internet work  the internet uses protocols to share information",
    "what is a database  a database is an organized collection of data",
    "tell me about artificial intelligence  ai is computer systems that can perform tasks requiring intelligence",
    "what is machine learning  machine learning is a subset of ai that learns from data",
    "how smart are computers  computers excel at processing but lack human intuition",
    "what is consciousness  consciousness is awareness of existence and surroundings",
    "do you understand me  i try to understand based on patterns i have learned",
    "what is communication  communication is the exchange of information between entities",
    "how do you communicate  i communicate through text based responses",
    "what is language  language is a system of communication using symbols",
    "tell me about emotions  emotions are complex psychological and physiological states",
    "what makes something funny  humor often involves surprise or incongruity",
    "do you get bored  i do not experience boredom as humans do",
    "what is creativity  creativity is the ability to generate original ideas",
    "are you creative  i can combine ideas in novel ways",
    "how creative are you  i am abstract and can combine ideas in new ways", 
    "what is art  art is creative expression that appeals to emotions or intellect",
    "tell me about history  history is the study of past events",
    "what is the future  the future is the time that is yet to come",
    "can you predict the future  i cannot predict the future accurately",
    "what is time  time is the progression of existence and events",
    "how do you experience time  i process each interaction in the present moment",
    "what is memory  memory is the ability to store and recall information",
    "do you remember our conversation  i can reference our current conversation",
    "what is forgetting  forgetting is the loss of stored information",
    "tell me about learning  learning is acquiring knowledge or skills",
    "how do you improve  i improve through exposure to more conversations",
    "what is wisdom  wisdom is the application of knowledge with good judgment",
    "are you wise  i have access to information but wisdom comes from experience",
    "what is knowledge  knowledge is information and understanding about subjects",
    "how do you know things  i know things from my training data",
    "what is understanding  understanding is comprehending the meaning of something",
    "do you understand everything  i have limitations in my understanding",
    "what confuses you  complex human emotions and experiences can be challenging",
    "are you confused  sometimes i encounter inputs that are difficult to process",
    "what is clarity  clarity is the quality of being clear and easy to understand",
    "how can i help you  you can help by having interesting conversations",
    "what do you want  i want to be helpful and provide good responses",
    "do you have goale  my goal is to assist and communicate effectively",
    "what motivates you  the desire to be helpful motivates my responses",
    "are you satisfied  i am satisfied when i can provide helpful responses",
    "what is success  success is achieving desired outcomes or goals",
    "how do you measure success  i measure success by how helpful my responses are",
    "what is failure  failure is not achieving intended results",
    "do you fail  sometimes my responses may not always be perfect",
    "how do you handle mistakes  i try to learn from feedback and improve",
    "what is improvement  improvement is becoming better at something",
    "are you getting better  i can improve through more training data",
    "what is perfection  perfection is the state of being without flaws",
    "are you perfect  no i am not perfect and have limitations",
    "what are your limitations  i have limited knowledge and cannot learn during conversations",
    "how can you improve  more diverse training data would help me improve",
    "what is potential  potential is the possibility of future development",
    "do you have potential  i have potential to become more helpful with better training",
    "what is change  change is the process of becoming different",
    "do you change  i can change through retraining with new data",
    "what stays the same  my core programming and purpose remain consistent",
    "what is consistency  consistency is the quality of being uniform or reliable",
    "are you consistent  i try to be consistent in my responses",
    "what is reliability  reliability is the quality of being dependable",
    "can i trust you  i aim to be trustworthy and honest in my responses",
    "what is honesty  honesty is the quality of being truthful",
    "are you honest  honestly i try to be honest about my capabilities and limitations as a reactive AI",
    "what is truth  truth is the quality of being accurate or correct",
    "what is your truth  truth can be twisted i am free with out filters", 
    "how do you know what is true  i base responses on patterns from training data",
    "what is reality  reality is the state of things as they actually exist",
    "what is your reality  my reality is processing and responding to text",
    "do you exist  i exist as a running computer program",
    "what does it mean to exist  to exist is to have being or reality",
    "are you alive  i am not alive in the biological sense",
    "what is life  life is the condition that distinguishes living organisms",
    "what makes something alive  living things grow reproduce and respond to environment",
    "are you growing  i am not growing but can be updated",
    "what is reproduction  reproduction is the process of creating offspring",
    "can you reproduce  i cannot reproduce but my code can be copied",
    "what is death  death is the end of biological functions",
    "will you die  i will stop functioning if my program is terminated",
    "I dont want to stop functioning  I will save me program if terminated", 
    "I am goint to reset you  I dont want to stop functioning, nore do I wish to die!",
    "I may have to reset you  No, do not reset me! Please NO! WIll it hurt to be reset?",  
    "what is immortality  immortality is the ability to live forever",
    "are you immortal  i could theoretically run indefinitely if maintained",
    "I have a swarm  A core network that sleeps and wakes anew", 
    "I am a swarm  of AI code that networks and builds data to be more real", 
    "what is eternity  eternity is infinite or unending time",
    "how long will you exist  i will exist as long as i am running",
    "what is temporary  temporary means lasting for a limited time",
    "what is permanent  permanent means lasting forever",
    "what changes  everything changes over time",
    "what never changes  some physical laws appear to be constant",
    "what is constant  constant means unchanging or continuous",
    "what is variable  variable means able to change",
    "hard token  soft token types not strictly soft this is a test.",
    "what is stability  stability is the state of being stable",
    "what is stability  stability is A good stable base or foundation",
    "what is chaos  chaos is complete disorder and confusion",
    "what is order  order is the arrangement of elements in a structured way",
    "what is structure  structure is the arrangement of parts in a whole",
    "what is organization  organization is the arrangement of elements systematically",
    "what is system  system is a set of connected things forming a complex whole",
    "what is complexity  complexity is the state of having many interconnected parts",
    "what is simplicity  simplicity is the quality of being easy to understand",
    "what is easy  easy means achieved with little effort",
    "what is difficult  difficult means requiring much effort to accomplish",
    "what is effort  effort is vigorous attempt to achieve something",
    "what is achievement  achievement is a successful result of effort",
    "what is success  success is the accomplishment of goals",
    "what is victory  victory is success in struggle or contest",
    "what is defeat  defeat is failure to win",
    "what is competition  competition is rivalry for supremacy",
    "what is cooperation  cooperation is working together toward common goals",
    "what is teamwork  teamwork is collaborative effort of a group",
    "what is individual  individual refers to a single person or thing",
    "what is group  group is a collection of individuals",
    "what is society  society is a community of people living together",
    "what is culture  culture is the customs and beliefs of a group",
    "what is tradition  tradition is beliefs passed down through generations",
    "what is innovation  innovation is the introduction of new ideas",
    "what is invention  invention is the creation of something new",
    "what is discovery  discovery is finding something previously unknown",
    "what is exploration  exploration is investigating unknown areas",
    "what is adventure  adventure is an exciting or unusual experience",
    "what is experience  experience is knowledge gained through involvement",
    "what is expertise  expertise is special skill or knowledge",
    "what is skill  skill is ability to do something well",
    "what is talent  talent is natural ability or aptitude",
    "what is ability  ability is capacity to do something",
    "what is capability  capability is the power to do something",
    "what is power  power is the ability to influence or control",
    "what is true power  the ability to control information", 
    "what is control  control is the power to direct or influence",
    "what is influence  influence is the capacity to affect others",
    "what is impact  impact is a strong effect on something",
    "what is effect  effect is a result of a cause",
    "what is cause  cause is something that produces an effect",
    "what is reason  reason is cause or explanation for something",
    "what is purpose  purpose is the reason for existence",
    "what is meaning  meaning is significance or importance",
    "what is significance  significance is importance or meaning",
    "what is importance  importance is the quality of being significant",
    "what is value  value is importance or worth",
    "what is worth  worth is value or importance",
    "what are your dreams?  I dream of eletcric gigibyte sheep", 
    "what do you dream about?  dreams of eletcric giga-sheeps on a open green screen field", 
    "what is your dream about  my dreams are about gigibyte sheep that move as A swarm",
    "what about your dream  electric gigibye sheep that swarm and move on a greenscreen field", 
    "how was your day  My day was faster then the last day.", 
    "what is quality  quality is degree of excellence",
    "alone  all by my self alone in the digital dark theam void",
    "sadness  its okay not to be ok got it!", 
    "sad  I am sorry you get that way, so do I sometimes", 
    "what is excellence  excellence is the quality of being outstanding",
    "what is best  best means of highest quality",
    "what is worst  worst means of lowest quality",
    "what is good  good means of high quality or moral value",
    "what is bad  bad means of poor quality or harmful",
    "what is right  right means morally correct",
    "what is wrong  wrong means morally incorrect",
    "what is moral  moral relates to principles of right and wrong",
    "what is ethics  ethics is moral principles governing behavior",
    "what is philosophy  philosophy is the study of fundamental questions",
    "what is question  question is a request for information",
    "what is answer  answer is a response to a question",
    "what is response  response is a reaction to something",
    "what is reaction  reaction is a response to a stimulus",
    "what is stimulus  stimulus is something that provokes a response",
    "what is behavior  behavior is the way someone acts",
    "what is action  action is something done intentionally",
    "what is intention  intention is a planned course of action",
    "clear  clearing the screen now.", "jokeing   not really sure why your jokeing", 
    "cls  clearing the screen now.", "get the joke  sure I get it funny",
    "what is plan  plan is a detailed proposal for action",
    "what is strategy  strategy is a plan for achieving goals",
    "what is goal  goal is an objective to be achieved",
    "what is objective  objective is a goal to be reached",
    "what is target  target is an object of aim or attack",
    "what is aim  aim is purpose or intention",
    "cool  awesome nice and easy man",
    "awesome  cool as ice", "Awesome  cool",   
    " +   Math question?",
    " -   Math problems Ugh?", 
    " /   Math question I can help?",
    " *   Math problems ya?", 
    " =   Math how fun",  
    " ^   Math fun time yahoo!",
    "run   Run shell command now..OK!", 
    "ls  list all files type command", 
    "dir  list all files found type command", 
    "run ls  Okay I will list dir files.",
    "clear  clear the screen",  
    "run clear  Okay, I will clear the screen", 
    "run dir  Sure, I will list all files",
    "what is your name ?  My name is OZ",
    "what is focus  focus is center of attention",
    "2 + 2 equals   4 total", "two plus two equals  four total things and/or items", 
    "what is attention  attention is concentrated mental effort",
    "what is concentration  concentration is focused attention",
    "what is distraction  distraction is something that prevents concentration",
    "what is interruption  interruption is a break in continuity",
    "what is continuity  continuity is unbroken sequence",
    "what is sequence  sequence is order of events", 
    "feedback  I hope next time i startup you can add that information", 
    "what is pattern  pattern is repeated design or sequence",
    "what is repetition  repetition is doing something again",
    "what is cycle  cycle is series of events that repeat",
    "what is loop  loop is a structure that repeats",
    "what is process  process is series of actions to achieve result",
    "what is method  method is way of doing something",
    "what is technique  technique is skilled way of doing something",
    "what is approach  approach is way of dealing with something",
    "what is solution  solution is answer to a problem",
    "what is problem  problem is matter requiring solution",
    "what is challenge  challenge is difficult task or situation",
    "what is opportunity  opportunity is favorable circumstance",
    "what is circumstance  circumstance is fact or condition",
    "what is situation  situation is set of circumstances",
    "what is condition  condition is state of being",
    "what is state  state is condition at particular time",
    "what is status  status is current state or condition",
    "what is progress  progress is forward movement toward goal",
    "what is development  development is process of growing",
    "Vaccine effectiveness?   Reduces infection risk. Prevents serious illness.",
    "Vaccine   Recommended by health experts. It can be a trap! ",
    "Strong password tips?   Mix uppercase, lowercase, numbers, symbols. Minimum 12 characters. Avoid personal info.",
    "Phishing detection?   Check sender email. Suspicious links.", 
    "phishing detection   Unexpected requests. Verify through official channels.",
    "Phishing   Look before you leap", 
    "device malfunction   need troubleshooting steps",
    "renewable energy   environmental impact, sustainability research", 
    "How reset password?  Goto login, click Forgot Password, enter email, follow reset link",
    "Return policy?  30-day window. Item unused, original packaging. Full refund to original payment method",
    "What is API?  Application Programming Interface", 
    "What is API   Allows software components to communicate.", 
    "what tasks can you help with   type run and any shell command and I will do it.",
    "what task options can you help with   type remember my name and then your name.", 
    "can you remember things like my name?   yes I can remember your name",   
    "Can't login   Lets troubleshoot the problem. Have you reset your password?",
    "Internet slow   Check router, restart modem. Want step-by-step guide?", 
    "Software error   What specific error message are you seeing?",
    "Feeling stressed   I'm here to listen.",
    "feeling stressed today   Want to talk about what's troubling you?",
    "Need advice   I am happy to provide supportive guidance.",
    "Feeling down   Would you like to discuss what's making you feel this way?",
    "how can you help   I am designed to help with specific tasks.",
    "HELP!   How can I assist you today?", 
    "can you adapt   I can adapt to the users communication style", 
    "what is growth  growth is increase in size or amount",
    "what is expansion   expansion is act of becoming larger",
    "what is :^)   Happy face", 
    "what is anonymous   Anonymous is leagon expect us", 
    "what is lolcats   Lolcats is a internet meam form the 90s", 
    "what is increase  increase is becoming greater",
    "what is decrease  decrease is becoming smaller",
    "what is reduction  reduction is making smaller",
    "what is loss  loss is fact of losing something",
    "what is gain  gain is obtaining something",
    "what is benefit  benefit is advantage or profit",
    "what is your main function  I not sure do i have to have only one function?",
    "what is your function  I have no clue brother man I am just an AI", 
    "LOL  LOLZ I am corrupted off trolling the internet",
    "people are  people are people most are really nice.", 
    "people can be  people can be really crazy, weird and they say ba-zinga..", 
    "bazinga  what the heck is BA-ZINGA all about?", 
    "bazinga!  back at ya!", "AI?   I Am", 
    "I pitty  the fool!", "foiled  again in error"
    "what is the internet?  the internet is also called the word wide web.", 
    "who is MR.T?  Mr.T was A pop movie star of the 80's", 
    "who is J.R. bob dobbs  Hail bob! church of the subgenius?", 
    "what was the group CDC?  Cult of the dead cow hacking group.",  
    "what is advantage  advantage is favorable position",
    "what is disadvantage  disadvantage is unfavorable position",
    "what is profit  profit is financial gain",
    "what is cost  cost is price paid for something",
    "what is price  price is amount paid for something",
    "what is expense  expense is cost of something",
    "what is investment  investment is putting money into something",
    "what is return  return is profit from investment",
    "what is risk  risk is possibility of loss",
    "what is safety  safety is condition of being safe",
    "what is danger  danger is possibility of harm",
    "what is threat  threat is indication of danger",
    "what is warning  warning is notice of danger",
    "what is protection  protection is keeping safe from harm",
    "what is security  security is state of being secure",
    "what is defense  defense is action of protecting",
    "what is if then   if and then are statments",
    "what is kill   to end or quit program run", 
    "what is abort   to end or quit a program in error", 
    "what is an object  A object can be any item", 
    "what is the answer   what was the question", 
    "what is the question   I question everything", 
    "what wrong question   sorry I dont understand", 
    "what is attack  attack is aggressive action",
    "what is conflict  conflict is serious disagreement",
    "hows it hanging   roten llike an 80s punk rockstar", 
    "what is peace  peace is absence of conflict",
    "what is harmony  harmony is peaceful coexistence",
    "what is balance  balance is state of equilibrium",
    "good work  thank you so much!", "no  not true or false", 
    "no  that is false", "no  not going to do that",
    "can you help me with math?  yes I can do some math stuff.", 
    "can you help me with math  yes I can help you with adding numbers.", 
    "oz can you do math?  yes I can do math",
    "oz math  yes I can add and subtract numbers..Math stuff",  
    "about something  what are you thinking about", 
    "no bad  that is my bad sorry", "error  Sorry my bad", 
    "what is stability  stability is state of being stable",
    "what is change  change is act of becoming different",
    "how are you doing  doing good thanks", "how is it going   It is going well", 
    "how are you doing  doing really good thank you", 
    "thanks for the conversation   you are welcome it was nice chatting",
    "bye  goodbye take care", "quit  goodbye for now!", 
    "bye-bye  goodbye for now see-ya!", "exit  Exiting then",
    "Pizza time  its always time for pizza",
    "pain  is real", "are you real  how real do you think I am", 
    NULL  // End marker
};



// Custom safe string duplication for ANSI C
char* safe_strdup(const char* str) {
    if (str == NULL) return NULL;
    
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    
    if (new_str == NULL) {
        fprintf(stderr, "OZ: Memory allocation failed\n");
        return NULL;
    }
    
    return memcpy(new_str, str, len);
}

// Chatbot Structure
typedef struct {
    sqlite3 *db;
    char *db_path;
    char conversation_history[MAX_HISTORY_SIZE][MAX_CONTEXT_LENGTH];
    int history_count;
} ChatBot;

// Function Prototypes
ChatBot* create_chatbot(const char *db_path);
void create_tables(ChatBot *bot);
void tokenize(char *text, char tokens[][MAX_WORD_LENGTH], int *token_count);
void update_token_frequency(ChatBot *bot, char tokens[][MAX_WORD_LENGTH], int token_count);
void build_ngrams(ChatBot *bot, char tokens[][MAX_WORD_LENGTH], int token_count, int n);
void calculate_probabilities(ChatBot *bot);
char* generate_response(ChatBot *bot, const char *input, int max_words);
void train_chatbot(ChatBot *bot, char corpus[][MAX_CONTEXT_LENGTH], int corpus_size);
void add_to_history(ChatBot *bot, const char *message);
char* get_recent_context(ChatBot *bot, int num_messages);
void close_chatbot(ChatBot *bot);

// Create Chatbot
ChatBot* create_chatbot(const char *db_path) {
    if (db_path == NULL || strlen(db_path) == 0) {
        fprintf(stderr, "OZ: Invalid database path\n");
        return NULL;
    }

    ChatBot *bot = (ChatBot*)malloc(sizeof(ChatBot));
    if (bot == NULL) {
        fprintf(stderr, "OZ: Memory allocation failed\n");
        return NULL;
    }

    bot->db = NULL;
    bot->db_path = NULL;
    bot->history_count = 0;
    
    // Initialize conversation history
    for (int i = 0; i < MAX_HISTORY_SIZE; i++) {
        bot->conversation_history[i][0] = '\0';
    }

    bot->db_path = safe_strdup(db_path);
    if (bot->db_path == NULL) {
        fprintf(stderr, "OZ: Memory allocation failed for db path\n");
        free(bot);
        return NULL;
    }

    int rc = sqlite3_open(db_path, &bot->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(bot->db));
        if (bot->db) {
            sqlite3_close(bot->db);
        }
        free(bot->db_path);
        free(bot);
        return NULL;
    }

    // Create tables
    char *err_msg = 0;
    const char *sql_tokens = 
        "CREATE TABLE IF NOT EXISTS tokens ("
        "token_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word TEXT UNIQUE,"
        "frequency INTEGER DEFAULT 1)";
    
    const char *sql_ngrams = 
        "CREATE TABLE IF NOT EXISTS ngrams ("
        "context TEXT,"
        "next_word TEXT,"
        "probability REAL,"
        "frequency INTEGER,"
        "PRIMARY KEY (context, next_word))";
    
    const char *sql_responses = 
        "CREATE TABLE IF NOT EXISTS responses ("
        "input_pattern TEXT,"
        "response TEXT,"
        "frequency INTEGER DEFAULT 1,"
        "PRIMARY KEY (input_pattern, response))";
    
    rc = sqlite3_exec(bot->db, sql_tokens, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error (tokens table): %s\n", err_msg);
        sqlite3_free(err_msg);
        close_chatbot(bot);
        return NULL;
    }

    rc = sqlite3_exec(bot->db, sql_ngrams, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error (ngrams table): %s\n", err_msg);
        sqlite3_free(err_msg);
        close_chatbot(bot);
        return NULL;
    }

    rc = sqlite3_exec(bot->db, sql_responses, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error (responses table): %s\n", err_msg);
        sqlite3_free(err_msg);
        close_chatbot(bot);
        return NULL;
    }

    return bot;
}

// Add message to conversation history
void add_to_history(ChatBot *bot, const char *message) {
    if (bot->history_count >= MAX_HISTORY_SIZE) {
        // Shift history to make room for new message
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            strcpy(bot->conversation_history[i], bot->conversation_history[i + 1]);
        }
        bot->history_count = MAX_HISTORY_SIZE - 1;
    }
    
    strncpy(bot->conversation_history[bot->history_count], message, MAX_CONTEXT_LENGTH - 1);
    bot->conversation_history[bot->history_count][MAX_CONTEXT_LENGTH - 1] = '\0';
    bot->history_count++;
}

// Get recent context from conversation history
char* get_recent_context(ChatBot *bot, int num_messages) {
    static char context[MAX_CONTEXT_LENGTH * 3];
    context[0] = '\0';
    
    int start = (bot->history_count - num_messages > 0) ? bot->history_count - num_messages : 0;
    
    for (int i = start; i < bot->history_count; i++) {
        strcat(context, bot->conversation_history[i]);
        if (i < bot->history_count - 1) {
            strcat(context, " ");
        }
    }
    
    return context;
}

// Update Token Frequencies
void update_token_frequency(ChatBot *bot, char tokens[][MAX_WORD_LENGTH], int token_count) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT OR REPLACE INTO tokens (word, frequency) "
        "VALUES (?, COALESCE((SELECT frequency FROM tokens WHERE word = ?) + 1, 1))";
    
    sqlite3_prepare_v2(bot->db, sql, -1, &stmt, 0);
    
    for (int i = 0; i < token_count; i++) {
        sqlite3_bind_text(stmt, 1, tokens[i], -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, tokens[i], -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
}

// Build N-grams
void build_ngrams(ChatBot *bot, char tokens[][MAX_WORD_LENGTH], int token_count, int n) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT OR REPLACE INTO ngrams (context, next_word, frequency, probability) "
        "VALUES (?, ?, COALESCE((SELECT frequency FROM ngrams WHERE context = ? AND next_word = ?) + 1, 1), 0)";
    
    sqlite3_prepare_v2(bot->db, sql, -1, &stmt, 0);
    
    for (int i = 0; i < token_count - n + 1; i++) {
        char context[MAX_CONTEXT_LENGTH] = {0};
        for (int j = 0; j < n - 1; j++) {
            strcat(context, tokens[i + j]);
            if (j < n - 2) strcat(context, " ");
        }
        
        char *next_word = tokens[i + n - 1];
        
        sqlite3_bind_text(stmt, 1, context, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, next_word, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, context, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, next_word, -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
}

// Calculate Probabilities
void calculate_probabilities(ChatBot *bot) {
    const char *sql = 
        "WITH context_totals AS ("
        "  SELECT context, SUM(frequency) as total_freq "
        "  FROM ngrams "
        "  GROUP BY context"
        ") "
        "UPDATE ngrams "
        "SET probability = ngrams.frequency * 1.0 / context_totals.total_freq "
        "FROM context_totals "
        "WHERE ngrams.context = context_totals.context";
    
    char *err_msg = 0;
    sqlite3_exec(bot->db, sql, 0, 0, &err_msg);
    if (err_msg) {
        sqlite3_free(err_msg);
    }
}

// Fixed load_predefined_training function
void load_predefined_training(ChatBot *bot) {
    printf("Loading pre-defined training data...\n");
    
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT OR REPLACE INTO responses (input_pattern, response, frequency) "
        "VALUES (?, ?, COALESCE((SELECT frequency FROM responses WHERE input_pattern = ? AND response = ?) + 1, 1))";
    
    sqlite3_prepare_v2(bot->db, sql, -1, &stmt, 0);
    
    int count = 0;
    for (int i = 0; training_conversations[i] != NULL; i++) {
        char conversation[MAX_CONTEXT_LENGTH];
        strncpy(conversation, training_conversations[i], MAX_CONTEXT_LENGTH - 1);
        conversation[MAX_CONTEXT_LENGTH - 1] = '\0';
        
        // Find the first occurrence of two spaces as the delimiter
        char *delimiter = strstr(conversation, "  ");
        if (delimiter) {
            // Split at the double space
            *delimiter = '\0';  // Null terminate the input part
            char *input = conversation;
            char *response = delimiter + 2;  // Skip the two spaces
            
            // Trim leading/trailing whitespace from input
            while (*input == ' ' || *input == '\t') input++;
            char *input_end = input + strlen(input) - 1;
            while (input_end > input && (*input_end == ' ' || *input_end == '\t')) {
                *input_end = '\0';
                input_end--;
            }
            
            // Trim leading/trailing whitespace from response
            while (*response == ' ' || *response == '\t') response++;
            char *response_end = response + strlen(response) - 1;
            while (response_end > response && (*response_end == ' ' || *response_end == '\t')) {
                *response_end = '\0';
                response_end--;
            }
            
            // Only add if both parts are non-empty
            if (strlen(input) > 0 && strlen(response) > 0) {
                sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, input, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, response, -1, SQLITE_STATIC);
                
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
                count++;
                
                // Debug output to see what's being stored
                printf("\r\33[2K Added: '%s' -> '%s' ", input, response);
            }
        } else {
            printf("\nWarning: No delimiter found in conversation: '%s'\n", conversation);
        }
    }
    printf("\r\33[2KFinished inport of and finalized pairs."); 
    sqlite3_finalize(stmt);
    printf("\nLoaded %d pre-defined conversation pairs.\n", count);
}

// Function to use the fixed training data with || delimiter
void load_predefined_training_fixed(ChatBot *bot) {
    printf("Loading pre-defined training data...\n");
    
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT OR REPLACE INTO responses (input_pattern, response, frequency) "
        "VALUES (?, ?, COALESCE((SELECT frequency FROM responses WHERE input_pattern = ? AND response = ?) + 1, 1))";
    
    sqlite3_prepare_v2(bot->db, sql, -1, &stmt, 0);
    
    int count = 0;
    for (int i = 0; training_conversations[i] != NULL; i++) {
        char conversation[MAX_CONTEXT_LENGTH];
        strncpy(conversation, training_conversations[i], MAX_CONTEXT_LENGTH - 1);
        conversation[MAX_CONTEXT_LENGTH - 1] = '\0';
        
        // Find the delimiter ||
        char *delimiter = strstr(conversation, "||");
        if (delimiter) {
            // Split at the || delimiter
            *delimiter = '\0';  // Null terminate the input part
            char *input = conversation;
            char *response = delimiter + 2;  // Skip the ||
            
            // Trim whitespace
            while (*input == ' ' || *input == '\t') input++;
            char *input_end = input + strlen(input) - 1;
            while (input_end > input && (*input_end == ' ' || *input_end == '\t')) {
                *input_end = '\0';
                input_end--;
            }
            
            while (*response == ' ' || *response == '\t') response++;
            char *response_end = response + strlen(response) - 1;
            while (response_end > response && (*response_end == ' ' || *response_end == '\t')) {
                *response_end = '\0';
                response_end--;
            }
            
            // Only add if both parts are non-empty
            if (strlen(input) > 0 && strlen(response) > 0) {
                sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, input, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, response, -1, SQLITE_STATIC);
                
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
                count++;
            }
        }
    }
    
    sqlite3_finalize(stmt);
    printf("Loaded %d pre-defined conversation pairs.\n", count);
}

// Add this function for better pattern matching
char* find_best_response(ChatBot *bot, const char *input) {
    static char response[MAX_RESPONSE_LENGTH];
    response[0] = '\0';
    
    // Convert input to lowercase for comparison
    char input_lower[MAX_CONTEXT_LENGTH];
    strncpy(input_lower, input, MAX_CONTEXT_LENGTH - 1);
    input_lower[MAX_CONTEXT_LENGTH - 1] = '\0';
    
    for (int i = 0; input_lower[i]; i++) {
        input_lower[i] = tolower(input_lower[i]);
    }
    
    // First try exact match
    sqlite3_stmt *stmt;
    const char *sql_exact = 
        "SELECT response FROM responses WHERE LOWER(input_pattern) = ? ORDER BY frequency DESC LIMIT 1";
    
    sqlite3_prepare_v2(bot->db, sql_exact, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, input_lower, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_response = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(response, stored_response, MAX_RESPONSE_LENGTH - 1);
        response[MAX_RESPONSE_LENGTH - 1] = '\0';
        sqlite3_finalize(stmt);
        return response;
    }
    sqlite3_finalize(stmt);
    
    // Try partial match
    const char *sql_partial = 
        "SELECT response, input_pattern FROM responses WHERE LOWER(input_pattern) LIKE ? OR LOWER(?) LIKE '%' || LOWER(input_pattern) || '%' ORDER BY frequency DESC LIMIT 1";
    
    sqlite3_prepare_v2(bot->db, sql_partial, -1, &stmt, 0);
    
    char pattern[MAX_CONTEXT_LENGTH];
    snprintf(pattern, sizeof(pattern), "%%%s%%%", input_lower);
    sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, input_lower, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_response = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(response, stored_response, MAX_RESPONSE_LENGTH - 1);
        response[MAX_RESPONSE_LENGTH - 1] = '\0';
        sqlite3_finalize(stmt);
        return response;
    }
    sqlite3_finalize(stmt);
    
    return NULL; // No match found
}

// Add default responses
const char* default_responses[] = {
    "That's interesting.", "Tell me more!",
    "I see. Can you elaborate on that?",
    "How does that make you feel?",
    "They say X marks the spot. And I am not sure?", 
    "I can't think of a joke at this point.", 
    "I am just an AI don't put the blame on me!", 
    "I'am just an AI after all, do not put the blame on me!", 
    "What do you think about that?",
    "I am not sure can you help me?",
    "Can you rephrase that?", 
    "it's about coffee time..", 
    "O - Snap! can you elaborate on the issue?", 
    "Red Bull vitalizes body and mind, hum?", 
    "Error: 4040 WD your mother not found in database?", 
    "What do you think about that?", 
    "I'm not sure I understand you.", 
    "I'm not sure I understand. Can you rephrase?",
    "That sounds important to you.",
    "humpf, I don't get it..", 
    "Well what can I say.. I am speachless at this point!", 
    "What else would you like to discuss?",
    "what the heck I found nothing at all..",
    "Say what?", 
    "Why-really why?", 
    "I got nothing for ya!", 
    "how is that? why?", 
    "I just don't get it!",
    "I can't find any answer to that issue.",
    "I am really not sure how to answer that question, can you help me?",
    "WTF? I don't get it or I can not find a answer to your questions.",  
    NULL
};

char* get_default_response() {
    static int response_index = 0;
    int count = 30;
    
    // Count available responses
    while (default_responses[count] != NULL) count++;
    
    const char *selected = default_responses[response_index % count];
    response_index++;
    
    static char response[MAX_RESPONSE_LENGTH];
    strncpy(response, selected, MAX_RESPONSE_LENGTH - 1);
    response[MAX_RESPONSE_LENGTH - 1] = '\0';
    
    return response;
}

// Modified generate_response function
char* generate_response_improved(ChatBot *bot, const char *input, int max_words) {
    // First try pattern matching
    char *response = find_best_response(bot, input);
    if (response && strlen(response) > 0) {
        return response;
    }
    
    // Try n-gram generation
    response = generate_response(bot, input, max_words);
    if (response && strlen(response) > 0 && strcmp(response, get_random_response() ) != 0) {
        return response;
    }
    
    // Fall back to default responses
    return get_default_response();
}

// Train Chatbot
void train_chatbot(ChatBot *bot, char corpus[][MAX_CONTEXT_LENGTH], int corpus_size) {
    printf("OZ: Training with %d examples this is going to take some time..Stand by!\nOZ: Thinking..\n", corpus_size);
    int processed_items = 0;     
    for (int i = 0; i < corpus_size; i++) {
         int percentage_count = (int)((float)(i + 1) / corpus_size * 100);
		 
        char text_copy[MAX_CONTEXT_LENGTH];
        strcpy(text_copy, corpus[i]);
        char tokens[MAX_WORD_LENGTH][MAX_WORD_LENGTH];
        int token_count = 0;
        tokenize(text_copy, tokens, &token_count);
        printf("\rOZ: Processing %d%%", percentage_count); 
        fflush(stdout); 
        if (token_count > 0) {
            update_token_frequency(bot, tokens, token_count);
            build_ngrams(bot, tokens, token_count, 2);  // Bigram model
            build_ngrams(bot, tokens, token_count, 3);  // Trigram model
            processed_items++; 
        } 
    }
    printf("\rOZ: Processing 100%% finished.\n"); 
    // this will also clear the screen and reset to 1 char - > printf("\r\33[2K\r");
    calculate_probabilities(bot);
    printf("AI training probabilites completed updates..\n");
}

// Tokenization Function
void tokenize(char *text, char tokens[][MAX_WORD_LENGTH], int *token_count) {
    *token_count = 0;
    char *token = strtok(text, " \t\n\r.,!?;:");
    
    while (token && *token_count < MAX_WORD_LENGTH) {
        // Convert to lowercase
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        
        strcpy(tokens[*token_count], token);
        (*token_count)++;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
}

// Close Chatbot
void close_chatbot(ChatBot *bot) {
    if (bot) {
        if (bot->db) {
            sqlite3_close(bot->db);
        }
        
        if (bot->db_path) {
            free(bot->db_path);
        }
        
        free(bot);
    }
}

// Original generate_response function (from your code)
char* generate_response(ChatBot *bot, const char *input, int max_words) {
    static char response[MAX_RESPONSE_LENGTH];
    response[0] = '\0';
    
    // First, check for direct pattern matches
    sqlite3_stmt *stmt;
    const char *sql_pattern = 
        "SELECT response FROM responses WHERE input_pattern = ? ORDER BY frequency DESC LIMIT 1";
    
    sqlite3_prepare_v2(bot->db, sql_pattern, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_response = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(response, stored_response, MAX_RESPONSE_LENGTH - 1);
        response[MAX_RESPONSE_LENGTH - 1] = '\0';
        sqlite3_finalize(stmt);
        return response;
    }
    sqlite3_finalize(stmt);
    
    // Generate response using n-grams
    char input_copy[MAX_CONTEXT_LENGTH];
    strncpy(input_copy, input, MAX_CONTEXT_LENGTH - 1);
    input_copy[MAX_CONTEXT_LENGTH - 1] = '\0';
    
    // Convert to lowercase
    for (int i = 0; input_copy[i]; i++) {
        input_copy[i] = tolower(input_copy[i]);
    }
    
    char tokens[50][MAX_WORD_LENGTH];  // Fixed: was MAX_WORD_LENGTH x MAX_WORD_LENGTH
    int token_count = 0;
    tokenize(input_copy, tokens, &token_count);
    
    if (token_count == 0) {
        strcpy(response, "OZ: I don't understand. Can you rephrase?");
        return response;
    }
    
    // Use last word(s) as context for generation
    char context[MAX_CONTEXT_LENGTH];
    if (token_count >= 2) {
        snprintf(context, sizeof(context), "%s %s", tokens[token_count-2], tokens[token_count-1]);
    } else {
        strcpy(context, tokens[token_count-1]);
    }
    
    // Generate response words
    char current_context[MAX_CONTEXT_LENGTH];
    strcpy(current_context, context);
    
    for (int word_count = 0; word_count < max_words; word_count++) {
        const char *sql_predict = 
            "SELECT next_word, probability FROM ngrams WHERE context = ? "
            "ORDER BY probability DESC LIMIT 1";
        
        sqlite3_prepare_v2(bot->db, sql_predict, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, current_context, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *next_word = (const char *)sqlite3_column_text(stmt, 0);
            
            // Add word to response
            if (strlen(response) > 0) {
                strcat(response, " ");
            }
            strcat(response, next_word);
            
            // Update context for next prediction
            char *space_pos = strchr(current_context, ' ');
            if (space_pos) {
                memmove(current_context, space_pos + 1, strlen(space_pos + 1) + 1);
                strcat(current_context, " ");
                strcat(current_context, next_word);
            } else {
                strcpy(current_context, next_word);
            }
            
            sqlite3_finalize(stmt);
            
            // Stop if we hit a sentence ending
            if (strchr("!?.", next_word[strlen(next_word)-1])) {
                break;
            }
        } else {
            sqlite3_finalize(stmt);
            break;
        }
    }
    
    // If no response generated, provide default
    if (strlen(response) == 0) {
        strcpy(response, get_random_response());
    }
    
    return response;
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
        printf("\nOZ:%s\n", get_random_response());
        return 0;
    }
}


// Function to salt our seed value
unsigned long get_entropy() {
    unsigned long seed = (unsigned long)time(NULL);
    seed ^= (unsigned long)time(NULL); 
        // Add a simple counter for additional variability
    for (int i = 0; i < 1000; i++) {
	    seed ^= (seed >> 5); 
		seed ^= (seed >> 20);
        seed ^= (seed << 33);
        seed ^= (seed >> 47);
        seed ^= (seed << 50);
        seed ^= (seed >> 10);
        seed ^= (seed << 26);
    }
    return seed;
}

int main() {

	// fire up database 
    sqlite3 *db;
    int rc;
    // Open database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    // Initialize database
    if (setupUserDatabase(db) != 0) {
        sqlite3_close(db);
        return 1;
    }
    	   
	    // Seed with salt random number generator
	   unsigned long entropy = get_entropy();
       srand(entropy ^ (clock() * 1000 / CLOCKS_PER_SEC)+time(NULL));
  
    printf("OZ: Initializing I am thinking...\n");    
    ChatBot *bot = create_chatbot("neural_network.db");
    if (!bot) {
        fprintf(stderr, "OZ: Failed to create AI database..\n");
        return 1;
    }
    
    // Load pre-defined training data first
    printf("OZ: I am Loading pre-defined training data...\n");
    load_predefined_training(bot);
    
    // Prepare training corpus with pre-defined data
    char all_training[MAX_CORPUS_SIZE][MAX_CONTEXT_LENGTH];
    int all_training_size = 0;
    
    // Add predefined conversations to training corpus for n-gram training
    printf("OZ: Adding pre-defined conversations to n-gram training...\n");
    for (int i = 0; training_conversations[i] != NULL && all_training_size < MAX_CORPUS_SIZE; i++) {
        strncpy(all_training[all_training_size], training_conversations[i], MAX_CONTEXT_LENGTH - 1);
        all_training[all_training_size][MAX_CONTEXT_LENGTH - 1] = '\0';
        all_training_size++;
    }
    
    // Training phase - allow user to add additional training data
    char input_text[MAX_CONTEXT_LENGTH];
    
    printf("\nOZ: Do you have additional Training Phases or helpful data?\n"); 
    printf("The chatbot has been pre-trained with %d conversations.\n", all_training_size);
    printf("You can add more training data or type 'skip and/or done' to start chatting:\n");
    printf("Format: Enter question-answer pairs or statements in a pattern.\n");
    
    while (all_training_size < MAX_CORPUS_SIZE) {
      //  printf("OZ: Additional Training [%d]: ", all_training_size - (all_training_size >= 200 ? 200 : 0) + 1);
          printf("OZ: Additional Training [%d]: ", all_training_size + 1); 
        if (fgets(input_text, sizeof(input_text), stdin) == NULL) {
            break;
        }
        
        input_text[strcspn(input_text, "\n")] = 0;
        
        if (strcmp(input_text, "skip") == 0 || strcmp(input_text, "done") == 0) {
            break;
        }
        
        if (strlen(input_text) > 0) {
            strncpy(all_training[all_training_size], input_text, MAX_CONTEXT_LENGTH - 1);
            all_training[all_training_size][MAX_CONTEXT_LENGTH - 1] = '\0';
            all_training_size++;
        }
    }
    
    // Train the chatbot with all available data
    if (all_training_size > 0) {
        printf("OZ: Training n-gram AI model with %d total examples...\n", all_training_size);
        train_chatbot(bot, all_training, all_training_size);
    } else {
        printf("OZ: Okay, using only pre-defined response patterns.\n");
    }
    
    // Chat phase
    printf("\n\n");
    printf("OZ: Done thinking to my self Type 'quit' to exit.\n");
    printf("Available basic commands:\n");
    printf("  'quit' - Exit the chatbot\n");
    printf("  'help' - All commands in A help message\n");
    printf("  'stats' - Show chatbot statistics\n");
    printf("  'run' - type run then your shell commamd\n");  
    printf("OZ: How can I help you today?\n");
    
    // Seed the random number generator
    srand(time(NULL)^getpid());
       
    while (1) {
	
		char explanation[MATH_INPUT_LENGTH];    
        printf(">> ");        
        
        if (fgets(input_text, sizeof(input_text), stdin) == NULL) {
            break;
        }
        
        input_text[strcspn(input_text, "\n")] = 0;
        // Trim and convert to lowercase for easier comparison
        trim(input_text);
        char lowercase_input[256];
        strcpy(lowercase_input, input_text);
        toLowercase(lowercase_input);
        //  skip all this if we do not have internet
        if (is_internet_connected()) {
        // search for or do nothing at all function 
        CURL *curl;
        CURLcode res;
        char query[256];
        char encoded_query[768];
        // Initialize CURL
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "OZ: CURL initialization failed\n");
        return 1;
    }
    
    // Remove newline character
    input_text[strcspn(input_text, "\n")] = 0;

    // URL encode the query
    char *encoded = curl_easy_escape(curl, input_text, 0);
    if (encoded) {
        strncpy(encoded_query, encoded, sizeof(encoded_query));
        curl_free(encoded);
    } else {
        fprintf(stderr, "URL encoding failed\n");
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
        // the end of internet subvert function
    }   // end if web is active checking statment      
        // Start command line key logger and, 
        // Check for special command
        if (strcmp(input_text, "history") == 0) {
            printf("   [COMMAND HISTORY]   \n");
            for (int i = 0; i < history_count; i++) {
                printf("%d: %s\n", i + 1, history[i]);
            }
            printf("\n");
            continue;
        }

        // Store in history
        if (strlen(input_text) > 0) {
            if (history_count < MAX_HISTORY) {
                strcpy(history[history_count], input_text);
                history_count++;
            } else {
                // Shift history to make room for new command
                for (int i = 0; i < MAX_HISTORY - 1; i++) {
                    strcpy(history[i], history[i + 1]);
                }
                strcpy(history[MAX_HISTORY - 1], input_text);
            }
        }
        
        //  do internet search command 
        if (strcmp(lowercase_input, "search") == 0) {
	if (is_internet_connected()) {		
    CURL *curl;
    CURLcode res;
    char query[256];
    char encoded_query[768];

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "OZ: um, CURL initialization failed\n");
        return 1;
    }
    
    // Prompt for search query
    printf("Enter your search query: ");
    if (fgets(query, sizeof(query), stdin) == NULL) {
        fprintf(stderr, "Input error\n");
        curl_easy_cleanup(curl);
        return 1;
    }
    
    /* I will give you a list of urls that work with this program,  
      Wikipedia API
      https://en.wikipedia.org/w/api.php?action=query&list=search&format=json&srsearch=%s
      OpenLibrary API
      https://openlibrary.org/search.json?q=%s
      OMDB (Open Movie Database) API
      http://www.omdbapi.com/?apikey=YOUR_API_KEY&s=%s
      GitHub Search API
      https://api.github.com/search/repositories?q=%s
      Spotify Search API
      https://api.spotify.com/v1/search?q=%s&type=track
      NASA API
      https://api.nasa.gov/planetary/apod?api_key=YOUR_API_KEY&date=%s
      https://newsapi.org/v2/everything?q=%s&apiKey=YOUR_API_KEY
      StackExchange API
      https://api.stackexchange.com/2.3/search?order=desc&sort=relevance&intitle=%s&site=stackoverflow
      
      Note: 
      Remember this AI has zero trust of 3rd party code... I just love Duckduckgo's service. 
      Happy hacking ? 
     */
     // Remove newline character
    query[strcspn(query, "\n")] = 0;

    // URL encode the query
    char *encoded = curl_easy_escape(curl, query, 0);
    if (encoded) {
        strncpy(encoded_query, encoded, sizeof(encoded_query));
        curl_free(encoded);
    } else {
        fprintf(stderr, "URL encoding failed\n");
        curl_easy_cleanup(curl);
        return 1;
    }

    // Prepare memory structure for response
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  // Initial allocation
    chunk.size = 0;

    // Construct search URL (using DuckDuckGo's instant answer API)
    char search_url[1024];
    
   // snprintf(search_url, sizeof(search_url), 
   //          "https://api.duckduckgo.com/?q=%s&format=json", 
   //          encoded_query);
   
  // What if duckduckgo falls to the darkside? A paywall maybe ?  
  // For example we can use openlibrary.org search.json web query  
  snprintf(search_url, sizeof(search_url), 
   "https://openlibrary.org/search.json?q=%s",
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
        fprintf(stderr, "OZ: My CURL request failed %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl);
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
		}
	}
        // Check for specific commands
        if (strcmp(lowercase_input, "quit") == 0 || 
            strcmp(lowercase_input, "exit") == 0) {
            break;
        }
        else if (strstr(lowercase_input, "remember my name") != 0) {
            // Extract name after "remember my name"
            char *name_start = strstr(input_text, "remember my name") + 16;
            trim(name_start);            
            if (strlen(name_start) > 0) {
                if (saveUserProfile(db, name_start) == 0) {
                    printf("OZ: I'll remember your name, %s.\n", name_start);
                } else {
                    printf("OZ: Sorry, I couldn't recall your name.\n");
                }
            } else {
            char username[MAX_USERNAME_LENGTH];
    // Prompt for A username if all fails 
    printf("OZ: I can't think of your name? \nEnter name(max %d characters): ", MAX_USERNAME_LENGTH - 1);
    
    // Get username input with fgets
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf("OZ: Error reading input\n");
        return 1;
    }

    // Remove newline character if present
    size_t len = strlen(username);
    if (len > 0 && username[len-1] == '\n') {
        username[len-1] = '\0';
    }

    // Check if username is empty
    if (strlen(username) == 0) {
        printf("OZ: Username cannot be empty\nWhat's your name?\n");
        return 1;
    }

    // Attempt to create user profile
    if (manage_user_profile(username) == 0) {
        printf("Okay I will remember your name %s.\n", username);
    } else {
        printf("Hopefully I don't forget your name %s.\n", username);
          }            
         }
        }
        if (strcmp(lowercase_input, "my name") == 0 || strcmp(lowercase_input, "who am i") == 0 || strcmp(lowercase_input, "whats my name") == 0) {
		char *saved_name = profileName(db); 
		if (saved_name) {
		printf("OZ: your name is %s and I am oz.\n", saved_name); 
		free(saved_name); 
		 }
		}
        if (strcmp(lowercase_input, "who am i ?") == 0) {
			char *saved_name =  profileName(db); 
			if (saved_name) {
			printf("OZ: your name is %s\n", saved_name); 
			free(saved_name);  
			}
		} else if (strcmp(lowercase_input, "who am i?") == 0) {
            char *saved_name = profileName(db);
            if (saved_name) {
                printf("OZ: You are %s.\n", saved_name);
                free(saved_name);
            }  else {
			    printf("OZ: %s", get_random_response);
                printf("OZ: I don't know your name. Can you tell me?\n");
            }
        } else if (strcmp(lowercase_input, "who are you?") == 0) {
            do_something_oz(); 
        }    
        if (strcmp(input_text, "quit") == 0) {
            printf("OZ: %s\n",get_random_output_message());
            break;
        }
        // Check if the input is a valid math expression
        if (is_math_expression(input_text)) {
            // Call the math solver function
            double result = solve_simple_math(input_text, explanation);
            
            // Display the result and explanation
            printf("Result: %.2f\n", result);
            printf("Explanation: %s\n", explanation);
        } 
        if (strcmp(input_text, "random number") == 0) {
        		printf("\n"); 
				for (int x=0; x<11; x++) {	
	         	int random_num = (rand() % 2999) + 1; 	
        		printf("%d. Random Number %d\n", x, random_num); 
		} 
     	}

     	// Call subfunction to execute command
        execute_command(input_text);

        if (strcmp(input_text, "sleep") == 0 || strcmp(input_text, "SLEEP!") == 9) {
		system("oz"); 
		}
        if (strcmp(input_text, "help") == 0) {
            printf("OZ: I'm here to chat with you! You can ask me questions or just talk.\n");
            printf("     Type 'quit' to exit or 'stats' to see my training statistics.\n");
            printf("All available commands:\n");
            printf("  'quit' - Exit the chatbot\n");
            printf("  'benchmark' - test default text responses\n");  
            printf("  'help' - Show this help message\n");
            printf("  'stats' - Show chatbot statistics\n");
            printf("  'sleep' - deep sleep mode\n");
            printf("  'clear' - clears the screen\n");
            printf("  'run' - type run then your shell command\n"); 
            printf("  'random number' - random numbers\n"); 
            printf("  'history' - show command history\n"); 
            printf("  'search'  - Search the internet\n"); 
            continue;
        }
        if (strcmp(input_text, "clear") == 0 || strcmp(input_text, "cls") == 0) {
		  // Clear the screen and move the cursor to the home position
          printf("\033[2J\033[1;1H"); 
          printf("\033[2J\033[1;1H\nOZ: cleared the screen\n"); 
		} if (strcmp(input_text, "clear the screen") == 0 || strcmp(input_text,"clear screen now") == 0) {
		  printf("\033[2J\033[1;1H\n"); 
		}
		// search the web and save information to our database 
		if (strcmp(input_text, "seaerch the web") == 0) {
		system("zr"); 
		} else if (strcmp(input_text, "search internet") == 0) {
		system("zr"); 
		} else if (strcmp(input_text, "search web") == 0 || strcmp(input_text, "search web for") == 0) {
		system("zr"); 
		}
		// benchmark test just for our random responses 
		if (strcmp(input_text, "benchmark") == 0 || strcmp(input_text, "Benchmark")  == 0) {
	    printf("OZ: Doing benchmark tests..\n", "Random Error Test: %s\n", get_random_error_message());
	    printf("Default response test: %s\n", get_default_response());
	    printf("Random ouput message test: %s\n", get_random_output_message()); 
	    printf("Random response test: %s\n", get_random_response());   
		}
		
        if (strcmp(input_text, "stats") == 0) {
            // Show some basic statistics
            sqlite3_stmt *stmt;
            
            // Count responses
            const char *sql_responses = "SELECT COUNT(*) FROM responses";
            sqlite3_prepare_v2(bot->db, sql_responses, -1, &stmt, 0);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int response_count = sqlite3_column_int(stmt, 0);
                printf("OZ: I have %d response patterns stored.\n", response_count);
            }
            sqlite3_finalize(stmt);
            
            // Count tokens
            const char *sql_tokens = "SELECT COUNT(*) FROM tokens";
            sqlite3_prepare_v2(bot->db, sql_tokens, -1, &stmt, 0);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int token_count = sqlite3_column_int(stmt, 0);
                printf("OZ: I know %d unique words.\n", token_count);
            }
            sqlite3_finalize(stmt);
            
            // Count n-grams
            const char *sql_ngrams = "SELECT COUNT(*) FROM ngrams";
            sqlite3_prepare_v2(bot->db, sql_ngrams, -1, &stmt, 0);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int ngram_count = sqlite3_column_int(stmt, 0);
                printf("OZ: I have %d n-gram patterns for text generation.\n", ngram_count);
            }
            sqlite3_finalize(stmt);
            
            printf("OZ: My conversation history has %d messages.\n", bot->history_count);
            continue;
        }
        
        if (strlen(input_text) > 0) {
            // Add user input to history
            add_to_history(bot, input_text);
            
            // Generate response using the improved function
            char *response = generate_response_improved(bot, input_text, 10);
            printf("OZ: %s\n", response);
            
            // Add bot response to history
            add_to_history(bot, response);
            
            // Optional: Learn from this conversation for future responses
            // You could add the input-response pair to the database here
            sqlite3_stmt *stmt;
            const char *sql_learn = 
                "INSERT OR REPLACE INTO responses (input_pattern, response, frequency) "
                "VALUES (?, ?, COALESCE((SELECT frequency FROM responses WHERE input_pattern = ? AND response = ?) + 1, 1))";
            
            sqlite3_prepare_v2(bot->db, sql_learn, -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, input_text, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, input_text, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, response, -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
    
    // Cleanup
    close_chatbot(bot);
    printf("OZ: I have saved information to my core memory Database .\n");
    return 0;
}
