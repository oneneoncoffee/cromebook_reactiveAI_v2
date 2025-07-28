/* Project date: 06.09.2025 Project time:01:14:19AM 
 * Project name: OX */
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

// Function to create the "jokes" table if it doesn't exist
static int createTable(sqlite3 *db) {
    const char *sql = "CREATE TABLE IF NOT EXISTS jokes (joke TEXT)";
    char *errMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    return SQLITE_OK;
}

// Function to insert a joke into the "jokes" table
static int insertJoke(sqlite3 *db, const char *joke) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO jokes (joke) VALUES (?)";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    rc = sqlite3_bind_text(stmt, 1, joke, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return SQLITE_OK;
    } else if (rc == SQLITE_ERROR) {
        fprintf(stderr, "SQL step error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    } else if (rc == SQLITE_MISUSE) {
        fprintf(stderr, "SQL step misuse error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    } else {
        fprintf(stderr, "SQL step unexpected return value: %d\n", rc);
        sqlite3_finalize(stmt);
        return rc;
    }
}

// Function to escape single quotes in a string
static char *escapeSingleQuotes(const char *str) {
    int len = strlen(str);
    char *escapedStr = malloc(len * 2 + 1);
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] == '\'') {
            escapedStr[j++] = '\'';
        }
        escapedStr[j++] = str[i];
    }
    escapedStr[j] = '\0';
    return escapedStr;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
		printf("Program %s was made to only add jokes into our database. \n", argv[0]);
		printf("you can also add other random responses to make our AI learn,\n");
		printf("and act more human in it's responses.\n");
        fprintf(stderr, "Usage: %s <text_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *databaseFile = "neural_network.db";
    const char *textFile = argv[1];

    sqlite3 *db;
    int rc = sqlite3_open(databaseFile, &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return EXIT_FAILURE;
    }

    // Create the "jokes" table if it doesn't exist
    if (createTable(db) != SQLITE_OK) {
        sqlite3_close(db);
        return EXIT_FAILURE;
    }

    // Open the text file
    FILE *file = fopen(textFile, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", textFile);
        sqlite3_close(db);
        return EXIT_FAILURE;
    }

    char line[1024];
    int lineNumber = 1;
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character at the end of the line
        line[strcspn(line, "\n")] = 0;
        printf("Inserting joke %d: %s\n", lineNumber, line);
        // Insert the joke into the database
        rc = insertJoke(db, line);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Error inserting joke %d: %s\n", lineNumber, sqlite3_errmsg(db));
            break;
        }
        lineNumber++;
    }
    fclose(file);

    sqlite3_close(db);
    return EXIT_SUCCESS;
}
