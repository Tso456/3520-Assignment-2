#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct task* create_node(int arrival_time, int service_time);

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

int main (int argc, char *argv[]){

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
    
    //For each line of text of input file, pass in data to new node of linked list
    int j = 0;
    struct task* head;
    while (fscanf(fptr, "%i %i", &file_arrival_time, &file_service_time) == 2) {
        if (j == 0){
            j++;
            head = create_node(file_arrival_time, file_service_time);
        }
        else {
            struct task* rover = head;
            while (rover->next != NULL)
            {
                rover = rover->next;
            }
            rover->next = create_node(file_arrival_time, file_service_time);
        }
        
    }
    struct task* rover = head;
    while (rover->next != NULL)
    {
        rover = rover->next;
    }
    rover->next = head; //Complete circular list

    fclose(fptr);

    return 0;
}

//Create new node and links appropriate data
struct task* create_node(int arrival_time, int service_time)
{
    struct task* new_node = (struct task*)malloc(sizeof(struct task));
    
    new_node->service_time = service_time;
    new_node->arrival_time = arrival_time;
    new_node->remaining_time = service_time;
    
    new_node->completion_time = 0;
    new_node->response_time = 0;
    new_node->wait_time = 0;
    
    return new_node;
}
