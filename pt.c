// to make this source code at the command line type,
// gcc pt.c -o pt -lcurl
// if you don't have libcurl installed on linux/cromebook,
// at the command line type in,
// sudo apt update && sudo apt install libcurl4-openssl-dev && curl --version
//
// I recomend open soruce GPT4all not open AI.... open AI is a pay walled service and not public.
// this will work with your payed data service and docker.. good luck

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_LINE_LENGTH 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_LINE_LENGTH 256

// Structure to hold the response data
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function to write the response data
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, struct MemoryStruct *userp) {
    size_t realsize = size * nmemb;
    userp->memory = realloc(userp->memory, userp->size + realsize + 1);
    if (userp->memory == NULL) {
        printf("Not enough memory!\n");
        return 0;
    }
    memcpy(&(userp->memory[userp->size]), contents, realsize);
    userp->size += realsize;
    userp->memory[userp->size] = 0;
    return realsize;
}

// Function to read configuration from a file
int read_config(const char *filename, char *api_key, char *model) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open config file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "API_KEY=", 8) == 0) {
            strcpy(api_key, line + 8);
            api_key[strcspn(api_key, "\n")] = 0; // Remove newline
        } else if (strncmp(line, "MODEL=", 6) == 0) {
            strcpy(model, line + 6);
            model[strcspn(model, "\n")] = 0; // Remove newline
        }
    }

    fclose(file);
    return 0;
}

// Function to send a request to the OpenAI API
void chat_with_gpt(const char *api_key, const char *model, const char *user_input) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk = {0};

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        // Set the URL for the API endpoint
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");

        // Set the headers
        struct curl_slist *headers = NULL;
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
        headers = curl_slist_append(headers, auth_header);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Prepare the JSON data
        char json_data[512];
        snprintf(json_data, sizeof(json_data),
                 "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}",
                 model, user_input);

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Print the response (you may want to parse this JSON to extract the message)
            printf("Response: %s\n", chunk.memory);
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }
    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file> <user_input>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *config_file = argv[1];
    char api_key[256] = {0};
    char model[256] = {0};

    // Read configuration from the specified file
    if (read_config(config_file, api_key, model) != 0) {
        return EXIT_FAILURE;
    }

    // Check if the model is provided
    if (strlen(model) == 0) {
        fprintf(stderr, "Model parameter is missing in the configuration file.\n");
        return EXIT_FAILURE;
    }

    // Check if user input is provided
    if (argc < 3) {
        fprintf(stderr, "Please provide user input as the second argument.\n");
        return EXIT_FAILURE;
    }

    // Concatenate user input from command line arguments
    char user_input[512] = {0};
    for (int i = 2; i < argc; i++) {
        strcat(user_input, argv[i]);
        if (i < argc - 1) {
            strcat(user_input, " "); // Add space between words
        }
    }

    // Call the function to chat with GPT
    chat_with_gpt(api_key, model, user_input);

    return EXIT_SUCCESS;
}
