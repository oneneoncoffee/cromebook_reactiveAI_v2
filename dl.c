#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

void print_file_size_verbros(off_t size) {
    if (size >= 1073741824) {
        printf("%.2f GB ", size / 1073741824.0);
    } else if (size >= 1048576) {
        printf("%.2f MB ", size / 1048576.0);
    } else if (size >= 1024) {
        printf("%.2f KB ", size / 1024.0);
    } else {
        printf("%ld bytes ", size);
    }
}

void print_file_size(off_t size, char *buffer) {
    if (size >= 1073741824) {
        sprintf(buffer, "%.2f GB", size / 1073741824.0);
    } else if (size >= 1048576) {
        sprintf(buffer, "%.2f MB", size / 1048576.0);
    } else if (size >= 1024) {
        sprintf(buffer, "%.2f KB", size / 1024.0);
    } else {
        sprintf(buffer, "%ld bytes", size);
    }
}

double get_delay_input() {
char input[20];
double seconds;
while(1) {
	printf("Enter delay in seconds or type 'skip' no delay: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "skip") == 0) {
            return 0.15;
        }
        if (strcmp(input, "go") == 0) {
            return 00.09;
        }
        if (strcmp(input, "run") == 0) {
            return 00.00;
        }
        if (strlen(input) == 0) {
            printf("input cannot be empty. Please enter a number.\n");
            continue;
        }
        if (strlen(input) == 0) {
            return 0.15;
        }
         char *endptr;
         seconds = strtol(input, &endptr, 20);
         if (*endptr != '\0' || endptr == input || seconds < 0) {
         printf("Invalid input. Please enter a valid positive number.\n");
         continue;
        }
        return seconds;
}
}

void delay_(double seconds) {
usleep((unsigned int)(seconds * 1000000));
}


void calculate_size(const char *dir_path, long long *total_size) {
    struct dirent *entry;
    struct stat file_stat;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Skip the special entries "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }





        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

        if (lstat(file_path, &file_stat) == 0) {
            if (S_ISREG(file_stat.st_mode)) {
                *total_size += file_stat.st_size;
            } else if (S_ISDIR(file_stat.st_mode)) {
                calculate_size(file_path, total_size);
            }
        } else {
            perror("lstat");
        }
    }

    closedir(dp);
}


void print_size(long long size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double size_in_units = (double)size;
    int unit_index = 0;

    while (size_in_units >= 1024 && unit_index < 4) {
        size_in_units /= 1024;
        unit_index++;
    }

    printf("Total size of directory: %.2f %s\n", size_in_units, units[unit_index]);
}

void print_file_attributes(mode_t mode, char *buffer) {
    sprintf(buffer, "%c%c%c%c%c%c%c%c%c",
            (mode & S_IRUSR) ? 'r' : '-',
            (mode & S_IWUSR) ? 'w' : '-',
            (mode & S_IXUSR) ? 'x' : '-',
            (mode & S_IRGRP) ? 'r' : '-',
            (mode & S_IWGRP) ? 'w' : '-',
            (mode & S_IXGRP) ? 'x' : '-',
            (mode & S_IROTH) ? 'r' : '-',
            (mode & S_IWOTH) ? 'w' : '-',
            (mode & S_IXOTH) ? 'x' : '-');
}

void print_file_attributes_verbros(mode_t mode) {
    printf("Attributes: ");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void list_directory_verbose(const char *path) {
    double  delay = get_delay_input();
    struct dirent *entry;
    struct stat file_stat;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        printf("\033[14m Created: %s ", ctime(&file_stat.st_ctime));
        print_file_size_verbros(file_stat.st_size);
        print_file_attributes_verbros(file_stat.st_mode);
        printf("\033[02m File: \033[00m \033[34m %s \033[00m", entry->d_name);
        printf("\n");
        delay_(delay);
    }

    closedir(dp);
}

void list_directory(const char *path) {
    double delay = get_delay_input();
    struct dirent *entry;
    struct stat file_stat;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        char size_buffer[50];
        char attributes_buffer[10];
        print_file_size(file_stat.st_size, size_buffer);
        print_file_attributes(file_stat.st_mode, attributes_buffer);

        printf("\033[31mSize %-15s Attributes:%-15s Created:%-25s \033[00m \033[34mFile Name:%-30s ",
                 size_buffer,
                 attributes_buffer,
                 ctime(&file_stat.st_ctime),
                 entry->d_name);
       printf("\n");
       delay_(delay);

}

    closedir(dp);
    printf("\033[00m");
}

void print_usage() {
printf("\n\nUsage: dl [Options] [Path]\n");
printf("Options:\n");
printf("1.) -help \nShow program usage and other helpfull information.\n");
printf("2.) -verbose \nThis program will output a comprehensive listing of the specified directory, including file names, sizes, modification times, and types.\n");
printf("3.) -pathsize \nThis program calculates and shows the total size of files within a designated directory.\n\n");
}

int main(int argc, char *argv[]) {
    int verbose = 0;
    int total_pathsize = 0;
    long long total_size = 0;
    if (argv[2] == NULL) {
    list_directory("..");
    if (strcmp(argv[1], "-h") == 0  || strcmp(argv[1], "--help") == 0) {
    print_usage();
    return 0;
    } else {
    return 0;
    }
    }
    for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
    verbose = 1;
     }
    if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--pathsize") == 0) {
    total_pathsize = 1;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
    print_usage();
    return 0;
    }
    }
    if (total_pathsize) {
    calculate_size(argv[2], &total_size);
    print_size(total_size);
    return EXIT_SUCCESS;
    }
    if (verbose) {
    if (argc >=2) {
    const char *path = (argc > 1) ? argv[2] : ".";
    list_directory_verbose(path);
    return EXIT_SUCCESS;
    } else if (argc < 2) {
    const char *path = ".";
    list_directory_verbose(path);
    return EXIT_SUCCESS;
    }
    } else {
    const char *path = (argc > 1) ? argv[1] : ".";
    list_directory(path);
    }
    return 0;
}
