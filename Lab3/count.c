#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAXNUMBER 10

int main(int argc, char **argv){
    if(argc < 4) perror("Invalid arguments.");
    else {
        char *grades_path = argv[1];
        int TAs = atoi(argv[2]);
        int min_grade = atoi(argv[3]);
        pid_t PIDs[TAs];

        FILE *fptr;
        if((fptr = fopen(grades_path, "r")) == NULL) perror("Could not open the input file");
        else {
            char buffer[MAXNUMBER];
            int students = 0;
            if(fgets(buffer, MAXNUMBER, fptr) != NULL) students = atoi(buffer);
            int grades[students], idx = 0, midterm, final;
            while(idx < students && fgets(buffer, MAXNUMBER, fptr) != NULL) {
                if(sscanf(buffer, "%d %d", &midterm, &final) > 0) {
                    grades[idx++] = midterm + final;
                }
            }

            int group = students / TAs;
            int stat_loc;
            pid_t pid;
            int results[TAs];
            for(idx = 0; idx < TAs; idx++){ // TAs
                pid = fork();
                if (pid == -1) perror("error in fork");
                else if(pid == 0) {
                    int start = idx*group;
                    int end = idx == TAs-1 ? students : start + group;
                    int passed = 0;
                    for(int i = start; i < end; i++) {
                        if(grades[i] >= min_grade) passed++;
                    }   
                    exit(passed);           
                }
                else PIDs[idx] = pid;
            }
            for(idx = 0; idx < TAs; idx++) { // Department
                pid = waitpid(PIDs[idx], &stat_loc, 0);
                results[idx] = !(stat_loc & 0x00FF) ? stat_loc>>8 : 0;                
            }
            for(idx = 0; idx < TAs; idx++) printf("%d ", results[idx]);
            printf("\n");
        }
    }
    return 0;
}