#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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
        printf("Not enough memory!\n");
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
            printf("Abstract: %s\n", cleaned_abstract);

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
                printf("Abstract: %s\n", cleaned_abstract);

                free(abstract);
                free(cleaned_abstract);
            } else {
                printf("No abstract found\n");
            }
        }
    } else {
        printf("No abstract found\n");
    }
}

int main() {
    CURL *curl;
    CURLcode res;

    MemoryStruct chunk;
    chunk.memory = malloc(1);  // Initial allocation
    chunk.size = 0;            // No data at this point

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        char query[256];
        printf("Enter your search query: ");
        fgets(query, sizeof(query), stdin);
        query[strcspn(query, "\n")] = 0; // Remove newline character

        char *encoded_query = curl_easy_escape(curl, query, 0); // URL-encode the query
        if (encoded_query == NULL) {
            printf("Failed to encode query\n");
            return 1;
        }

        char url[256];
        snprintf(url, sizeof(url), "https://api.duckduckgo.com/?q=%s&format=json", encoded_query);

        // Print the constructed URL for debugging
        printf("Constructed URL: %s\n", url);

        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform the request
        res = curl_easy_perform(curl);
        // Check for errors
        if (res != CURLE_OK) {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Extract and print the Abstract
            if (chunk.size > 0) {
                extract_abstract(chunk.memory);
            } else {
                printf("No data received\n");
            }
        }

        // Free the encoded query
        curl_free(encoded_query);

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Free the response data
    if (chunk.memory) {
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}
