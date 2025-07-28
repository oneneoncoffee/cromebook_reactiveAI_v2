#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <time.h>
#define MAX_RESPONSE_LENGTH 65536
#define MAX_INPUT 256
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("OZ: Memory allocation failed.. You may have to reboot your computer?\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

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
    printf("OZ: %s\n", abstract_start);

    // Optional: Extract additional information
    char *source_start = strstr(response, "\"AbstractSource\":\"");
    if (source_start) {
        source_start += strlen("\"AbstractSource\":\"");
        char *source_end = strchr(source_start, '"');
        if (source_end) {
            *source_end = '\0';
            printf("OZ: My srouce for your Internet search is %s\n", source_start);
        }
    }
}

char* extract_clean_text(const char *response) {
    char *cleaned_text = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
    if (!cleaned_text) return NULL;
    memset(cleaned_text, 0, MAX_RESPONSE_LENGTH);

    // Find AbstractText
    char *start = strstr(response, "\"AbstractText\":\"");
    if (!start) {
        strcpy(cleaned_text, "OZ: No description available.");
        return cleaned_text;
    }

    start += strlen("\"AbstractText\":\"");
    char *end = start;
    int depth = 0;

    // Find end of text
    while (*end) {
        if (*end == '\\' && *(end + 1) == '"') end += 2;
        else if (*end == '"' && depth == 0) break;
        end++;
    }

    // Copy and clean text
    char *write = cleaned_text;
    int in_tag = 0;

    while (start < end) {
        // Skip HTML tags
        if (*start == '<') { in_tag = 1; start++; continue; }
        if (*start == '>') { in_tag = 0; start++; continue; }
        if (in_tag) { start++; continue; }

        // Handle escape sequences
        if (*start == '\\') {
            start++;
            switch (*start) {
                case 'n': case 't': case 'r': *write++ = ' '; break;
                case '"': *write++ = '\''; break;
                default: *write++ = *start; break;
            }
        } 
        // Decode HTML entities
        else if (strncmp(start, "&amp;", 5) == 0) { *write++ = '&'; start += 4; }
        else if (strncmp(start, "&lt;", 4) == 0) { *write++ = '<'; start += 3; }
        else if (strncmp(start, "&gt;", 4) == 0) { *write++ = '>'; start += 3; }
        else if (strncmp(start, "&quot;", 6) == 0) { *write++ = '"'; start += 5; }
        else if (strncmp(start, "&#39;", 5) == 0) { *write++ = '\''; start += 4; }
        else *write++ = *start;

        start++;
    }
    *write = '\0';

    // Trim whitespace
    char *trim_start = cleaned_text;
    char *trim_end = cleaned_text + strlen(cleaned_text) - 1;

    while (*trim_start && (*trim_start == ' ' || *trim_start == '\t')) trim_start++;
    while (trim_end > trim_start && (*trim_end == ' ' || *trim_end == '\t')) trim_end--;
    *(trim_end + 1) = '\0';

    return trim_start;
}

int save_search_log(const char *query, const char *response) {
    sqlite3 *db;
    int rc = sqlite3_open("neural_network.db", &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }
    
    // Extract clean text
    char *cleaned_text = extract_clean_text(response);
    
    // Prepare SQL statement
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Responses (UserInput, BotResponse) VALUES (?, ?);";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        free(cleaned_text);
        return rc;
    }
    
    // Bind parameters
    sqlite3_bind_text(stmt, 1, query, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cleaned_text ? cleaned_text : "No description", -1, SQLITE_STATIC);
    
    // Execute statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }
    
    // Cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(cleaned_text);
    
    return rc;
}

// Function to create database and table if not exists
int create_database() {
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("neural_network.db", &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }
    
    // SQL to create table
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS Responses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "UserInput TEXT,"
        "BotResponse TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return rc;
    }
    
    sqlite3_close(db);
    return SQLITE_OK;
}


int main() {
    // Initialize database
    if (create_database() != SQLITE_OK) {
        fprintf(stderr, "OZ: I failed to initialize database\n");
        return 1;
    }
    if (is_internet_connected()) {
    CURL *curl;
    CURLcode res;
    char query[256];
    char encoded_query[768];
    char response_buffer[MAX_RESPONSE_LENGTH] = {0};

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "OZ: Web search CURL initialization failed\n");
        return 1;
    }

    // Prompt for search query
    printf("Enter your search query: ");
    if (fgets(query, sizeof(query), stdin) == NULL) {
        fprintf(stderr, "Input error\n");
        curl_easy_cleanup(curl);
        return 1;
    }

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
        // Copy response to a buffer for logging
        strncpy(response_buffer, chunk.memory, sizeof(response_buffer) - 1);
        
        // Extract and print result
        extract_search_result(chunk.memory);
        
        // Save search log to database
        save_search_log(query, response_buffer);
    }

    // Cleanup
    free(chunk.memory);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    } else {
	// Okay lets respond with somehting form our database  
	 sqlite3 *db;
    char *errMsg = 0;
    int rc;
    char input[MAX_INPUT];
    char *sql;
    sqlite3_stmt *stmt;

    // Open database
    rc = sqlite3_open("neural_network.db", &db);
    if (rc) {
        fprintf(stderr, "OZ: Can't open database %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Create table if not exists
    sql = "CREATE TABLE IF NOT EXISTS Responses ("
          "UserInput TEXT, "
          "BotResponse TEXT, "
          "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "OZ: I gat A SQL error %s\n", errMsg);
        sqlite3_free(errMsg);
    }

        // Get user input
        printf(">> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            
        }
        // Remove newline
        input[strcspn(input, "\n")] = 0;

        // Prepare SQL statement to find response
        sql = "SELECT BotResponse FROM Responses WHERE UserInput = ? LIMIT 1;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "OZ: I failed to prepare statement\n");
        }

        // Bind input parameter
        sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC);

        // Execute query
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_ROW) {
            // Found a response
            const unsigned char *response = sqlite3_column_text(stmt, 0);
            printf("OZ: %s\n", response);

            // Insert conversation into database
            sql = "INSERT INTO Responses (UserInput, BotResponse) VALUES (?, ?);";
            sqlite3_stmt *insert_stmt;
            rc = sqlite3_prepare_v2(db, sql, -1, &insert_stmt, 0);
            
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(insert_stmt, 1, input, -1, SQLITE_STATIC);
                sqlite3_bind_text(insert_stmt, 2, (const char*)response, -1, SQLITE_STATIC);
                sqlite3_step(insert_stmt);
                sqlite3_finalize(insert_stmt);
            }
        } else {
                // Seed random number generator
    srand(time(NULL));

    // Define responses array
    const char* database_responses[] = {
        "Sorry, the record appears to be missing 404.",
        "Unfortunately, no matching entry was located.",
        "Database search returned zero results. Epic fail!",
        "The requested information could not be retrieved.",
        "No data found matching your query parameters.",
        "Search unsuccessful. Item not present in records.",
        "System unable to locate the specified information.",
        "Record not found in current database configuration.",
        "Negative search result. Information unavailable.",
        "Information unavailable at this time.",
        "Query returned empty set or error maybe? Data not discovered.",
        "Unable to identify or missing information. Maybe it was a typeo?",
        "Search parameters produced no corresponding records.",
        "Sorry I failed to produce results.",
        "No matching information exists.",
        "Requested data appears to be absent from my records.",
        "System reports zero matches for your search.",
        "Information retrieval unsuccessful. Not good!",
        "No corresponding entry detected in my memory.",
        "Search criteria yielded no results. Ugh!",
        "Database query returned null response."
    };

        // Number of responses
        int num_responses = sizeof(database_responses) / sizeof(database_responses[0]);
        // Select a random response
        int random_index = rand() % num_responses;
        // Print the randomly selected response
        printf("OZ: %s\n", database_responses[random_index]);        
        }

        // Finalize statement
        sqlite3_finalize(stmt);   
    // Close database
    sqlite3_close(db);	 
	}
    return 0;
}
