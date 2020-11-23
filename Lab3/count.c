#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXNUMBER 10

int main(int argc, char **argv){
    if(argc < 4) perror("Invalid arguments.");
    else {
        char *grades_path = argv[1];
        int TAs = atoi(argv[2]);
        int min_grade = atoi(argv[3]);

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
            int pid, stat_loc;
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
            }
            for(idx = 0; idx < TAs; idx++) { // Department
                pid = wait(&stat_loc);
                results[idx] = !(stat_loc & 0x00FF) ? stat_loc>>8 : 0;                
            }
            for(idx = 0; idx < TAs; idx++) printf("%d ", results[idx]);
            printf("\n");
        }
    }
    return 0;
}