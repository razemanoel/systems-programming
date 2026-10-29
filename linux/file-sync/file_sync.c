#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_FILES 100
#define MAX_FILENAME 256
#define MAX_PATH 1024

// Validates the number of arguments passed 
void validate_arguments(int argc) {
    if (argc != 3) {
        printf("Usage: file_sync <source_directory> <destination_directory>\n");
        exit(1);
    }
}

// Prints the current working directory
void print_current_dir(char *cwd) {
    if (getcwd(cwd, MAX_PATH) == NULL) {
        perror("getcwd failed");
        exit(1);
    }
    printf("Current working directory: %s\n", cwd);
}

// Checks if the source directory exists
void check_src_dir(const char *source_dir) {
    DIR *dir = opendir(source_dir);
    if (dir == NULL) {
        printf("Error: Source directory '%s' does not exist.\n", source_dir);
        exit(1);
    }
    closedir(dir);
}

// Checks if a file is a regular file 
int is_regular_file(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) 
        return 1;
    return 0;
}

// Check the destination directory exists - creates it if not
void check_dst_dir(const char *dst_dir) {
    // Try to open dst_dir as directory
    DIR *dir = opendir(dst_dir);
    if (dir == NULL) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process creates the directory
            execl("/bin/mkdir", "mkdir", "-p", dst_dir, NULL);
            perror("mkdir failed");
            exit(1);
        } else {
            // Parent wait for child to finish, then print prompt
            wait(NULL);
            printf("Created destination directory '%s'.\n", dst_dir);
        }
    } else {
        closedir(dir);
    }
}

// Computes the absolute path of a given directory
void dir_absolute_path(const char *dir, const char *original_dir, char *absolute_path) {
    // Change to the target directory
    if (chdir(dir) != 0) {
        perror("chdir failed");
        exit(1);
    }
    // Get the absolute path of the current directory
    if (getcwd(absolute_path, MAX_PATH) == NULL) {
        perror("getcwd failed");
        exit(1);
    }
    // Return to the original working directory
    if (chdir(original_dir) != 0) {
        perror("chdir (return to original) failed");
        exit(1);
    }
}

// Prints synchronization message 
void print_sync_msg(char *src_dir, char *dst_dir, const char *original_dir) {
    char src_path[MAX_PATH], dst_path[MAX_PATH];
    // Resolve the absolute path for both directories
    dir_absolute_path(src_dir,original_dir, src_path);
    dir_absolute_path(dst_dir,original_dir, dst_path);
    // Print final message
    printf("Synchronizing from %s to %s\n", src_path, dst_path);
}

// Constructs a full file path by combining a directory and a filename.
void build_path(char *path, const char *dir, const char *file) {
    // Copy the directory path into 'path'
    strncpy(path, dir, MAX_PATH - 1);
    path[MAX_PATH - 1] = '\0';
    // If 'path' doesn't already end with '/', add it
    int len = strlen(path);
    if (len > 0 && path[len - 1] != '/') {
        strncat(path, "/", MAX_PATH - strlen(path) - 1);
    }
    // Append the file name to the directory path
    strncat(path, file, MAX_PATH - strlen(path) - 1);
}

// Allocates and returns a copy of the given filename
char *file_name(const char *name) {
    int len = strlen(name);
    if (len >= MAX_FILENAME ) {
        printf("File's name : %s too long, skipping...", name);
        return NULL;
    }
    // Allocate memory for the filename string
    char *str = malloc((len + 1) * sizeof(char));
    if (str == NULL) {
        perror("malloc failed");
        exit(1);
    }
    // Copy the name and null-terminate it
    strncpy(str, name, len);
    str[len] = '\0';
    return str;
}

// Sorts array of filenames alphabetically using bubble sort
void sort_files(char *files[], int count) {
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (strcmp(files[i], files[j]) > 0) {
                char *tmp = files[i];
                files[i] = files[j];
                files[j] = tmp;
            }
        }
    }
}

// Lists all regular files in source_dir and returns the count
int list_files(const char *source_dir, char *files[]) {
    int count = 0;
    // Try to open dst_dir as directory
    DIR *dir = opendir(source_dir);
    if (dir == NULL) {
        perror("opendir failed");
        return 0;
    }
    // Extract file from directory
    struct dirent *dir_file = readdir(dir);
    // Iterate over all directory files
    while (dir_file != NULL) {
        if (count >= MAX_FILES)
            break;
        char path[MAX_PATH];
        // Find file's path and add to list
        build_path(path, source_dir, dir_file->d_name);
        // Check if file is regular file (not directory..)
        if (is_regular_file(path)) {
            // Build str file's name and add to list
            char *name = file_name(dir_file->d_name);
            if (name != NULL) {
                files[count] = name;
                count++;
            }            
        }
        // Move to next file
        dir_file = readdir(dir);
    }
    closedir(dir);
    // Sort files alphabetically
    sort_files(files, count);
    return count;
}

// Compares two files using diff, returns 1 if different, 0 if identical
int files_diff(const char *path1, const char *path2) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Suppress output
        int null_fd = open("/dev/null", O_WRONLY);
        dup2(null_fd, STDOUT_FILENO);
        close(null_fd);
        // Child procces to diff files
        execl("/usr/bin/diff", "diff", "-q", path1, path2, NULL);
        perror("diff failed");
        exit(1);
    } else {
        // Parent waits to check result
        int status;
        wait(&status);
        return !(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
    
}

// Compares modification times of two files
int check_time(const char *path1, const char *path2) {
    struct stat stat1, stat2;
    // Stat fails - doesnt exist
    if (stat(path1, &stat1) < 0) 
        return -1;
    if (stat(path2, &stat2) < 0) 
        return 1;
    // path1 is newer
    if (stat1.st_mtime > stat2.st_mtime) 
        return 1;
    // path2 is newer
    if (stat1.st_mtime < stat2.st_mtime) 
        return -1;
    // Both have the same modification time
    return 0;
}

// Copies file from src_path to dst_path using `cp` command
void copy_file(const char *src_path, const char *dst_path) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child procces to copy the file
        execl("/bin/cp", "cp", src_path, dst_path, NULL);
        perror("cp failed");
        exit(1);
    } else {
        // Parent waits to print prompt
        wait(NULL);
        printf("Copied: %s -> %s\n", src_path, dst_path);
    }
}

// synchronization files from source to dest
void sync_files(const char *source_dir, const char *dest_dir, const char *original_dir) {
    char *files[MAX_FILES];
    int count = list_files(source_dir, files);
    // Iterate through all files from source directory
    for (int i = 0; i < count; ++i) {
        // Construct the full path and absolute path
        char src_path[MAX_PATH], dst_path[MAX_PATH];
        char src_abs_dir[MAX_PATH], dst_abs_dir[MAX_PATH];
        // Get absolute path for both directories 
        dir_absolute_path(source_dir,original_dir, src_abs_dir);
        dir_absolute_path(dest_dir,original_dir, dst_abs_dir);
        // Build the file's absolute path 
        build_path(src_path, src_abs_dir, files[i]);
        build_path(dst_path, dst_abs_dir, files[i]);
        // If file doesn't exist in destination, copy it
        if (!is_regular_file(dst_path)) {
            printf("New file found: %s\n", files[i]);
            copy_file(src_path, dst_path);
        } else {
            // If file exists, check if different
            if (files_diff(src_path, dst_path)) {
                int cmp = check_time(src_path, dst_path);
                // Source file is newer — update destination
                if (cmp > 0) {
                    printf("File %s is newer in source. Updating...\n", files[i]);
                    copy_file(src_path, dst_path);
                // Destination file is newer — do not overwrite
                } else {
                    printf("File %s is newer in destination. Skipping...\n", files[i]);
                }
            // Files are identical — no need to copy
            } else {
                printf("File %s is identical. Skipping...\n", files[i]);
            }
        }
        // Free memory for filename
        free(files[i]);
    }
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    print_current_dir(cwd);
    validate_arguments(argc);
    // Extract directories
    char *src_dir = argv[1];
    char *dst_dir = argv[2];
    // Check source and destination directories
    check_src_dir(src_dir);
    check_dst_dir(dst_dir);
    // Synchronize the files between source and destination
    print_sync_msg(src_dir, dst_dir, cwd);
    sync_files(src_dir, dst_dir, cwd);
    printf("Synchronization complete.\n");
    return 0;
}
