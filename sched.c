#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct task* read_file_return_list(FILE *input_file, int task_id);
struct task* create_node(int arrival_time, int service_time, int current_task_id);

void roundRobin(struct task *allTasks, int timeSlice, int overhead, FILE *fp, double *avg_resp_time, int *total_overhead);
void enqueue(struct task **queueHead, struct task *taskToAdd);
struct task* dequeue(struct task **queueHead);
void print_queue(FILE *fp, struct task *queue);
void free_list(struct task *list);

struct task{
    int
        task_id, 
        service_time,
        remaining_time,
        completion_time,
        response_time,
        wait_time,
        arrival_time;
        struct task *next;
};

int main (int argc, char *argv[]){

    int opt;
    int t, i, o; 
    char* output_file; 

    while((opt = getopt(argc, argv, "t:i:o:f:")) != -1)
    {
        switch(opt)
        {
            case 't': 
                t = atoi(optarg);
                break;
            case 'i': 
                i = atoi(optarg);
                break;
            case 'o': 
                o = atoi(optarg);
                break;
            case 'f': 
                output_file = optarg;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
        }
    }

    struct task* head = read_file_return_list(stdin, i); 

    FILE *fptr = fopen(output_file, "w");
    double avg_r_time = 0;
    int t_overhead = 0;
    roundRobin(head, t, o, fptr, &avg_r_time, &t_overhead);

    fclose(fptr);
    return 0;
}


struct task* read_file_return_list(FILE *input_file, int task_id){
    char ch;

    int file_arrival_time, file_service_time;
    
    int j = 0;
    struct task* head;
    while (fscanf(input_file, "%i %i", &file_arrival_time, &file_service_time) == 2) {
        if (j == 0){
            j++;
            head = create_node(file_arrival_time, file_service_time, task_id);
        }
        else {
            struct task* rover = head;
            while (rover->next != NULL)
            {
                rover = rover->next;
            }
            rover->next = create_node(file_arrival_time, file_service_time, task_id);
        }
        task_id++;
        
    }

    return head;
}


struct task* create_node(int arrival_time, int service_time, int current_task_id)
{
    struct task* new_node = (struct task*)malloc(sizeof(struct task));
    
    new_node->service_time = service_time;
    new_node->arrival_time = arrival_time;
    new_node->remaining_time = service_time;
    
    new_node->completion_time = 0;
    new_node->response_time = 0;
    new_node->wait_time = 0;
    new_node->task_id = current_task_id;
    
    new_node->next = NULL; 
    
    return new_node;
}

void roundRobin(struct task *allTasks, int timeSlice, int overhead, FILE *fp, double *avg_resp_time, int *total_overhead) {
    int currentTime = 0;
    *total_overhead = 0;
    
    struct task *futureTasks = allTasks;
    struct task *readyQueue = NULL;
    struct task *completedList = NULL;
    struct task *cpu = NULL;

    int sliceRemaining = 0;
    int overheadRemaining = 0;

    printf("%-5s %-10s %-10s %-10s %s\n", "Time", "CPU", "serv", "remaining", "ready");
    fprintf(fp, "%-5s %-10s %-10s %-10s %s\n", "Time", "CPU", "serv", "remaining", "queue");
    printf("------------------------------------------------------------------\n");
    fprintf(fp, "------------------------------------------------------------------\n");

    while (futureTasks != NULL || readyQueue != NULL || cpu != NULL || overheadRemaining > 0) {
        
        while (futureTasks != NULL && futureTasks->arrival_time == currentTime) {
            struct task *arrivedTask = futureTasks;
            futureTasks = futureTasks->next;
            arrivedTask->next = NULL;
            enqueue(&readyQueue, arrivedTask);
        }
        
        printf("%-5d ", currentTime);
        fprintf(fp, "%-5d ", currentTime);
        
        if (cpu != NULL) {
            printf("%-10d %-10d ", cpu->task_id, cpu->service_time);
            fprintf(fp, "%-10d %-10d ", cpu->task_id, cpu->service_time);
            
            if (cpu->remaining_time - 1 == 0) {
                printf("%-10s ", "0 (done)");
                fprintf(fp, "%-10s ", "0 (done)");
            } else {
                printf("%-10d ", cpu->remaining_time - 1);
                fprintf(fp, "%-10d ", cpu->remaining_time - 1);
            }
        } else {
            printf("%-10d %-10d %-10s ", 0, 0, "");
            fprintf(fp, "%-10d %-10d %-10s ", 0, 0, "");
        }

        print_queue(fp, readyQueue);
        printf("\n");
        fprintf(fp, "\n");
        
        if (cpu != NULL) {
            cpu->remaining_time--;
            sliceRemaining--;
            
            if (cpu->remaining_time == 0) {
                cpu->completion_time = currentTime + 1;
                enqueue(&completedList, cpu);
                cpu = NULL;

                if (readyQueue != NULL) {
                    if (overhead == 0) {
                        cpu = dequeue(&readyQueue);
                        sliceRemaining = (cpu->remaining_time < timeSlice) ? cpu->remaining_time : timeSlice;
                        if (cpu->remaining_time == cpu->service_time) {
                            cpu->response_time = (currentTime + 1) - cpu->arrival_time;
                        }
                    } else {
                        overheadRemaining = overhead;
                        *total_overhead += overhead;
                    }
                }
            }
            else if (sliceRemaining == 0) {
                enqueue(&readyQueue, cpu);
                cpu = NULL;
                
                if (readyQueue != NULL) {
                    if (overhead == 0) {
                        cpu = dequeue(&readyQueue);
                        sliceRemaining = (cpu->remaining_time < timeSlice) ? cpu->remaining_time : timeSlice;
                        if (cpu->remaining_time == cpu->service_time) {
                            cpu->response_time = (currentTime + 1) - cpu->arrival_time;
                        }
                    } else {
                        overheadRemaining = overhead;
                        *total_overhead += overhead;
                    }
                }
            }
        }
        else if (overheadRemaining > 0) {
            overheadRemaining--;

            if (overheadRemaining == 0) {
                cpu = dequeue(&readyQueue);

                sliceRemaining = (cpu->remaining_time < timeSlice) ? cpu->remaining_time : timeSlice;

                if (cpu->remaining_time == cpu->service_time) {
                    cpu->response_time = (currentTime + 1) - cpu->arrival_time;
                }
            }
        }
        else if (readyQueue != NULL) {
            if (overhead == 0) {
                cpu = dequeue(&readyQueue);
                sliceRemaining = (cpu->remaining_time < timeSlice) ? cpu->remaining_time : timeSlice;
                if (cpu->remaining_time == cpu->service_time) {
                    cpu->response_time = (currentTime + 1) - cpu->arrival_time;
                }
            } else {
                overheadRemaining = overhead;
                *total_overhead += overhead;
            }
        }

        currentTime++;
    }
    printf("------------------------------------------------------------------\n");
    fprintf(fp, "------------------------------------------------------------------\n");
    printf("All tasks complete.\n");
    fprintf(fp, "All tasks complete.\n");
    
    free_list(completedList);
}

void enqueue(struct task **queueHead, struct task *taskToAdd) {
    taskToAdd->next = NULL; 

    if (*queueHead == NULL) {
        *queueHead = taskToAdd;
    } else {
        struct task *current = *queueHead;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = taskToAdd;
    }
}

struct task* dequeue(struct task **queueHead) {
    if (*queueHead == NULL) {
        return NULL;
    }
    
    struct task *taskToRun = *queueHead;
    *queueHead = (*queueHead)->next;
    
    taskToRun->next = NULL; 
    return taskToRun;
}

void print_queue(FILE *fp, struct task *queue) {
    if (queue == NULL) {
        return;
    }

    struct task *current = queue;
    while (current != NULL) {
        printf("%d-%d", current->task_id, current->remaining_time);
        fprintf(fp, "%d-%d", current->task_id, current->remaining_time);
        if (current->next != NULL) {
            printf(", ");
            fprintf(fp, ", ");
        }
        current = current->next;
    }
}

void free_list(struct task *list) {
    struct task *current = list;
    struct task *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}
