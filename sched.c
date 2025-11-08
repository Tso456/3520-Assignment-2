#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct task* read_file_return_list(FILE *input_file, int task_id);
struct task* create_node(int arrival_time, int service_time, int current_task_id);

void fairmix(struct task *list, char* current_output_file, int time_slice, int overhead);
struct task *duplicate_list(struct task *list);
int reverse_max_min(int max_min);
int is_in_ready_queue(struct task *queue, struct task *node);
struct task *append_to_ready_queue(struct task *list, struct task *queue, struct task *node);
struct task *sort_return_queue(struct task *queue, int max_min);
struct task *swap_nodes(struct task *list, struct task *node1, struct task *node2);


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

    struct task *head = read_file_return_list(stdin, i); //Create linked list
    //struct task *round_robin_list = duplicate_list(head);
    struct task *fairmix_list = duplicate_list(head);

    //FILE *fptr = fopen(output_file, "w");
    double avg_r_time = 0;
    int t_overhead = 0;
    //roundRobin(round_robin_list, t, o, fptr, &avg_r_time, &t_overhead);
    fairmix(fairmix_list, output_file, t, o);
    

    struct task *trailing = NULL;
    struct task *current = head;
    //Free original list
    while (current != NULL){
        trailing = current;
        current = current->next;
        free(trailing);
    }
}

//Duplicates linked list so original list doesn't accidentally get freed between algorithms
struct task *duplicate_list(struct task *list)
    {
        //empty list fallback
        if (list == NULL) {
            return NULL;
        }

        // create the head node, keeping it for later return
        struct task *head_node = (struct task*)malloc(sizeof(struct task));
        head_node->arrival_time = list->arrival_time;
        head_node->completion_time = list->completion_time;
        head_node->has_run = list->has_run;
        head_node->next = list->next;
        head_node->remaining_time = list->remaining_time;
        head_node->response_time = list->response_time;
        head_node->service_time = list->service_time;
        head_node->task_id = list->task_id;
        head_node->wait_time = list->wait_time;

        // the 'temp' pointer points to the current "last" node in the new list
        struct task *temp = head_node;

        list = list->next;
        while (list != NULL)
        {
            struct task *new_node = (struct task*)malloc(sizeof(struct task));
            new_node->arrival_time = list->arrival_time;
            new_node->completion_time = list->completion_time;
            new_node->has_run = list->has_run;
            new_node->next = list->next;
            new_node->remaining_time = list->remaining_time;
            new_node->response_time = list->response_time;
            new_node->service_time = list->service_time;
            new_node->task_id = list->task_id;
            new_node->wait_time = list->wait_time;

            // modify the Next pointer of the last node to point to the new last node
            temp->next = new_node;
            temp = new_node;
            list = list->next;
        }

        return head_node;

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

    new_node->next = NULL;
    
    return new_node;
}



/*
Summary:
Input:
Output:
*/
void fairmix(struct task *list, char* current_output_file, int time_slice, int overhead){
    int time_count = 0; //Counts up for each CPU cycle
    int current_cycle = 1;
    int next_max_min = 0; //0 if next cycle should use min, 1 if next cycle should use max
    int is_task_finished = 0;
    int current_overhead_value = 0;
    int task_run_on_current_cycle = 0;

    struct task *ready_queue = NULL; //Declare ready queue

    while (list != NULL || ready_queue != NULL){ //change to while 1 and then return later?
        printf("TIME: %i\n", time_count);
        //If last cycle finished a task, new cycle should reverse max_min
        if (is_task_finished){
            current_cycle = 0;
            next_max_min = reverse_max_min(next_max_min);
            is_task_finished = 0;
        }
        //If modulo returns 0 that means time slice has finished so reverse max_min
        else if (time_count != 0 && current_cycle % time_slice == 0){
            current_cycle = 0;
            next_max_min = reverse_max_min(next_max_min);
        }

        struct task* list_rover = list;
        task_run_on_current_cycle = 0;
        struct task *rover = list_rover;
        while (rover != NULL)
        {
            //If task is due next cycle or has recently been run but isn't in queue, append it to ready queue to be run in the future
            if ((is_task_finished || current_cycle % time_slice == 0) && (!is_in_ready_queue(ready_queue, list_rover) && (list_rover->arrival_time <= time_count))){
                current_cycle = 0;
                //Delinks node from old list and saves reference
                if (rover->next == NULL){
                    break;
                }
                else if (rover == list_rover){
                    list = list_rover->next;
                }
                else{
                    while (rover->next != list_rover){
                        rover = rover->next;
                    }
                    rover->next = list_rover->next;
                }

                printf("\nAppending: %i\n", list_rover->service_time);
                ready_queue = append_to_ready_queue(NULL, ready_queue, list_rover);
                rover = list;
                printf("yep: %i\n", rover->service_time);
            }
            else{
                rover = rover->next;
            }
        }


        //If the ready queue still has contents in it, decrement value from the head node (which should be the node that needs to be worked on)
        if (ready_queue != NULL && !task_run_on_current_cycle){

            ready_queue = sort_return_queue(ready_queue, next_max_min);

            if (current_cycle % time_slice == 0){
                current_overhead_value = current_overhead_value + overhead; //Increases overhead at context switch
            }

            ready_queue->service_time = ready_queue->remaining_time;
            ready_queue->remaining_time--;
            task_run_on_current_cycle = 1;
            printf("Task ID: %i, Service Time: %i, Remaining Time: %i", ready_queue->task_id, ready_queue->service_time, ready_queue->remaining_time);

            //If the task has been fulfilled, remove it from the ready queue and move next node to front
            if (ready_queue->remaining_time <= 0){

                struct task *temp_ready_queue_node;
                if (ready_queue->next == NULL){
                    free(ready_queue);
                }
                else{
                    temp_ready_queue_node = ready_queue->next;
                    free(ready_queue);
                    ready_queue = temp_ready_queue_node;
                }
                
                //Everything has been finished
                if (list == NULL && ready_queue == NULL){

                }
                
                /*
                struct task *temp_list_rover = list;
                if (temp_list_rover->next == NULL){ //Only one node in list so just free it
                    free(temp_list_rover);
                }
                else{
                    while (temp_list_rover->next != ready_queue){
                        temp_list_rover = temp_list_rover->next;
                    }
                    struct task *temp_list_node = temp_list_rover->next;
                    temp_list_rover->next = temp_list_rover->next->next;
                    free(temp_list_node);
                }*/
                
                ready_queue = temp_ready_queue_node;

                printf(" (done)");
                is_task_finished = 1;
            }
            else {
                struct task *temp = ready_queue;
                ready_queue = ready_queue->next;
                ready_queue = append_to_ready_queue(NULL, ready_queue, temp);
            }
            printf("\n");
        }
        
        time_count++;
        current_cycle++;
    }
}

//Reverses max_min
int reverse_max_min(int max_min){
    if (max_min == 0){
        max_min = 1;
    }
    else{
        max_min = 0;
    }
    return max_min;
}

//Checks if given node is currently in ready queue and returns 1 if yes and 0 if no
int is_in_ready_queue(struct task *queue, struct task *node){
    struct task* rover = queue;
    while (rover != NULL)
    {
        if (rover == node){
            return 1;
        }

        rover = rover->next;
    }
    return 0; //Returns 0 if couldn't find node in list
}

//Appends input node to the ready queue and returns the new queue with the new node inserted
struct task *append_to_ready_queue(struct task *list, struct task *queue, struct task *node){
    struct task *rover;

    node->next = NULL;
    //If queue is empty then make node the head of the list
    if (queue == NULL){
        queue = node;
    }
    else{
        rover = queue;
        while (rover->next != NULL){
            rover = rover->next;
        }
        rover->next = node;
    }
    return queue;
}

/*
Summary:Sorts the ready queue and then returns the head node of the queue which is
 the node that should be run in the current cycle. Also frees the node from the queue as it is being used
Input: Current ready queue (which should not be empty) and max_min defines how to sort the queue
Output: The head node of the queue
*/
struct task *sort_return_queue(struct task *queue, int max_min){
    
    int queue_length = 1;
    struct task *rover = queue;

    //Get count of # nodes in queue
    while (rover->next != NULL){
        queue_length++;
        rover = rover->next;
    }

    printf("\nQueue before sorting with max_min of %i: \n", max_min);
    rover = queue;
    while (rover != NULL){
        printf("Task ID: %i, Service Time: %i\n", rover->task_id, rover->service_time);
        rover = rover->next;
    }
    
    int sorted_array[queue_length];

    int min = queue->service_time;
    int max = queue->service_time;
    int starting_position = 0;
    
    //Only sorts head node of list so that it is MIN or MAX of list depending on max_min input
    if (max_min == 0){

        if (queue->next == NULL){
            return queue; //List is sorted because it is only one node
        }

        rover = queue;
        struct task *temp = rover;
        while (rover != NULL){
            if (rover->service_time < min){
                min = rover->service_time;
                temp = rover; //Temp should point to node with smallest service time (MIN VALUE)
            }
            rover = rover->next;
        }

        queue = swap_nodes(queue, queue, temp);
        max_min = reverse_max_min(max_min);
    }
    else{ //Sort for head = max of queue
        if (queue->next == NULL){
            return queue; //List is sorted because it is only one node
        }

        rover = queue;
        struct task *temp = rover;
        while (rover != NULL){
            if (rover->service_time > max){
                max = rover->service_time;
                temp = rover; //Temp should point to node with largest service time (MAX VALUE)
            }
            rover = rover->next;
        }

        queue = swap_nodes(queue, queue, temp);
        max_min = reverse_max_min(max_min);
    }
    starting_position++; //Move starting position forward by one since head is now sorted correctly

    //Knowing the list has more than one node, iterates through list and sorts list correctly (ideally)
    for (int i = starting_position; i < queue_length; i++)
    {
        int j = 0;
        struct task *starting_position_rover = queue;
        //Make sure that sorting starts at new position so it doesn't overwrite already sorted data in array
        while (j != i){
            starting_position_rover = starting_position_rover->next;
            j++;
        }

        min = starting_position_rover->service_time;
        max = starting_position_rover->service_time;
        rover = starting_position_rover;

        if (max_min == 0){
            struct task *temp = rover;
            while (rover != NULL){
                if (rover->service_time < min){
                    min = rover->service_time;
                    temp = rover; //Temp should point to node with smallest service time (MIN VALUE)
                }
                rover = rover->next;
            }
            queue = swap_nodes(queue, starting_position_rover, temp);
            max_min = reverse_max_min(max_min);
        }
        else {
            struct task *temp = rover;
            while (rover != NULL){
                if (rover->service_time > max){
                    max = rover->service_time;
                    temp = rover; //Temp should point to node with largest service time (MAX VALUE)
                }
                rover = rover->next;
            }
            queue = swap_nodes(queue, starting_position_rover, temp);
            max_min = reverse_max_min(max_min);
        }

    }

    printf("\nQueue After Sorting: \n");
    rover = queue;
    while (rover != NULL){
        printf("Task ID: %i, Service Time: %i\n", rover->task_id, rover->service_time);
        rover = rover->next;
    }
    printf("\n");

    return queue; //Return sorted list
}


//Swaps two given nodes and returns list. Expects node 1 before node 2
struct task *swap_nodes(struct task *list, struct task *node1, struct task *node2){
    struct task *rover = list;

    if (node1 == node2){ //Don't execute function if same values
        return list;
    }

    if (node1 == list){ //First node is head of list so special logic required
        
        while (rover->next != node2){ //Move rover to node before node2
            rover = rover->next;
        }

        if (node1->next == node2){
            node1->next = node2->next;
            node2->next = node1;
            return node2;
        }

        struct task *after2 = node2->next;
    
        rover->next = node1;
        node2->next = node1->next;
        node1->next = after2;

        return node2; //New head
    }
    else{
        struct task *temp_rover = list;

        while (temp_rover->next != node1){
            temp_rover = temp_rover->next; //Move other rover to node before node1
        }
        while (rover->next != node2){ //Move rover to node before node2
            rover = rover->next;
        }

        if (node1->next == node2){
            node1->next = node2->next;
            node2->next = node1;
            temp_rover->next = node2;
            return list;
        }

        struct task *after2 = node2->next;

        temp_rover->next = node2;
        rover->next = node1;
        struct task *temp = node1->next;
        node1->next = after2;
        node2->next = temp;

        return list;
    }
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