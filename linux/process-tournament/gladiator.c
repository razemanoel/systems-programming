#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_FILE_NAME 10

// Function to get opponent attack from their G{i}.txt file
int get_opponent_attack(int opponent_num) {
    char filename[MAX_FILE_NAME];
    // Build file name like "G1.txt", "G2.txt", etc.
    snprintf(filename, sizeof(filename), "G%d.txt", opponent_num);
    // Open opponent's file to read their stats
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open opponent file");
        exit(1);
    }
    // Read opponent's health, attack, and their 3 opponent numbers
    int health, attack, opp1, opp2, opp3;
    if (fscanf(file, "%d, %d, %d, %d, %d", &health, &attack, &opp1, &opp2, &opp3) != 5) {
        printf("Invalid config format in %s\n", filename);
        fclose(file);
        exit(1);
    }
    fclose(file);
    return attack;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        exit(1);
    }

    // Extract gladiator number from file name e.g., G1.txt -> 1
    char file_name[MAX_FILE_NAME];
    // Find where the '.' is
    size_t copy_len = strcspn(argv[1], ".");
    // Copy just that part (e.g., "G1")
    strncpy(file_name, argv[1], copy_len);
    file_name[copy_len] = '\0';
    // Skip the 'G' and parse the number (e.g., from "G1" get 1)
    int gladiator_num;
    sscanf(file_name + 1, "%d", &gladiator_num);

    // Open the gladiator's config file to read their stats
    FILE *g_file = fopen(argv[1], "r");
    if (!g_file) {
        perror("Failed to open config file");
        exit(1);
    }
    // Read health, attack, and the 3 opponents from the config file
    int health, attack, opponents[3];
    if (fscanf(g_file, "%d, %d, %d, %d, %d", &health, &attack, &opponents[0], &opponents[1], &opponents[2]) != 5) {
        printf("Invalid config format in %s\n", argv[1]);
        fclose(g_file);
        exit(1);
    }
    fclose(g_file);

    // Prepare log file: G{i}_log.txt
    char log_name[20];
    snprintf(log_name, sizeof(log_name), "G%d_log.txt", gladiator_num);
    // Open the log file for writing (create new)
    FILE *log_file = fopen(log_name, "w");
    if (!log_file) {
        perror("Failed to create log file");
        exit(1);
    }
    // Log the start of the gladiator process, with its PID
    fprintf(log_file, "Gladiator process started. %d:\n", getpid());
    // Start the fight loop: keep going until health drops to 0 or below
    while (health > 0) {
        // Go through each of the 3 opponents in order
        for (int i = 0; i < 3; i++) {
            int opponent_attack = get_opponent_attack(opponents[i]);
            // Log the fight action
            fprintf(log_file, "Facing opponent %d... Taking %d damage\n", opponents[i], opponent_attack);
            health -= opponent_attack;
            if (health > 0) {
                // Still alive: log remaining health
                fprintf(log_file, "Are you not entertained? Remaining health: %d\n", health);
            } else {
                // Gladiator has fallen: log final health and exit loop
                fprintf(log_file, "The gladiator has fallen... Final health: %d\n", health);
                break;
            }
        }
    }
    fclose(log_file);
    return 0;
}
