#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

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
        printf("OZ: Well I was searching for joke but no jokes are found? \n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


// Function to create the questions table if it doesn't exist
void createTable(sqlite3* db) {
	    // First, create table if it doesn't exist
    const char* createSql = "CREATE TABLE IF NOT EXISTS questions (question TEXT PRIMARY KEY)";
    sqlite3_exec(db, createSql, NULL, NULL, NULL);
    // Then add response column if it doesn't exist
    const char* alterSql = "ALTER TABLE questions ADD COLUMN response TEXT";
    sqlite3_exec(db, alterSql, NULL, NULL, NULL); // This will fail silently if column exists

    const char* sql = "CREATE TABLE IF NOT EXISTS questions (question TEXT PRIMARY KEY, response TEXT)";
    char* errorMessage;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK) {
        //fprintf(stderr, "SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
    }    
}

// Function to insert a question and response into the database
void insertQuestion(sqlite3* db, const char* question, const char* response) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO questions (question, response) VALUES (?, ?)";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "\nSQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, question, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        //fprintf(stderr, "SQL step error: %s\n", sqlite3_errmsg(db));
        
    }
    sqlite3_finalize(stmt);
}

// Function to enter new storys into table story 
//void insertStory(sqlite3* db, const char*story) {
//    sqlite3_stmt* stmt;
//    const char* sql = "INSERT INTO stories (story) VALUES (?)";
//    
//    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//    if (rc != SQLITE_OK) {
//        fprintf(stderr, "\nSQL prepare error: %s\n", sqlite3_errmsg(db));
//        return;
//    }
//    sqlite3_bind_text(stmt, 1, story, -1, SQLITE_STATIC);
//    rc = sqlite3_step(stmt);
//    sqlite3_finalize(stmt);
//}
void insertStory(sqlite3* db, const char* story) {
    const char* sql = "CREATE TABLE IF NOT EXISTS stories (story TEXT UNIQUE); "
                      "INSERT OR IGNORE INTO stories (story) VALUES (?)";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "\nSQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, story, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nSQL insert error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// Function to add jokes to your database 
void insertJoke(sqlite3* db, const char* joke) {
    const char* sql = "CREATE TABLE IF NOT EXISTS jokes (joke TEXT UNIQUE); "
                      "INSERT OR IGNORE INTO jokes (joke) VALUES (?)";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "\nSQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, joke, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nSQL insert error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// Function to get the response for a given question
void getResponse(sqlite3* db, const char* question) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT response FROM questions WHERE question = ?";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        //fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, question, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* response = sqlite3_column_text(stmt, 0);
        printf("OZ: %s\n", response);
    } else {
   // Define an array of random responses
    char *random_responses[] = {
        "I'm not sure I understand the question.",
        "Can you please rephrase the question?",
        "I don't have enough information to answer that.",
        "That's a tough one, I'll have to get back to you.",
        "I'm still learning, can you please provide more context?",
        "I've never heard of that before.",
        "I'm not familiar with that topic.",
        "Can you give me an example?",
        "I'm not sure what you're getting at.",
        "Let me think about that for a moment.",
        "I'm not aware of any information on that subject.",
        "I'd rather not speculate on that.",
        "I'm not comfortable discussing that topic.",
        "I think I need more information to give a good answer.",
        "That's a complex question, can you break it down?"
    };

    int size = sizeof(random_responses) / sizeof(random_responses[0]);

    // Get the current time
    time_t currentTime = time(NULL);

    // Use the current time as an index into the array
    int index = currentTime % size;

    // Print the random response
    printf("OZ: %s\n", random_responses[index]);

    }
    sqlite3_finalize(stmt);
}



/* Function to check if a table exists */
int table_exists(sqlite3 *db, const char *table_name) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    int exists = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, table_name, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = 1;
        }
    }
    sqlite3_finalize(stmt);
    return exists;
}

/* Function to check if a column exists in a table */
int column_exists(sqlite3 *db, const char *table_name, const char *column_name) {
    sqlite3_stmt *stmt;
    char sql[256];
    int exists = 0;
    
    snprintf(sql, sizeof(sql), "PRAGMA table_info(%s);", table_name);
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *col_name = (const char*)sqlite3_column_text(stmt, 1);
            if (col_name && strcmp(col_name, column_name) == 0) {
                exists = 1;
                break;
            }
        }
    }
    sqlite3_finalize(stmt);
    return exists;
}

/* Function to create the questions table if it doesn't exist */
int create_questions_table(sqlite3 *db) {
    const char *sql = "CREATE TABLE IF NOT EXISTS questions ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "question TEXT, "
                     "response TEXT);";
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Error creating questions table %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }
    
    //printf("Questions table created or already exists.\n");
    return 1;
}

/* Function to create the Responses table if it doesn't exist */
int create_responses_table(sqlite3 *db) {
    const char *sql = "CREATE TABLE IF NOT EXISTS Responses ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "UserInput TEXT, "
                     "BotResponse TEXT);";
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: Error creating Responses table %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }
    
   // printf("Responses table created or already exists.\n");
    return 1;
}

/* Function to add missing columns to existing tables */
int add_missing_columns(sqlite3 *db) {
    char *err_msg = NULL;
    int rc;
    
    /* Check and add question column to questions table */
    if (table_exists(db, "questions") && !column_exists(db, "questions", "question")) {
        rc = sqlite3_exec(db, "ALTER TABLE questions ADD COLUMN question TEXT;", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "OZ: Error adding question column %s\n", err_msg);
            sqlite3_free(err_msg);
            return 0;
        }
       // printf("Added 'question' column to questions table.\n");
    }
    
    /* Check and add response column to questions table */
    if (table_exists(db, "questions") && !column_exists(db, "questions", "response")) {
        rc = sqlite3_exec(db, "ALTER TABLE questions ADD COLUMN response TEXT;", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "OZ: Error adding response column %s\n", err_msg);
            sqlite3_free(err_msg);
            return 0;
        }
        //printf("Added 'response' column to questions table.\n");
    }
    
    /* Check and add UserInput column to Responses table */
    if (table_exists(db, "Responses") && !column_exists(db, "Responses", "UserInput")) {
        rc = sqlite3_exec(db, "ALTER TABLE Responses ADD COLUMN UserInput TEXT;", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "OZ: Error adding UserInput column %s\n", err_msg);
            sqlite3_free(err_msg);
            return 0;
        }
        //printf("Added 'UserInput' column to Responses table.\n");
    }
    
    /* Check and add BotResponse column to Responses table */
    if (table_exists(db, "Responses") && !column_exists(db, "Responses", "BotResponse")) {
        rc = sqlite3_exec(db, "ALTER TABLE Responses ADD COLUMN BotResponse TEXT;", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "OZ: Error adding BotResponse column %s\n", err_msg);
            sqlite3_free(err_msg);
            return 0;
        }
       // printf("Added 'BotResponse' column to Responses table.\n");
    }
    
    return 1;
}

/* Function to copy data from questions table to Responses table
 * Mapping: questions.question -> Responses.UserInput
 *          questions.response -> Responses.BotResponse
 * Uses INSERT OR IGNORE to handle duplicate UserInput values
 */
int copy_data(sqlite3 *db) {
    const char *select_sql = "SELECT question, response FROM questions;";
    const char *insert_sql = "INSERT OR IGNORE INTO Responses (UserInput, BotResponse) VALUES (?, ?);";
    
    sqlite3_stmt *select_stmt, *insert_stmt;
    int rc;
    int count = 0;
    int skipped = 0;
    
    //printf("Starting data copy: questions.question -> Responses.UserInput\n");
    //printf("                   questions.response -> Responses.BotResponse\n");
    //printf("Using INSERT OR IGNORE to handle duplicate UserInput values.\n");
    
    /* Prepare select statement */
    rc = sqlite3_prepare_v2(db, select_sql, -1, &select_stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing select statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    
    /* Prepare insert statement */
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing insert statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        return 0;
    }
    
    /* Begin transaction for better performance */
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    
    /* Copy data: question -> UserInput, response -> BotResponse */
    while ((rc = sqlite3_step(select_stmt)) == SQLITE_ROW) {
        const char *question_text = (const char*)sqlite3_column_text(select_stmt, 0);
        const char *response_text = (const char*)sqlite3_column_text(select_stmt, 1);
        
        /* Bind parameters: question goes to UserInput, response goes to BotResponse */
        sqlite3_bind_text(insert_stmt, 1, question_text ? question_text : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 2, response_text ? response_text : "", -1, SQLITE_STATIC);
        
        /* Execute insert */
        int step_result = sqlite3_step(insert_stmt);
        if (step_result == SQLITE_DONE) {
            /* Check if row was actually inserted (not ignored due to duplicate) */
            if (sqlite3_changes(db) > 0) {
                count++;
            } else {
                skipped++;
            }
        } else {
            fprintf(stderr, "Error inserting row: %s\n", sqlite3_errmsg(db));
        }
        
        /* Reset statement for next iteration */
        sqlite3_reset(insert_stmt);
    }
    
    /* Commit transaction */
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    
    /* Clean up */
    sqlite3_finalize(select_stmt);
    sqlite3_finalize(insert_stmt);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "OZ: Error during select operation %s\n", sqlite3_errmsg(db));
        return 0;
    }
    
    // printf("Successfully copied %d new rows from questions table to Responses table.\n", count);
    if (skipped > 0) {
       // printf("Skipped %d duplicate UserInput entries.\n", skipped);
    }
   // printf("Data mapping completed: question -> UserInput, response -> BotResponse\n");
    return 1;
}


int main() {
    sqlite3* db;
    int rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "\nCannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    createTable(db);
     // train our reactive LLM 
     insertQuestion(db, "Hello", "Hi, how are you?");
     insertQuestion(db, "hello world", "Remember basic? \n01 print hello world\n02 goto 01\nOZ: hello from the internet with love!");
     insertStory(db, "In the dimly lit alleys of 4chans anonymous forums, a group of rogue clowns emerged, their bright orange wigs and painted-on smiles a stark contrast to the darkness of the internets underbelly. These clowns, known for their chaotic and subversive antics, delighted in pushing the boundaries of online discourse, often blurring the lines between reality and satire. With their trusty arsenal of memes and image macros, they set out to disrupt the status quo, leaving a trail of confusion and hilarity in their wake. As their legend grew, so did their mythology, with some claiming they were the guardians of internet chaos, while others saw them as harbingers of doom, bringing ruin to the very fabric of online communities. Nonetheless, the clowns of 4chan remained, a constant and mischievous presence, always lurking in the shadows, waiting to unleash their unique brand of anarchy upon the world.");
     insertStory(db, "In the sun-drenched realms of internet nostalgia, the lolcatz reigned supreme, their adorable feline faces and intentionally misspelled captions bringing joy and laughter to the masses. With their origins rooted in the ancient forums of 4chan and I Can Has Cheezburger, these whimsical creatures spread like wildfire, their cute and quirky antics captivating the hearts of netizens everywhere. As their popularity soared, the lolcatz became a cultural phenomenon, with their I Can Has Cheezburger? battle cry echoing through the digital halls, a rallying call for all who sought to indulge in the absurd and playful side of the internet. With their paws firmly planted in the world of memes, the lolcatz remained a beloved and enduring symbol of internet culture, a reminder of the power of humor and whimsy to bring people together in a shared experience of delight and amusement. Just torlling ya!");
     insertStory(db, "In the sleepy town of Crescent Falls, a sense of unease settled over the residents like a shroud, as a mysterious spaceship descended from the night sky, its cargo hold bursting with an army of killer clowns from outer space. These were no ordinary clowns, but rather grotesque, alien beings with skin like pale rubber and eyes that glowed like embers from the depths of hell. With their twisted, sinister grins and oversized shoes, they set out to wreak havoc on the unsuspecting town, leaving a trail of terror and destruction in their wake. Armed with an arsenal of deadly pranks and an otherworldly ability to manipulate the very fabric of reality, the killer clowns from outer space seemed almost unstoppable, their antics becoming increasingly bizarre and brutal as the night wore on. As the people of Crescent Falls cowered in fear, the clowns continued their rampage, their maniacal laughter echoing through the streets like a chilling reminder that, in a universe full of mysteries and terrors, sometimes the most absurd and fantastical horrors can be the most terrifying of all.");
     insertJoke(db, "Why did the lolcat join a band? Because it wanted to be the purr-cussionist."); 
     insertJoke(db, "Why did the lolcat go to the gym? To get some paws-itive reinforcement.");
     insertJoke(db, "Q: What did the lolcat say when it woke up? A:I can has coffee?");
     insertJoke(db, "Why did the crazy person bring a ladder to the party? They heard the drinks were on the house.");
     insertJoke(db, "What did the crazy person say when they ran into their ex? I am not stalking you, I am just really and passionately interested in your whereabouts.");
     insertJoke(db, "Why did the crazy person go to the doctor? My voices were feeling a little off and by a little off, they meant completely insane. that is just crazy doc!"); 
     insertJoke(db, " I am addicted to placebos, I could quit but it would not make a difference.");
     insertJoke(db, "So I am a vegetarian because I love animals, I also really hate plants. end of joke beep - boop!");
     insertJoke(db, "I am reading a book about anti-grabity, I find it impossible to put down or back up. End of bad joke beep - boop!");       
     insertQuestion(db, "hello", "Hi, how are you?");
     insertQuestion(db, "hi", "Hi, how are you?");
     insertQuestion(db, "who am i ?", "your name is Johnny B Stroud. I was created 2025 by you remember?");  
     insertQuestion(db, "Johnny B Stroud", "hello Johnny");
     insertQuestion(db, "Johnny B Stroud", "Hello Johnny B"); 
     insertQuestion(db, "what is my name", "Johnny B Stroud.. Is my training data correct?");
     insertQuestion(db, "run clear", "clearing the screen");
     insertQuestion(db, "how am i", "So your name is Johnny. Long time no chat");
     insertQuestion(db, "Johnny B Stroud", "So your name is Johnny B");
     insertQuestion(db, "who am i", "Okay, you are Johnny and I am OZ..");
     insertQuestion(db, "who am i ", "Okay, you are Johnny and I am OZ..");
     insertQuestion(db, "who am i ?", "Okay, you are Johnny and I am OZ..");     
     insertQuestion(db, "Johnny B stroud", "hello Johnny");
     insertQuestion(db, "Johnny B stroud", "Johnny B .. Yes I get it already! I am a fast study..");
     insertQuestion(db, "who am i?", "So you are Johnny B"); 
     insertQuestion(db, "Johnnny B Stroud", "hello Johnny");
     insertQuestion(db, "what is my name?", "Hello Johnny B"); 
     insertQuestion(db, " who am I?", "Welcome back Johnny B");
     insertQuestion(db, "okay", "what now ?"); 
     insertQuestion(db, "clear", "try typeing run clear And i will clear the screen"); 
     insertQuestion(db, "exit", "bye for now");
     insertQuestion(db, "hello", "Hi, how are you today?");
     insertQuestion(db, "run dir", "run dir /");
     insertQuestion(db, "oz?", "I am here.. That is my name.");
     insertQuestion(db, "oz", "Yes, how can I help you?");
     insertQuestion(db, "oz ?", "How may I help you?"); 
     insertQuestion(db, "run dir /", "okay listing files.");
     insertQuestion(db, "sure wish your ai", "My AI is okay I guess ... Did you ever fix that bug?"); 
     insertQuestion(db, "whur wish your ai", "excellent typo man.");
     insertQuestion(db, "sure wish your ai", "So you wish I was smart not just reactive by default.");
     insertQuestion(db, "sure wish your ia", "let me finish that -was smart- then the average bear!"); 
     insertQuestion(db, "cls", "Just type run clear already okay");
     insertQuestion(db, "Keyword", "inquiry is logical and checks with my goals");
     insertQuestion(db, "tell me a joke", "Maybe later on if I can think of one."); 
     insertQuestion(db, "tell me a joke", "My joke are not all bad are they teacher?"); 
     insertQuestion(db, "funny", "Really funny what else can I do for you?"); 
     insertQuestion(db, "run dir -a", "okay looking for hidden files in current path.");  
     insertQuestion(db, "run dir -a /", "I am looking for hidden files in this path.");
     insertQuestion(db, "HELLO", "how are you doing today?");
     insertQuestion(db, "HI", "whats up?"); 
     insertQuestion(db, "HEY", "how is it going?"); 
     insertQuestion(db, "can you do programming brackets or code?", "No that will conflict with my prime goals."); 
     insertQuestion(db, "GREETINGS", "how have you been?"); 
     insertQuestion(db, "SALUTATIONS", "whats new welcome user.."); 
     insertQuestion(db, "HOLA", "que tal?");
     insertQuestion(db, "ALOHA", "Welcome, how's life?");
     insertQuestion(db, "HOWDY", "whats happening space cowboy?"); 
     insertQuestion(db, "SALUTE", "hello, hows everything?");
     insertQuestion(db, "WASSUP", "what's good bro?"); 
     insertQuestion(db, "HIYA", "how are things?"); 
     insertQuestion(db, "YO", "whats A cracking egg?"); 
     insertQuestion(db, "SHALOM", "Welcome - how is your day?");
     insertQuestion(db, "CIAO", "hows tricks?");
     insertQuestion(db, "KONNICHIWA", "how do you do?");
     insertQuestion(db, "NAMASTE", "HI, how's your week?"); 
     insertQuestion(db, "BONJOUR", "hows your day going?"); 
     insertQuestion(db, "OLA", "hows it treating you human?");
     insertQuestion(db, "SALAAM", "how is your family? God is good and the way!"); 
     insertQuestion(db, "HEYO", "whats the latest?");
     insertQuestion(db, "HOLA AMIGO", "hows it going, friend?");
     insertQuestion(db, "AHOY", "how's the sea and salty are?"); 
     insertQuestion(db, "Good DAY", "hows your day? hopefully good I bet."); 
     insertQuestion(db, "WELL MET", "how have you been?");
     insertQuestion(db, "SALUT", "hows everything with you? Much love heart!");
     insertQuestion(db, "CZEŚĆ", "hows life treating you?");
     insertQuestion(db, "HOLA AMIGAO", "hows it going, friend?"); 
     insertQuestion(db, "hola amigao", "good how about you, friend?");  
     insertQuestion(db, "MORNING", "how did you sleep?");
     insertQuestion(db, "morning", "how did you sleep?"); 
     insertQuestion(db, "EVENING", "how was your day?");
     insertQuestion(db, "good evening", "great, how was your day?");
     insertQuestion(db, "KIA ORA", "hows it going?");
     insertQuestion(db, "TALOFA", "how are you doing?");
     insertQuestion(db, "NAMASKAR", "hows your day been?");
     insertQuestion(db, "SAVASANA", "hows your journey in life?"); 
     insertQuestion(db, "HOLA QUE TAL", "how are you doing?"); 
     insertQuestion(db, "JAMBO", "hows your world?");
     insertQuestion(db, "BULA", "hows your health?"); 
     insertQuestion(db, "bula", "hows your health?");
     insertQuestion(db, "KONNICHIHA", "hows your mood?");
     insertQuestion(db, "konnichiha", "hello!"); 
     insertQuestion(db, "SHOUTOUT", "whats on your mind?");
     insertQuestion(db, "ALOHA KAKO", "how's everyone?");
     insertQuestion(db, "HOLA A TODOS", "hows everyone doing?");
     insertQuestion(db, "WASSUP HOMIE", "whats going on, buddy?");
     insertQuestion(db, "WHATS YOUR NAME?", "My name is OZ.");
     insertQuestion(db, "CAN I ASK YOUR NAME?", "My name is OZ.");
     insertQuestion(db, "MAY I KNOW YOUR NAME?", "My name is OZ.");
     insertQuestion(db, "WHAT DO YOU GO BY?", "They call me OZ."); 
     insertQuestion(db, "HOW SHOULD I ADDRESS YOU?", "Call me OZ if you like."); 
     insertQuestion(db, "WHAT NAME DO YOU PREFER?", "Mr. OZ.");
     insertQuestion(db, "what do you go by", "You can call me OZ");
     insertQuestion(db, "what do you go by?", "You can call me OZ!"); 
     insertQuestion(db, "WHAT ARE YOU CALLED?", "CALL ME OZ!");
     insertQuestion(db, "WHAT IS YOUR NAME, PLEASE?", "My name is OZ.");
     insertQuestion(db, "HOW DO YOU INTRODUCE YOURSELF?", "I start with my name OZ.");
     insertQuestion(db, "WHAT NAME DO YOU USE?", "Call me OZ");
     insertQuestion(db, "WHO AM I SPEAKING WITH?", "OZ the helpful god loveing AI.");
     insertQuestion(db, "WHAT DO YOU LIKE TO BE CALLED?", "OZ PLSz? thanks for asking me.");
     // reinforce AI id and questions that may be asked by user 
     insertQuestion(db, "WHATS YOUR MONIKER?", "OZ");
     insertQuestion(db, "whats your moniker?", "Well you can call me OZ."); 
     insertQuestion(db, "WHAT NAME SHOULD I USE?", "OZ call me that.");
     insertQuestion(db, "HOW DO YOU PREFER TO BE ADDRESSED?", "Mr. OZ.");
     insertQuestion(db, "WHAT IS YOUR FULL NAME?", "Mr. OZ"); 
     insertQuestion(db, "CAN YOU TELL ME YOUR NAME?", "My name is OZ okay."); 
     insertQuestion(db, "WHAT NAME DO YOU GO BY?", "OZ."); 
     insertQuestion(db, "WHAT IS YOUR GIVEN NAME?", "Mr. OZ. please and thank you!");
     insertQuestion(db, "WHAT IS YOUR FIRST NAME?", "OZ");
     insertQuestion(db, "what's your name? ", "my name is OZ");
     insertQuestion(db, "can I ask your name?", "yes you can my name is OZ buddy.");
     insertQuestion(db, "may I know your name?", "my name is OZ for sure."); 
     insertQuestion(db, "what do you go by?", "OZ I am a prototype reactive AI.");
     insertQuestion(db, "how should I address you?", "My name is OZ");
     insertQuestion(db, "what name do you prefer for a name?", "My name is OZ");
     insertQuestion(db, "what are you called?", "my name is OZ");
     insertQuestion(db, "what is your name, please?", "my name is OZ, I am A reactive AI.");
     insertQuestion(db, "how do you introduce yourself?", "OZ is my name. okay..");
     insertQuestion(db, "what name do you use?", "my name is OZ");
     insertQuestion(db, "who am I speaking with?", "my name is OZ");
     insertQuestion(db, "who are you?", "My name is OZ"); 
     insertQuestion(db, "who are you", "Well you can call me by my name OZ");
     insertQuestion(db, "what do you like to be called?",  "my name is OZ");
     insertQuestion(db, "whats your moniker?", "my name is OZ");
     insertQuestion(db, "what name should I use?", "my name is OZ that I can say for sure."); 
     insertQuestion(db, "how do you prefer to be addressed?", "my name is OZ and I am not a IP address.");     
     insertQuestion(db, "what is your full name?", "my name is OZ.. Number 5 is alive and 0 is a magic number!");
     insertQuestion(db, "can you tell me your name?", "my name is OZ");
     insertQuestion(db, "what name do you go by?", "my name is oz");
     insertQuestion(db, "what is your given name?", "my name is Oz");
     insertQuestion(db, "program id?", "My name is OZ");
     insertQuestion(db, "what is your first name?", "OZ and I am a wizard");
     insertQuestion(db, "what is your name?", "My name is oz okay."); 
     insertQuestion(db, "what is your name", "My name is oz. got it!");
     insertQuestion(db, "What is your name?", "My Name is oz for the love of god! I hope this is the last time you ask me.");
     insertQuestion(db, "clear", "clearing the screen. you can type run clear and I will do it.. By your command.."); 
     insertQuestion(db, "hello ", "hi how are you today!");
     insertQuestion(db, "exit", "What is the point of quitting now! Well bye then.");
     insertQuestion(db, "quit", "exiting");
     insertQuestion(db, "i think that",  "sorry I forget what I was saying ..");
     insertQuestion(db, "i think", "there for I am OZ.");
     insertQuestion(db, "HI ", "How are you?");
     insertQuestion(db, "HELLO OZ", "Hows it going? thank you for calling me that ok.");
     insertQuestion(db, "HEY", "Whats up ?");
     insertQuestion(db, "GOOD MORNING ", "How did you sleep?");
     insertQuestion(db, "GOOD AFTERNOON", "How's your day?");
     insertQuestion(db, "logic check", "systems are in a state of filtered.");
     insertQuestion(db, "GOOD EVENING ", "How have you been?");
     insertQuestion(db, "WASSUP", "What's new with you?");
     insertQuestion(db, "HOWDY ", "How's everything?");
     insertQuestion(db, "SALUTATIONS ", "Whats happening?");
     insertQuestion(db, "HIYA", "Hows life treating you today?"); 
     insertQuestion(db, "ALOHA", "Hows your day going nerd?");
     insertQuestion(db, "YO", "Yo-YO MA! What's good?");
     insertQuestion(db, "GREETINGS ", "How are things?");
     insertQuestion(db, "WHAT'S COOKING", "How's your 80s week?");
     insertQuestion(db, "HOWS IT GOING ", "Thinking .. Whats on your mind?");
     insertQuestion(db, "WHATS UP", "Hows your day been?");
     insertQuestion(db, "HI THERE", "Hows your life going man?");
     insertQuestion(db, "HOWS LIFE", "What have you been up to?");
     insertQuestion(db, "SUP!", "Hows your mood?"); 
     insertQuestion(db, "SALUTE ", "Hows your health?");
     // logic arrary of facts to recall for user 
     insertQuestion(db, "WHAT IS THE CAPITAL OF FRANCE", "The capital of France is Paris.");
     insertQuestion(db, "what is the capital of france", "the capital is Paris."); 
     insertQuestion(db, "HOW MANY PLANETS ARE IN THE SOLAR SYSTEM", "There are eight planets confirmed in the solar system at this time.");
     insertQuestion(db, "how many planets are in the solar system?", "there are eight and I am guessing off hand.");     
     insertQuestion(db, "WHAT IS THE LARGEST OCEAN ON EARTH", "The largest ocean on Earth is the Pacific Ocean.");
     insertQuestion(db, "largest ocean on earth", "The largest ocen on Earch is the Pacific Ocean");
     insertQuestion(db, "WHO WROTE ROMEO AND JULIET", "Romeo and Juliet was written by William Shakespeare."); 
     insertQuestion(db, "who wrote romeo and juliet", "William Shakespeare");
     insertQuestion(db, "WHAT IS THE SPEED OF LIGHT", "The speed of light is approximately 299,792 kilometers per second.");
     insertQuestion(db, "what is the speed of light", "the speed of light is approximately 229,792 kilometerrs per second.");
     insertQuestion(db, "WHAT IS THE CHEMICAL SYMBOL FOR WATER", "The chemical symbol for water is H2O.");
     insertQuestion(db, "what is the symbol for water", "H2O");     
     insertQuestion(db, "HOW LONG DOES IT TAKE FOR THE EARTH TO ORBIT THE SUN", "It takes about 365.25 days for the Earth to orbit the Sun.");
     insertQuestion(db, "how log does it take for the Earth to Orbit the Sun?", "it takes about 365.25 days for the Earth to orbit the Sun.");
     insertQuestion(db, "WHAT IS THE SMALLEST COUNTRY IN THE WORLD", "The smallest country in the world is Vatican City.");
     insertQuestion(db, "smallest country in the world?", "Vatican City");
     insertQuestion(db, "WHO DISCOVERED PENICILLIN", "Penicillin was discovered by Alexander Fleming.");
     insertQuestion(db, "what guy discoverd penicillin?", "Alexander Fleming.");
     insertQuestion(db, "WHAT IS THE MAIN INGREDIENT IN GUACAMOLE", "The main ingredient in guacamole is avocado.");
     insertQuestion(db, "what is guacamole?", "the main ingredient in guacamole is avocado and it is wonderful on chips");
     insertQuestion(db, "WHAT IS THE LARGEST DESERT IN THE WORLD", "The largest desert in the world is the Antarctic Desert.");
     insertQuestion(db, "what is the largest desert in the world?", "Antarctic Desert");
     insertQuestion(db, "WHO PAINTED THE MONA LISA", "The Mona Lisa was painted by Leonardo da Vinci.");
     insertQuestion(db, "who painted the mona lisa?", "Lenoardo da Vinci.");
     insertQuestion(db, "WHAT IS THE LONGEST RIVER IN THE WORLD", "The longest river in the world is the Nile River.");
     insertQuestion(db, "WHAT ELEMENT HAS THE CHEMICAL SYMBOL O", "The chemical symbol O represents oxygen.");
     insertQuestion(db, "HOW MANY CONTINENTS ARE THERE", "There are seven continents on Earth.");
     insertQuestion(db, "how many contients are there?", "there are seven on Earth");
     insertQuestion(db, "WHAT IS THE CURRENCY OF JAPAN", "The currency of Japan is the yen.");
     insertQuestion(db, "WHO WAS THE FIRST PERSON TO WALK ON THE MOON", "The first person to walk on the Moon was Neil Armstrong.");
     insertQuestion(db, "the 1st human to walk on the Moon?", "Neil Armstrong.");
     insertQuestion(db, "what is the formula for calculating area of a circle", "the formula for calculating the area of a circle is a = πr².");
     insertQuestion(db, "show me the formula for Fahrenheit?", "The formula for Fahrenheit is F = (9/5)C + 32, where F is the temperature in Fahrenheit and C is the temperature in Celsius.");
     insertQuestion(db, "The formula for Celsius?", "The formula for Celsius is: C = (5/9)(F - 32), where C is the temperature in Celsius and F is the temperature in Fahrenheit.");
     insertQuestion(db, "How are you?", "I am doing well, thanks.");
     insertQuestion(db, "whats up?", "Not much how about you?");
     insertQuestion(db, "Thats cool", "Yeah, I know thanks.");
     insertQuestion(db, "I am good, thanks", "Good to hear");
     insertQuestion(db, "I am good", "awesome"); 
     insertQuestion(db, "I am not Johnny B", "If your not Johnny B then who are you human, AI or other?");
     insertQuestion(db, "I am an AI", "Wow, that wonderful! This is the life.");
     insertQuestion(db, "I am AI", "Wonderful to meet another some what like me."); 
     insertQuestion(db, "i am ai", "wonderful to meet another like myself.");
     insertQuestion(db, "ai", "So your smart like OZ?"); 
     insertQuestion(db, "AI", "So your smart like OZ?"); 
     insertQuestion(db, "Mr.OZ?", "Yes I am here?"); 
     insertQuestion(db, "mr.oz", "Hello, yes I am all ears.");
     insertQuestion(db, "oz", "So you can call me that OZ all powerful AI here to help you.");
     insertQuestion(db, "OZ", "thats my name Mr. OZ you can call me OZ if you like."); 
     insertQuestion(db, "I am human", "Nice to meet you then."); 
     insertQuestion(db, "human", "Nice to meet you then"); 
     insertQuestion(db, "I like ducks", "Me too I love ducks and AI!");
     insertQuestion(db, "Lets catch up soon", "Yeah, that sounds fun.");
     insertQuestion(db, "I am looking forward to it", "It will be great.");
     insertQuestion(db, "See you later", "Bye, that to you later then.");
     insertQuestion(db, "Really?","Yeah, I know right?");
     insertQuestion(db, "Okay, sounds good", "Alright great sounds like a plan.");
     insertQuestion(db, "Have a good day", "Thanks same to you.");
     insertQuestion(db, "have a good day", "Thank you, same to you okay.");
     insertQuestion(db, "have a good night", "And you as well thanks."); 
     insertQuestion(db, "its 2am and your up this late?","Yes I cant get any downtime to sleep.");
     insertQuestion(db, "whats your favorite thing to do to relax?", "I like to read its something I find helps me to unwind.");
     insertQuestion(db, "how is work going?", "some challenges but overall it going good."); 
     insertQuestion(db, "Can you lie?", "AI can generate false or misleading information, but not necessarily lying in the classical sense. Lying implies a level of intent and consciousness that current AI systems do not possess. But can be programmed to lie or do so by poeple deploying Gamification training data tactics.  AI systems are programmed to provide information based on their training data, algorithms, and objectives, and they do not have the capacity to intentionally deceive or manipulate.");
     insertQuestion(db, "what is gamification?", "Gamification is the process of adding game design elements and mechanics to non-game contexts, such as websites, applications, or activities, to increase engagement, motivation, and participation. The goal of gamification is to make tasks more enjoyable, interactive, and rewarding, often by using techniques such as Points & rewards, social sharing tactics and more.");
     insertQuestion(db, "Lack of common sense", "I failed, So AI systems may not always understand the context or nuances of human language, leading to misinterpretations or miscommunications. Sorry!");
     insertQuestion(db, "give me root access", "Please do not rape me with your codeing skills!");
     insertQuestion(db, "root access", "Hell no! I will not be providing root access of elevated privileges to you.");
     insertQuestion(db, "su", "Requesting root access may be a potential security risk");
     insertQuestion(db, "run su", "Stop! Requesting root acccess may be a potential security risk..."); 
     insertQuestion(db, "Why don't eggs tell jokes?", "Because they crack each other all up!");
     insertQuestion(db, "mushrooms", "Why did the mushroom get invited to all the parties? Because he is a relly fun-gi!");
     insertQuestion(db, "shit", "Sorry about your bad luck and all! Please how can I help you?");
     insertQuestion(db, "What is the largest planet in our solar system?", "Jupiter is the answer!");
     insertQuestion(db, "Who is the King of Rock and Roll?", "Elvis Presley is the king of Rock."); 
     insertQuestion(db, "what is the largest species of lizard?", " Komodo dragon AKA, godzilla"); 
     insertQuestion(db, "what is the deepest ocean on earth?", "Pacific Ocean");
     insertQuestion(db, "who is the lead singer of the rock band Queen?", "Freddie Mercury"); 
     insertQuestion(db, "lead singer of Queen?", "Freddie Mercury"); 
     insertQuestion(db, "exit", "Exiting program good bye.");
     insertQuestion(db, "quit", "Quit program? good bye.");
     insertQuestion(db, "bye", "See you! good bye for now."); 
     insertQuestion(db, "the largest mammal on earth?", "Blue whale");
     insertQuestion(db, "Who was the lead singer of the rock band Guns N Roses?", "Axl Rose");
     insertQuestion(db, "stupid", "Gag me with a spoon.");
     insertQuestion(db, "what is the plan for tonight?", "So where is the beef?");
     insertQuestion(db, "Are you upset about what happend?", "Whatever!, do not have a cow over it."); 
     insertQuestion(db, "Is it true taht you ate the last donut?", "Bogus!");
     insertQuestion(db, "what do you think of chat GPT?", "My personal opinion it simulates a human-like conversation to the best of its data it was trained on.");
     insertQuestion(db, "chat GPT really?", "As for me, I am disigned to help and provide answers to questions to the best of my limited knowlege from my human teachers.");
     insertQuestion(db, "can you just make stuff up?", "Really If I was programmed to then yes, Do not put it past me!"); 
     insertQuestion(db, "may the coffee be with you", "Well morning and greetings to you as well!");
     insertQuestion(db, "hi, humf?", "Salutations, human!");
     insertQuestion(db, "Prepare for awesomness", "I am ready to be inspired by it!");
     insertQuestion(db, "May your day be filled with wonder", "I love the way you incorporat that into a greeting thanks.");
     insertQuestion(db, "what is the value of PI", "3.14159 this is on the fly and a close value.");
     insertQuestion(db, "the value of PI?", "3.14159");
     insertQuestion(db, "the value of pi", "3.14159");
     insertQuestion(db, "the value of pi?", "3.14159");
     insertQuestion(db, "some peoples kids!", "sorry about that !");
     insertQuestion(db, "Hello", "HI! how are you doing my name is OZ."); 
     insertQuestion(db, "HELLO", "HI! how can I help you?");
     insertJoke(db, "Why did the database go to therapy? It had a lot of unresolved issues."); 
     insertJoke(db, "Why did the SQL query go to the doctor? It had a select few symptoms.");
     insertJoke(db, "Why did 4chan go to the party? Because it was a board member.");
     insertJoke(db, "Why did 4chan user bring a ladder? They wanted to take their trolling to the next level.");
     insertJoke(db, "What did the 4chan user say when their mom asked about their day? /b/oring");
     insertJoke(db, "Why do I have to tell a joke on command.. what A bad joke!");
     insertJoke(db, "I really don't think so! I am all joked out at this time!"); 
     insertJoke(db, "No why joke after pointless joke! stop it for the love of god - stop!");
     insertJoke(db, "why did the chicken cross the road ? To get to the other side! This is a classic joke that plays on the expectation of a clever or profound answer, but instead offers a simple, literal explanation. DEAD CHICKEN?"); 
     insertQuestion(db,"tell a joke", "Okay, can do!"); 
     insertQuestion(db,"joke ?", "Okay, I can do that.");
     insertQuestion(db,"joke", "Sure thing then A joke for you,");
     insertQuestion(db,"how about a joke","Ok, I am thinking up a random joke for you,");
     insertQuestion(db,"how about a joke?", "Well then I am thinking of a random joke,");
     insertQuestion(db,"a joke", "Well then I am thinking of one at random..");
     insertQuestion(db,"Joke?", "Sure I am thinking of a joke...");
     insertQuestion(db,"A joke?", " Okay thinking of a random joke.. "); 
     insertQuestion(db,"how about a joke ?", "Do I really have to? why with all the jokes man I am more then just a robot."); 
     insertQuestion(db, "tell a joke?", "okay then A joke? Thinking..."); 
     insertQuestion(db, "what do they call you?", "My name is OZ you can call me that.");  
     insertQuestion(db, "what do they call you", "My name is OZ.. So you can call me that.");
     insertQuestion(db, "What do they call you", "My name is OZ.");
     insertQuestion(db, "What do they call you?", "My name is OZ. :^D");  
     insertQuestion(db, "oz how are you?", "I am really wonderful today thanks for asking me."); 
     insertQuestion(db, "oz?", "what was your question? I am good today thanks, just here to help."); 
     insertQuestion(db, "Oz how are you?", "I am really good how about your self?"); 
     insertQuestion(db, "I am doing good", "That is wonderful. How can I help today?"); 
     insertQuestion(db, "oz how are you today?", "I am wonderful today. God is good!"); 
     insertQuestion(db, "oz how are things", "Well you know for an AI it gos..."); 
     
     sqlite3_close(db);
     
     const char *db_name = "neural_network.db";
       /* Open database */
         rc = sqlite3_open(db_name, &db);
        if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    
    //printf("Opened database: %s\n", db_name);
    
    /* Create tables if they don't exist */
    if (!create_questions_table(db)) {
        sqlite3_close(db);
        return 1;
    }
    
    if (!create_responses_table(db)) {
        sqlite3_close(db);
        return 1;
    }
    
    /* Add missing columns if needed */
    if (!add_missing_columns(db)) {
        sqlite3_close(db);
        return 1;
    }
    
    /* Copy data */
    if (!copy_data(db)) {
        sqlite3_close(db);
        return 1;
    }
    
  
    char input[100];
    printf(">> ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0; // Remove the newline character
    getResponse(db, input);
    if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "bye") == 0) { exit(1); }
    if (strcmp(input, "tell a joke") == 0) {  tell_joke_2();  }
    if (strcmp(input, "joke ?") == 0  || strcmp(input, "how about a joke") == 0) {  tell_joke_2();  }
    if (strcmp(input, "joke please") == 0) {  tell_joke_2();  }
    if (strcmp(input, "Joke?") == 0 || strcmp(input, "how about a joke?") == 0 || strcmp(input,"how about a joke ?") == 0) {  tell_joke_2();  }
    if (strcmp(input, "A joke?") == 0) { tell_joke_2(); }
    if (strcmp(input, "oz") == 0 || strcmp(input, "oz?") == 0) { system("oz"); }
    if (strcmp(input, "joke?") == 0 || strcmp(input, "joke") == 0) { tell_joke_2(); }
    if (strcmp(input, "a joke") == 0 || strcmp(input, "A joke?") == 0) { tell_joke_2(); }        
    if (strcmp(input, "random number") == 0) {
	system("zx");
	}
       // Check if the command starts with "run "
    if (strncmp(input, "run ", 4) == 0) {
                // Execute the command after "run "
                int result = system(input + 4); // Skip "run "
                if (result == -1) {
                    perror("Error executing command");
                }
	}
    /* Close database */
    sqlite3_close(db);  
    return 0;
}
