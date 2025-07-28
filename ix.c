
/*   Project OZ AI framework 
 *   FILE: IX internet database text serach bash tool 
 *   Programming by Johnny Buckallew Stroud 
 *   Date: 05.12.2025 00:33:42
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

size_t write_memory(void *contents, size_t size, size_t nmemb, void *userp) {
    char **readBuffer = (char **)userp;
    *readBuffer = realloc(*readBuffer, size * nmemb + 1);
    if (*readBuffer == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(*readBuffer, contents, size * nmemb);
    (*readBuffer)[size * nmemb] = '\0';
    return size * nmemb;
}

void remove_html_tags(char *str) {
    char *ptr = str;
    while (*ptr != '\0') {
        if (*ptr == '<') {
            while (*ptr != '>' && *ptr != '\0') {
                ptr++;
            }
            if (*ptr == '>') {
                *ptr = ' ';
                ptr++;
            }
        } else if (*ptr == '"') {
            while (*ptr != '"' && *ptr != '\0') {
                ptr++;
            }
            if (*ptr == '"') {
                *ptr = ' ';
                ptr++;
            }
        } else if (*ptr == '\'') {
            while (*ptr != '\'' && *ptr != '\0') {
                ptr++;
            }
            if (*ptr == '\'') {
                *ptr = ' ';
                ptr++;
            }
        } else {
            ptr++;
        }
    }
}

int check_internet_connection() {
    struct hostent *he;
    struct sockaddr_in server;
    int sockfd;

    // Try to resolve a hostname (in this case, Google's DNS server)
    he = gethostbyname("8.8.8.8");
    if (he == NULL) {
        // If we can't resolve the hostname, there's likely no internet connection
        return 0;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // If we can't create a socket, there's likely no internet connection
        return 0;
    }

    // Set up the server address
    server.sin_family = AF_INET;
    server.sin_port = htons(53); // Try to connect to the DNS port
    server.sin_addr = *((struct in_addr *)he->h_addr);

    // Try to connect to the server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        // If we can't connect to the server, there's likely no internet connection
        close(sockfd);
        return 0;
    }

    // If we were able to connect, close the socket and return 1 to indicate an active internet connection
    close(sockfd);
    return 1;
}


void remove_urls(char *str) {
    char *ptr = str;
    while (*ptr != '\0') {
        if (*ptr == 'h' && *(ptr + 1) == 't' && *(ptr + 2) == 't' && *(ptr + 3) == 'p') {
            while (*ptr != ' ' && *ptr != '\0') {
                ptr++;
            }
            if (*ptr == ' ') {
                *ptr = '\0';
            }
        } else {
            ptr++;
        }
    }
}




int main() {

    if (check_internet_connection()) {
        printf("\nInternet connection is active\n");
    CURL *curl;
    CURLcode res;
    char *readBuffer = NULL;
    char query[256];
      printf("Enter a search query: ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = 0; // remove newline character

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        char *encoded_query = curl_easy_escape(curl, query, strlen(query));
        if (encoded_query) {
            char url[256];
            sprintf(url, "https://api.duckduckgo.com/?q=%s&format=json", encoded_query);

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");

            res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
            } else {
                remove_html_tags(readBuffer);
                remove_urls(readBuffer);
                printf("%s\n", readBuffer);
            }

            curl_free(encoded_query);
            curl_easy_cleanup(curl);
        } else {
            fprintf(stderr, "Failed to encode query string\n");
        }
    }

    curl_global_cleanup();
    free(readBuffer); 
    } else {
        printf("\nNo internet connection\n");
    }
    return 0;
}
