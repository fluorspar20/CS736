#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/wait.h>

struct WorkloadEntry {
    int id; int para; char func_name[64]; int time_interval; int func_class;
};

int main() {
    FILE *file = fopen("workload.txt", "r");
    if (file == NULL) { perror("Error"); return 1; }
    struct WorkloadEntry e;
    int idx = 0;

    while (fscanf(file, "%d %d %s %d %d", &e.id, &e.para, e.func_name, &e.time_interval, &e.func_class) == 5) {
        
        // Filter: We only want the specific conflict scenario
        // Class 1 (Victim) vs Class 3 (Bully)
        if (e.func_class != 1 && e.func_class != 3) {
            continue; // Skip others
        }

        pid_t pid = fork();
        if (pid == 0) { 
            char filename[256];
            
            if (e.func_class == 1) strcpy(filename, "./io_misclass");
            else strcpy(filename, "./cpu_hog");

            char p_str[32], i_str[32], c_str[32];
            snprintf(p_str, 32, "%d", e.para);
            snprintf(i_str, 32, "%d", e.id);
            snprintf(c_str, 32, "%d", e.func_class);
            
            char *const args[] = {filename, i_str, p_str, c_str, NULL};
            execve(filename, args, NULL);
            exit(1);
        } else { 
            usleep(e.time_interval * 1000);
        }
        idx++;
        if(idx >= 200) break;
    }
    fclose(file);
    while (wait(NULL) > 0);
    return 0;
}
