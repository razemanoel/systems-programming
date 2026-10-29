#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_GLADIATORS 4

int main() {
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_files[NUM_GLADIATORS] = {"G1", "G2", "G3", "G4"};
    pid_t pids[NUM_GLADIATORS];

    // Fork one process for each gladiator
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process: this becomes a gladiator
            char file_name[10];
            // Build the file name like "G1.txt", "G2.txt", etc.
            snprintf(file_name, sizeof(file_name), "%s.txt", gladiator_files[i]);
            // Replace the current process with the gladiator executable
            char *args[] = {"./gladiator", file_name, NULL};
            execv(args[0], args);
            perror("Exec failed");
            exit(1);
        } else {
            // Parent process
            pids[i] = pid;
        }
    }

    // Wait for all gladiators to finish, tracking the last PID to finish
    pid_t last_pid = 0;
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        int status;
        // Wait for one child to finish and get its PID
        last_pid = wait(&status);
    }
    // Find out which gladiator had the last PID (the winner)
    char* winner_name;
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        if (pids[i] == last_pid) {
            winner_name = gladiator_names[i];
            break;
        }
    }

    printf("The gods have spoken, the winner of the tournament is %s!\n", winner_name);
    return 0;
}
