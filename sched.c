#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct task* read_file_return_list(FILE *input_file, int task_id);
struct task* create_node(int arrival_time, int service_time, int current_task_id);

void roundRobin(struct task *allTasks, int timeSlice, int overhead, FILE *fp, double *avg_resp_time, int *total_overhead);
void enqueue_tail(struct task **queueHead, struct task *taskToAdd);
struct task* dequeue(struct task **queueHead);
void print_two_queues(FILE *fp, struct task *q1, struct task *q2);
void free_list(struct task *list);

struct task{
    int
        task_id, /* specified on CL via -i x (x is any large integer)*/
        service_time,
        remaining_time,
        completion_time,
        response_time,
        wait_time,
        arrival_time,
        has_run;
        struct task *next;
};

int main (int argc, char *argv[]){

    int opt;
    int t, i, o; //CLAs
    char* output_file; //Output file to be created and written to

    //GetOPt procedure to get CLAs from user
    while((opt = getopt(argc, argv, "t:i:o:f:")) != -1)
    {
        switch(opt)
        {
            case 't': //Time Slice
                t = atoi(optarg);
                break;
            case 'i': //Starting Task ID
                i = atoi(optarg);
                break;
            case 'o': //Overhead
                o = atoi(optarg);
                break;
            case 'f': //Output file
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

    struct task* head = read_file_return_list(stdin, i); //Create linked list

    FILE *fptr = fopen(output_file, "w");
    double avg_r_time = 0;
    int t_overhead = 0;
    //roundRobin(head, t, o, fptr, &avg_r_time, &t_overhead);

    return 0;
}

/*
Summary: Reads input file and creates linked list
Input: Designated input file with information and starting Task ID given by user CLA
Output: head node of circular linked list with list correctly appended to head
*/
struct task* read_file_return_list(FILE *input_file, int task_id){
    char ch;

    int file_arrival_time, file_service_time;
    
    //For each line of text of input file, pass in data to new node of linked list
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
    /*struct task* rover = head;
    while (rover->next != NULL)
    {
        rover = rover->next;
    }
    rover->next = head; //Complete circular list*/

    fclose(input_file);

    return head;
}

//Create new node and links appropriate data
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
    
    return new_node;
}


void roundRobin(struct task *allTasks, int timeSlice, int overhead, FILE *fp, double *avg_resp_time, int *total_overhead) {
    int currentTime = 0;
    *total_overhead = 0;

    struct task *futureTasks = allTasks;
    struct task *newQ = NULL;
    struct task *rrQ = NULL;
    struct task *completedList = NULL;
    struct task *cpu = NULL;
    int sliceRemaining = 0;

    long long sum_resp_time = 0;
    int tasks_completed = 0;

    printf("Round Robin scheduling results\n\n");
    fprintf(fp, "Round Robin scheduling results\n\n");
    printf("%-5s %-6s %-12s %-12s %s\n", "time", "cpu", "serv time", "remaining", "ready queue");
    fprintf(fp, "%-5s %-6s %-12s %-12s %s\n", "time", "cpu", "serv time", "remaining", "ready queue");
    printf("---------------------------------------------------------------\n");
    fprintf(fp, "---------------------------------------------------------------\n");

    while (futureTasks != NULL || newQ != NULL || rrQ != NULL || cpu != NULL) {
        while (futureTasks != NULL && futureTasks->arrival_time == currentTime) {
            struct task *arrivedTask = futureTasks;
            futureTasks = futureTasks->next;
            arrivedTask->next = NULL;
            enqueue_tail(&newQ, arrivedTask);
        }

        if (cpu != NULL) {
            if (cpu->remaining_time == 0) {
                cpu->completion_time = currentTime;
                sum_resp_time += cpu->response_time;
                tasks_completed++;
                enqueue_tail(&completedList, cpu);
                cpu = NULL;
            } else if (sliceRemaining == 0) {
                cpu->has_run = 1;
                enqueue_tail(&rrQ, cpu);
                cpu = NULL;
            }
        }

        if (cpu == NULL) {
            if (newQ != NULL) {
                cpu = dequeue(&newQ);
                *total_overhead += overhead;
                sliceRemaining = (cpu->remaining_time < timeSlice) ? cpu->remaining_time : timeSlice;
                if (cpu->has_run == 0) {
                    cpu->response_time = currentTime - cpu->arrival_time;
                    cpu->has_run = 1;
                }
            } else if (rrQ != NULL) {
                cpu = dequeue(&rrQ);
                *total_overhead += overhead;
                sliceRemaining = (cpu->remaining_time < timeSlice) ? cpu->remaining_time : timeSlice;
            }
        }

        if (cpu != NULL) {
            printf("%-5d %-6d %-12d ", currentTime, cpu->task_id, cpu->service_time);
            fprintf(fp, "%-5d %-6d %-12d ", currentTime, cpu->task_id, cpu->service_time);
            int nextRem = cpu->remaining_time - 1;
            if (nextRem == 0) {
                printf("%-12s ", "0 (done)");
                fprintf(fp, "%-12s ", "0 (done)");
            } else {
                printf("%-12d ", nextRem);
                fprintf(fp, "%-12d ", nextRem);
            }
        } else {
            printf("%-5d %-6s %-12s %-12s ", currentTime, "", "", "");
            fprintf(fp, "%-5d %-6s %-12s %-12s ", currentTime, "", "", "");
        }

        print_two_queues(fp, newQ, rrQ);
        printf("\n");
        fprintf(fp, "\n");

        if (cpu != NULL) {
            cpu->remaining_time--;
            sliceRemaining--;
	    cpu->service_time--;
        }

        currentTime++;
    }

    printf("---------------------------------------------------------------\n");
    fprintf(fp, "---------------------------------------------------------------\n");
    printf("All tasks complete.\n");
    fprintf(fp, "All tasks complete.\n");

    if (tasks_completed > 0) *avg_resp_time = (double)sum_resp_time / (double)tasks_completed;
    else *avg_resp_time = 0.0;

    free_list(completedList);
}

void enqueue_tail(struct task **queueHead, struct task *taskToAdd) {
    taskToAdd->next = NULL;
    if (*queueHead == NULL) {
        *queueHead = taskToAdd;
    } else {
        struct task *current = *queueHead;
        while (current->next != NULL) current = current->next;
        current->next = taskToAdd;
    }
}

struct task* dequeue(struct task **queueHead) {
    if (*queueHead == NULL) return NULL;
    struct task *taskToRun = *queueHead;
    *queueHead = (*queueHead)->next;
    taskToRun->next = NULL;
    return taskToRun;
}

void print_two_queues(FILE *fp, struct task *q1, struct task *q2) {
    int first = 1;
    struct task *cur = q1;
    while (cur != NULL) {
        if (!first) { printf(", "); fprintf(fp, ", "); }
        printf("%d-%d", cur->task_id, cur->remaining_time);
        fprintf(fp, "%d-%d", cur->task_id, cur->remaining_time);
        first = 0;
        cur = cur->next;
    }
    cur = q2;
    while (cur != NULL) {
        if (!first) { printf(", "); fprintf(fp, ", "); }
        printf("%d-%d", cur->task_id, cur->remaining_time);
        fprintf(fp, "%d-%d", cur->task_id, cur->remaining_time);
        first = 0;
        cur = cur->next;
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