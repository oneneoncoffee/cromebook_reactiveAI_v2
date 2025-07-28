// to make this project with gcc at the command line type:
// gcc al.c -o al -lcurl 
// GPT4ALL Project - An open source client for the cromebook made to work with a PC / Linux distro with the local host being our server. 
// GPT-4 All is an open-source project that aims to provide a version of the GPT-4 language model that can be run locally on personal 
// computers or servers. The project is designed to make advanced AI language models more accessible to developers, researchers, 
// and enthusiasts by allowing them to use the model without relying on cloud-based services.
// This kills the paywall around open AI's data service. It can be setup on a orange pi PC / Orange pi zero board. for example and old pc.
// default config file is .gpt4allkeys
//
// EXAMPLE: 
//# Configuration for GPT-4 All Client
//API_URL=http://localhost:5000/api/gpt4
//TIMEOUT=30
//MAX_TOKENS=150
//
//Installing GPT-4 All on a Debian-based system involves several steps, including setting up the necessary dependencies, 
//downloading the model, and running it. Below is a step-by-step guide to help you through the installation process.
//
//Step 1: Update Your System
//Before starting, it's a good idea to update your package list and upgrade any existing packages:
//
//bash
//
//Copy Code
//sudo apt update
//sudo apt upgrade -y
//Step 2: Install Required Dependencies
//You will need to install several dependencies, including Python, Git, and any necessary libraries. Run the following command:
//
//bash
//
//Copy Code
//sudo apt install python3 python3-pip git -y
//Step 3: Clone the GPT-4 All Repository
//Next, you need to clone the GPT-4 All repository from GitHub. 
//Replace <repository-url> with the actual URL of the GPT-4 All repository (if available):
//
//bash
//
//Copy Code
//git clone <repository-url>
//cd gpt4all
//Step 4: Set Up a Virtual Environment (Optional but Recommended)
//It's a good practice to use a virtual environment to manage your Python dependencies. You can create and activate a virtual environment as follows:
//
//bash
//
//Copy Code
//sudo apt install python3-venv -y
//python3 -m venv venv
//source venv/bin/activate
//Step 5: Install Python Dependencies
//Once you are in the project directory and have activated your virtual environment, install the required Python packages. This is usually done via a requirements.txt file provided in the repository:
//
//bash
//
//Copy Code
//pip install -r requirements.txt
//Step 6: Download the Model
//Depending on the repository, there may be instructions for downloading the model files. 
//This could involve running a script or manually downloading files. Check the repository's README or documentation for specific instructions.
//
//For example, if there is a script to download the model, you might run:
//
//bash
//
//Copy Code
//python download_model.py
//Step 7: Run the Model
//After downloading the model and installing the dependencies, you can run the model. 
//Again, refer to the repository's documentation for the exact command, but it might look something like this:
//
//bash
//
//Copy Code
//python run_gpt4all.py
//Step 8: Access the Model
//Depending on how the model is set up, you may be able to access it via a command line interface or a web interface. Follow the instructions provided in the repository to interact with the model.
//
//Step 9: (Optional) Set Up a Configuration File
//If the model requires a configuration file, create one as per the instructions in the repository. This file may include settings like API endpoints, model parameters, etc.
//
//Step 10: Test the Installation
//Finally, test the installation by sending a request to the model and checking if it responds correctly.
//
//Conclusion
//This guide provides a general overview of how to install GPT-4 All on a Debian-based system. 
//The specific steps may vary depending on the exact implementation and repository you are using, 
//so always refer to the official documentation for the most accurate and detailed instructions. 
//If you encounter any issues, check the repository's issues page or community forums for help.
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_INPUT_SIZE 1024
#define MAX_CONFIG_SIZE 256

typedef struct {
    char api_url[MAX_CONFIG_SIZE];
} Config;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    strcat(userp, contents);
    return size * nmemb;
}

void load_config(Config *config, const char *config_file) {
    FILE *file = fopen(config_file, "r");
    if (file) {
        fgets(config->api_url, MAX_CONFIG_SIZE, file);
        // Remove newline character
        config->api_url[strcspn(config->api_url, "\n")] = 0;
        fclose(file);
    } else {
        fprintf(stderr, "Could not open config file: %s\n", config_file);
        exit(EXIT_FAILURE);
    }
}

void send_request(const Config *config, const char *input) {
    CURL *curl;
    CURLcode res;
    char response[4096] = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, config->api_url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL); // Set headers if needed

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response: %s\n", response);
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <config_file> <input_text>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Config config;
    load_config(&config, argv[1]);
    send_request(&config, argv[2]);

    return EXIT_SUCCESS;
}
