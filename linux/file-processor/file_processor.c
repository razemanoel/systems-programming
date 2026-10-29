#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Maximum buffer size for reading/writing
#define MAX_RW_SIZE 1000

// Helper functions declaration
void handle_read(FILE *requests_fp, int data_fd, int results_fd);
void handle_write(FILE *requests_fp, int data_fd);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <data_file> <requests_file>\n", argv[0]);
        exit(1);
    }

    const char *data_filename = argv[1];
    const char *requests_filename = argv[2];

    // Open data.txt (read/write mode)
    int data_fd = open(data_filename, O_RDWR);
    if (data_fd < 0) {
        perror(data_filename);
        exit(1);
    }
    // Open requests.txt (read mode)
    FILE *requests_fp = fopen(requests_filename, "r");
    if (!requests_fp) {
        perror(requests_filename);
        close(data_fd);
        exit(1);
    }
    // Open (or create) read_results.txt and clear its content
    int results_fd = open("read_results.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (results_fd < 0) {
        perror("read_results.txt");
        fclose(requests_fp);
        close(data_fd);
        exit(1);
    }

    char command;
    // Process each command from requests.txt
    while (fscanf(requests_fp, " %c", &command) == 1) {
        switch (command) {
            case 'Q':
                // Quit command: stop processing
                break;

            case 'R':
                // Call the helper function to handle the 'R' command
                handle_read(requests_fp, data_fd, results_fd);
                break;

            case 'W':
                // Call the helper function to handle the 'W' command
                handle_write(requests_fp, data_fd);
                break;

            default:
                // Invalid command: skip it
                continue;
        }

        // Handle 'Q' case: break out of the while loop after processing
        if (command == 'Q') {
            break;
        }
    }

    // Close all files before exiting
    close(results_fd);
    fclose(requests_fp);
    close(data_fd);
    return 0;
}

// Helper function to handle the 'R' command
void handle_read(FILE *requests_fp, int data_fd, int results_fd) {
    // Read command: format -> R <start> <end>
    int start, end;
    if (fscanf(requests_fp, " %d %d", &start, &end) != 2) {
        return;  // Skip invalid command
    }

    // Get current size of data.txt
    off_t data_size = lseek(data_fd, 0, SEEK_END);
    // Validate offsets: skip if out of bounds or bad range
    if (start < 0 || end >= data_size || start > end) {
        return;  // Skip if invalid or out of bounds
    }
    // Number of bytes to read
    int read_len = end - start + 1;
    char buffer[MAX_RW_SIZE];
    // Move to start offset
    lseek(data_fd, start, SEEK_SET);
    // Read the requested range
    if (read(data_fd, buffer, read_len) != read_len) {
        perror("Read failed");
        return;
    }
    // Write to read_results.txt + append newline after each read
    write(results_fd, buffer, read_len);
    write(results_fd, "\n", 1);
}

// Helper function to handle the 'W' command (optimized with buffer)
void handle_write(FILE *requests_fp, int data_fd) {
    // Write command: format -> W <offset> <text>
    int offset;
    if (fscanf(requests_fp, " %d ", &offset) != 1) {
        return;  // Invalid command
    }
    // Read the rest of the line as the text to insert
    char text[MAX_RW_SIZE];
    if (fscanf(requests_fp, " %[^\n]", text) != 1) {
        return;  // Nothing to write
    }

    int write_len = strlen(text);
    if (write_len == 0) return;  // Nothing to write
    // Get current size of data.txt
    off_t data_size = lseek(data_fd, 0, SEEK_END);
    // Validate offset: allow writing at EOF but not beyond
    if (offset < 0 || offset > data_size) {
        return;  // Skip invalid offset (must allow write at EOF)
    }

    // Calculate how much data is after the insertion point (tail)
    size_t shift_size = data_size - offset;
    // Allocate buffer to hold the "tail" part of the file (everything after offset)
    char *buffer = malloc(shift_size);
    if (!buffer && shift_size > 0) {
        perror("malloc");
        return;
    }

    // Read the shift data into the buffer
    if (shift_size > 0) {
        // Move to start offset
        lseek(data_fd, offset, SEEK_SET);
        if (read(data_fd, buffer, shift_size) != shift_size) {
            perror("Read buffer");
            free(buffer);
            return;
        }
    }

    // Write the new text at the specified offset
    lseek(data_fd, offset, SEEK_SET);
    if (write(data_fd, text, write_len) != write_len) {
        perror("Write new text");
        free(buffer);
        return;
    }

    // Write the tail data back after the new text (shifting the old content forward)
    if (shift_size > 0) {
        if (write(data_fd, buffer, shift_size) != shift_size) {
            perror("Write tail");
        }
    }

    // Free the buffer to avoid memory leak
    free(buffer);
}
