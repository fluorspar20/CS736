#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/wait.h>

struct WorkloadEntry {
    int id;
    int para;
    char func_name[64];
    int time_interval;
    int func_class;
};

int main() {
    FILE *file = fopen("workload.txt", "r");
    if (file == NULL) { perror("Error opening file"); return EXIT_FAILURE; }

    struct WorkloadEntry entry;
    int idx = 0;
    while (fscanf(file, "%d %d %s %d %d", &entry.id, &entry.para, entry.func_name, &entry.time_interval, &entry.func_class) == 5) {
        
        pid_t child_pid = fork();
        if (child_pid == 0) { 
            // --- MIXED WORKLOAD LOGIC ---
            char filename[256];
            
            // If Short Task (Class 1 or 2), launch Deceptive I/O
            if (entry.func_class <= 2) {
                strcpy(filename, "./io_deceptive");
            } 
            // If Long Task (Class 3, 4, 5), launch Productive CPU
            else {
                strcpy(filename, "./cpu_task");
            }
            // ----------------------------

            char para_str[32]; snprintf(para_str, sizeof(para_str), "%d", entry.para);
            char id_str[32]; snprintf(id_str, sizeof(id_str), "%d", entry.id);
            char func_str[32]; snprintf(func_str, sizeof(func_str), "%d", entry.func_class);
            char *const cmdline[] = {filename, id_str, para_str, func_str, NULL}; 
            char *const envp[] = {"VAR=value", NULL}; 

            execve(filename, cmdline, envp);
            perror("Error calling execve");
            exit(EXIT_FAILURE);
        } else { 
            usleep(entry.time_interval * 1000);
        }
        idx++;
        if(idx >= 200) break; // Limit to 200 for faster testing
    }
    fclose(file);
    while (wait(NULL) > 0);
    return EXIT_SUCCESS;
}
