#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct task{
    int
        task_id, /* specified on CL via -i x (x is any large integer)*/
        service_time,
        remaining_time,
        completion_time,
        response_time,
        wait_time,
        arrival_time;
        struct task *next;
};

// FUnction Prototypes
void roundRobin(struct task *allTasks, int timeSlice, int overhead, FILE *fp, double *avg_resp_time, int *total_overhead);
void enqueue(struct task **queueHead, struct task *taskToAdd);
struct task* dequeue(struct task **queueHead);
struct task* create(int arrival_time, int service_time, struct task* next);
void print_queue(FILE *fp, struct task *queue);
void free_list(struct task *list);

int main (int argc, char *argv[]){
    
    struct task* head = malloc(sizeof(struct task));

    int opt;
    int t, i, o; //CLAs
    char* input_file; //Input file user inputs in CLA

    //GetOPt procedure to get CLAs from user
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
                input_file = optarg;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
        }
    }

    char ch;

    FILE *fptr = fopen(input_file, "r");

    int file_arrival_time, file_service_time;

    while (fscanf(fptr, "%i %i", &file_arrival_time, &file_service_time) == 2) {
        struct task* rover = head;
        while (rover->next != NULL)
        {
            rover = rover->next;
        }

        create(file_arrival_time, file_service_time, rover);
        
    }
    struct task* rover = head;
    while (rover->next != NULL)
    {
        printf("Arrival Time: %i, ", rover->arrival_time);
        printf("Service Time: %i\n", rover->service_time);
        rover = rover->next;
    }
    rover->next = head; //Complete circular list

    fclose(fptr);

    return 0;
}

//Create new node and link it to linked list
struct task* create(int arrival_time, int service_time, struct task* current_node)
{
    struct task* new_node = (struct task*)malloc(sizeof(struct task));
    if(new_node == NULL)
    {
        printf("Error creating a new node.\n");
        exit(0);
    }
    
    
    new_node->service_time = service_time;
    new_node->arrival_time = arrival_time;
    new_node->remaining_time = service_time;
    
    new_node->completion_time = 0;
    new_node->response_time = 0;
    new_node->wait_time = 0;
    
    current_node->next = new_node;
    //return;
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

	printf("%-5s %-10s %-10s %-10s %s\n", "Time", "CPU", "Remaining", "Service", "Ready Queue");
        fprintf(fp, "%-5s %-10s %-10s %-10s %s\n", "Time", "CPU", "Remaining", "Service", "Ready Queue");
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
            		printf("%-10d %-10d %-10d ", cpu->task_id, cpu->remaining_time, cpu->service_time);
            		fprintf(fp, "%-10d %-10d %-10d ", cpu->task_id, cpu->remaining_time, cpu->service_time);
        	} 
		else if (overheadRemaining > 0) {
            		printf("%-10s %-10s %-10s ", "OVERHEAD", "--", "--");
            		fprintf(fp, "%-10s %-10s %-10s ", "OVERHEAD", "--", "--");
        	} 
		else {
            		printf("%-10s %-10s %-10s ", "IDLE", "--", "--");
            		fprintf(fp, "%-10s %-10s %-10s ", "IDLE", "--", "--");
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
					overheadRemaining = overhead;
					*total_overhead += overhead;
				}
			}
			else if (sliceRemaining == 0) {
				enqueue(&readyQueue, cpu);
				cpu = NULL;
				
				if (readyQueue != NULL) {
					overheadRemaining = overhead;
					*total_overhead += overhead;
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
			overheadRemaining = overhead;
			*total_overhead += overhead;
		}

		currentTime++;
	}
	printf("------------------------------------------------------------------\n");
 	fprintf(fp, "------------------------------------------------------------------\n");
	printf("All tasks complete.\n");
	fprintf(fp, "All tasks complete.\n");
    
	//print_stats(fp, completedList, *total_overhead, avg_resp_time);
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
        printf("[empty]");
        fprintf(fp, "[empty]");
        return;
    }

    struct task *current = queue;
    printf("[");
    fprintf(fp, "[");
    while (current != NULL) {
        printf("%d(%d)", current->task_id, current->remaining_time);
        fprintf(fp, "%d(%d)", current->task_id, current->remaining_time);
        if (current->next != NULL) {
            printf(", ");
            fprintf(fp, ", ");
        }
        current = current->next;
    }
    printf("]");
    fprintf(fp, "]");
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
