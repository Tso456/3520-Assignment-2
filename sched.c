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
        wait_time;
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





    return 0;
}