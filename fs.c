#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/statfs.h>
#include <string.h>
#include <unistd.h>

void print_size(unsigned long long size) {
    const char *units[] = {"Bytes", "KB", "MB", "GB", "TB"};
    unsigned long long sizes[5];
    sizes[0] = size;
    for (int i = 1; i < 5; i++) {
        sizes[i] = sizes[i - 1] / 1024;
    }

    for (int i = 4; i >= 0; i--) {
        if (sizes[i] > 0) {
            printf("%llu %s\n", sizes[i], units[i]);
            break;
        }
    }
}
void formatBytes(unsigned long bytes, char *buffer, size_t size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = (double)bytes;
    int unitIndex = 0;

    while (value >= 1024 && unitIndex < 4) {
        value /= 1024;
        unitIndex++;
    }

    snprintf(buffer, size, "%.2f %s", value, units[unitIndex]);
}

const char* getFilesystemType(unsigned long magic) {
    switch (magic) {
        case 0xEF53: return "ext2/ext3/ext4";
        case 0x58465342: return "XFS";
        case 0x9123683E: return "Btrfs (Virtualization)";
        case 0x52534543: return "FAT32 (VFAT)";
        case 0x5346544E: return "NTFS";
        case 0x52654973: return "ReiserFS";
        case 0x3153464A: return "JFS";
        case 0x73717368: return "SquashFS";
        case 0x01021994: return "tmpfs";
        default: return "unknown";
    }
}

void displayDriveInfo(const char *path) {
    struct statvfs stat;
    struct statfs fs_info;
    if (statvfs(path, &stat) != 0) {
        perror("statvfs");
        return;
    }
    if (statfs(path, &fs_info) != 0) {
        perror("statfs");
        return;
    }
    unsigned long totalSpace = stat.f_blocks * stat.f_frsize;
    unsigned long freeSpace = stat.f_bfree * stat.f_frsize;
    char totalSpaceStr[20];
    char freeSpaceStr[20];
    formatBytes(totalSpace, totalSpaceStr, sizeof(totalSpaceStr));
    formatBytes(freeSpace, freeSpaceStr, sizeof(freeSpaceStr));
    printf("Total space %s / ", totalSpaceStr);
    printf("Free space %s\n", freeSpaceStr);
    printf("Block size %lu bytes\n", stat.f_frsize);
    printf("Total blocks %lu\n", stat.f_blocks);
    printf("Free blocks %lu\n", stat.f_bfree);
    printf("Filesystem type %s ", fs_info.f_type == 0xEF53 ? "ext4" : "Not ext2, ext3, ext4");
    printf("%s ", fs_info.f_type == 0x52534543 ? "vfat": ",not vfat");
    printf("%s \n", fs_info.f_type == 0x5346544E ? "ntfs": ",not ntfs");
    printf("Filesystem check %s\n", getFilesystemType(fs_info.f_type));
    printf("Mount options: %s\n", (fs_info.f_flags & ST_RDONLY) ? "read-only" : "read-write");
}


int main(int argc, char *argv[]) {
    struct statvfs stat;
    if (argc != 2) {
    struct statvfs stat;
    if (statvfs("/home", &stat) != 0) {
      perror("statvfs");
      return EXIT_FAILURE;
      }
    } else {
    struct statvfs stat;
    if (statvfs(argv[1], &stat) != 0) {
        perror("statvfs");
        return EXIT_FAILURE;
    }
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long free = stat.f_bfree * stat.f_frsize;
    unsigned long long used = total - free;
    if (argc != 2) {
    printf("Device Path: /home\n");
    } else {
    printf("Device Path %s\n", argv[1]);
    }
    printf("Total Drive Size ");
    print_size(total);
    printf("Free Space ");
    print_size(free);
    printf("Used Space ");
    print_size(used);
    if (total > 0) {
        double percentage_free = (double)free / total * 100;
        double percentage_used = (double)used / total * 100;
        printf("Percentage Free: %.2f%%", percentage_free);
        printf(" Used: %.2f%%\n", percentage_used);
    } else {
        printf("\n");
    }
    }
    if (argc != 2) {
    const char *mountPoints[] = {"/"};
    int numMountPoints = sizeof(mountPoints) / sizeof(mountPoints[0]);
    for (int i = 0; i < numMountPoints; i++) {
        displayDriveInfo(mountPoints[i]);
    }
    }
    return EXIT_SUCCESS;
}
