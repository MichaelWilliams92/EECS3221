
#include <stdlib.h> 
#include <pthread.h> 
#include <stdio.h>

typedef struct {

    float max;
    float min;
    float sum;
    float dif;
    FILE * fr;

}Array;

void * GetData(void * );

Array * Result;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int threadnum = 0;

int main(int argc, char * argv[]) {

    int FileNum;

    if (argc < 2) {
        fprintf(stderr, "usage: a.out <intger value>\n");
        return -1;
    }

    if (atoi(argv[1]) < 0) {
        fprintf(stderr, "%d must be >= 0\n", atoi(argv[1]));
        return -1;
    }

    pthread_t tid[argc];
    pthread_attr_t attr;
    Result = (Array * ) malloc(sizeof(Array) * argc);


    for (FileNum = 1; FileNum < argc; FileNum++) {

        char * FileName = argv[FileNum];

        pthread_attr_init( & attr);

        pthread_create( & tid[FileNum - 1], & attr, GetData, FileName);
    }

    for (FileNum = 1; FileNum < argc; FileNum++) {

        pthread_join(tid[FileNum - 1], NULL);

    }

    float Maximum = 0;
    float Minimum = 0;

    for (FileNum = 1; FileNum < argc; FileNum++) {

        if (Maximum == 0) {
            Maximum = Result[FileNum - 1].max;
        } else if (Result[FileNum - 1].max > Maximum) {
            Maximum = Result[FileNum - 1].max;

        }

        if (Minimum == 0) {
            Minimum = Result[FileNum - 1].min;
        } else if (Result[FileNum - 1].min < Minimum) {
            Minimum = Result[FileNum - 1].min;

        }
        printf("%s SUM=%0.5f DIF=%0.5f MIN=%0.5f MAX=%0.5f\n", argv[FileNum], Result[FileNum - 1].sum, Result[FileNum - 1].dif, Result[FileNum - 1].min, Result[FileNum - 1].max);
    }

    printf("MINIMUM=%0.5f MAXIMUM=%0.5f\n", Minimum, Maximum);
    return 0;

}

void * GetData(void * Name) {

    //int upper = atoi(param);
    float num = 0;
    float tmpmax = 0;
    float tmpmin = 0;
    float tmpsum = 0;
    float tmpdif = 0;

    FILE * file = fopen(Name, "r");

    if (Name == NULL) {
        printf("%d\n", "No File");
        exit(-1);
    }

    while (fscanf(file, "%f", & num) != EOF) {

        if (tmpmax == 0) {
            tmpmax = num;
        } else if (num > tmpmax) {
            tmpmax = num;

        }

        if (tmpmin == 0) {
            tmpmin = num;
        } else if (num < tmpmin) {
            tmpmin = num;

        }

    }

    tmpsum = tmpmax + tmpmin;
    tmpdif = tmpmin - tmpmax;

    pthread_mutex_lock( & lock);

    Result[threadnum].max = tmpmax;
    Result[threadnum].min = tmpmin;
    Result[threadnum].sum = tmpsum;
    Result[threadnum].dif = tmpdif;
    Result[threadnum].fr = Name;

    threadnum++;

    pthread_mutex_unlock( & lock);

    fclose(file);

    pthread_exit(0);
}
