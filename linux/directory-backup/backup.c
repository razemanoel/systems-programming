#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAX_PATH 1024

// Create a hard link for a regular file
void create_hard_link(const char *src, const char *dst) {
    // Use link() to create a hard link pointing from dst to src
    if (link(src, dst) == -1) {
        perror("link");
    }
}

// Copy a symbolic link
void copy_symlink(const char *src, const char *dst) {
    char link[MAX_PATH];
    // Read the symlink target into 'link'
    ssize_t len = readlink(src, link, sizeof(link) - 1);
    if (len == -1) {
        perror("readlink");
        return;
    }
    link[len] = '\0';
    // Create a new symlink at dst with the same target
    if (symlink(link, dst) == -1) {
        perror("symlink");
    }
}

// Create a new directory with the same permissions as the source
void copy_directory(const char *src, const char *dst) {
    struct stat st;
    // Get permissions of the source directory
    if (stat(src, &st) == -1) {
        perror("stat");
        return;
    }
    // Create a new directory with the same permission bits
    if (mkdir(dst, st.st_mode & 0777) == -1) {
        perror("mkdir");
    }
}

// Recursively copy the source directory to the destination
void backup(const char *src_dir, const char *dst_dir) {
    DIR *dir = opendir(src_dir);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip the current directory and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // Build full paths for source and destination
        char src_path[MAX_PATH];
        char dst_path[MAX_PATH];
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, entry->d_name);

        // Get information about the entry (follow symlinks if any)
        struct stat st;
        if (lstat(src_path, &st) == -1) {
            perror("lstat");
            return;
        }

        if (S_ISREG(st.st_mode)) {
            // Regular file: create a hard link
            create_hard_link(src_path, dst_path);
        } else if (S_ISLNK(st.st_mode)) {
            // Symbolic link: replicate the symlink
            copy_symlink(src_path, dst_path);
        } else if (S_ISDIR(st.st_mode)) {
            // Directory: create the same directory and recurse
            copy_directory(src_path, dst_path);
            // Recursive call
            backup(src_path, dst_path);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <backup_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *src_dir = argv[1];
    const char *backup_dir = argv[2];
    // Check if source directory exists
    struct stat st;
    if (stat(src_dir, &st) == -1) {
        perror("src dir");
        exit(EXIT_FAILURE);
    }
    // Check if backup directory already exists
    if (stat(backup_dir, &st) == 0) {
        perror("backup dir");
        exit(EXIT_FAILURE);
    }
    // Create the root backup directory
    copy_directory(src_dir, backup_dir);
    // Start the recursive backup
    backup(src_dir, backup_dir);

    return 0;
}
