/*
 * Family name: Williams 
 * Given Name: Michael 
 * Student Number: 211087798
 * EECS Login: mw1992 
 *
 *
 *copy.c is a program that creates two types of threads to perform seperate functions.  
 *After these threads have performed their function, the result is an exact copy
 *of the source file passed as a command-line argument.
 *
 *In addition to this, the program also creates a log file to allow clients
 *to trace the execution of the program and see exactly how it functions,
 *including the exact order the operations are performed.
*/

//included classes
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <semaphore.h>

#define TEN_MILLIS_IN_NANOS 10000000

//buffer item structure
typedef  struct {
     char  data;
     off_t offset ;
     char string[7];
} BufferItem ;

BufferItem *buffer = NULL;

//global variables
FILE *file = NULL;
FILE *copy = NULL;
FILE *log_record = NULL;


int i = 0; //counter
int buffer_size = 0;
int n_IN = 0;
int n_OUT = 0;

int out_helper = 0; //helps for exiting OUT function

int *can_in_join = NULL;
int *can_out_join = NULL;

//mutex locks
pthread_mutex_t operations_lock; //lock for when reading/producing and writing/consuming
pthread_mutex_t done_lock;  //lock for when letting main know thread is finished
pthread_mutex_t in_increaser_lock; //lock for when increasing IN thread number
pthread_mutex_t out_increaser_lock; //lock for when increasing OUT thread number
pthread_mutex_t in_decreaser_lock; //lock for decreasing IN thread number

void sleep_wait();
int filled_item_buffer();
int empty_item_buffer();
void *in(void *arg);
void *out(void *arg);
void close_release();

/*main will do all initial input checking and create IN and OUT threads
/Then main will send threads to the other function to do the other work
*/
int main(int argc, char *argv[]){
//information should be sent in the form "cpy <nIN> <nOUT> <file> <copy> <bufSize> <Log>"

//first check number of inputs
  if (argc != 7) {
	fprintf(stderr, "not enough arguments given\n");
	exit(-1);
  }

//checking argv[1]
//do we need to make sure its an integer??
  if (atoi(argv[1]) < 1){
	fprintf(stderr, "invalid number for IN threads.  must be a value >= 1\n");
	exit(-1);
  }

//checking argv[2]
//do we need to make sure its an integer??
  if (atoi(argv[2]) < 1){
	fprintf(stderr, "invalid number for OUT threads.  must be a value >= 1");
	exit(-1);
  }

//no checking argv[3], will check when we run file open

//checking argv[4]
if (argv[4] == NULL){
	fprintf(stderr, "invalid name for copy file");
	exit(-1);
}

//chekcing argv[5]
//do we need to make sure its an integer?
if(atoi(argv[5]) < 1){
	fprintf(stderr, "invalid buffer size. must be >= 1");
	exit(-1);
}

//checking argv[6]
if (argv[6] == NULL){
	fprintf(stderr, "invalid name for log_record file");
	exit(-1);
}

//intialize locks
pthread_mutex_init(&operations_lock, NULL);
pthread_mutex_init(&done_lock, NULL);
pthread_mutex_init(&in_increaser_lock, NULL);
pthread_mutex_init(&out_increaser_lock, NULL);
pthread_mutex_init(&in_decreaser_lock, NULL); 

//open source file, copy file, and log file

    file = fopen(argv[3], "r");          
       copy = fopen(argv[4], "w");
       log_record = fopen(argv[6], "w");

	// check for error in fopens
   if (file == NULL) {
	fprintf(stderr, "File does not exists or improper file given\n");
	exit(-1);
	}

   if (copy == NULL) {
	fprintf(stderr, "File does not exists or improper file given\n");
	exit(-1);
	}

       if (log_record == NULL) {
	fprintf(stderr, "File does not exists or improper file given\n");
	exit(-1);
	}  

//open copy

//create and initialize initial circular buffer and variables



buffer_size = atoi(argv[5]);
n_IN = atoi(argv[1]);
out_helper = n_IN; //OUT function helper
n_OUT = atoi(argv[2]);

can_in_join = malloc(n_IN * sizeof(int));
can_out_join = malloc(n_OUT * sizeof(int));

if(can_in_join == NULL || can_out_join == NULL){
	fprintf(stderr, "cant assign memory to threads!!\n");
	exit(-1);
}

    pthread_t t_IN[n_IN];
    pthread_t t_OUT[n_OUT];
    pthread_attr_t  t_attribute; 

//allocate buffer memory

	printf("\tCreating Buffer...\n\t");

   buffer = (BufferItem*) malloc(buffer_size * sizeof(BufferItem));

   for(i = 0; i < buffer_size; i++){
	strcpy(buffer[i].string, "empty"); //initialize to empty, change later
	}//for loop for making empty

   //initialize mutex locks

//create threads
//shoudl attributes be NULL??

printf("Creating in threads...\n\t");

   for (i = 0; i < n_IN; i++) {

	pthread_attr_init(&t_attribute);        
	pthread_create(&t_IN[i], NULL, (void *) in, file);

    }// for loop, creating array of IN threads

printf("Creating out threads...\n\t");

  for (i = 0; i < n_OUT; i++) {

	pthread_create(&t_OUT[i], NULL, (void *) out, copy);

    }// for loop, creating array of OUT threads

//wait for threads to finish

printf("Waiting for in threads...\n\t");

for(i = 0; i < n_IN; i++){
   while(can_in_join[i] == 0);  //something happens here

}

	printf("Waiting for out threads...\n\t");

for(i = 0; i < n_OUT; i++){
   while(can_out_join[i] == 0);
}

	//can_in/out_join loops go here, what for??

printf("Waiting for join in threads...\n\t");

    for (i = 0; i < n_IN; i++) {


	pthread_join(t_IN[i], NULL);

    }//for loop waiting for IN threads to finish

   for (i = 0; i < n_OUT; i++) {

printf("Waiting for join out threads...\n\t");

	pthread_join(t_OUT[i], NULL);

    }//for loop waiting for IN threads to finish

	printf("Finished Successfully.\n");

//close_release();

fclose(file);
fclose(copy);
fclose(log_record);

//free(buffer);
//free(can_in_join);
//free(can_out_join);

exit(0); //program finished
}//main

void sleep_wait(){
struct timespec t;

t.tv_sec = 0;
t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);
nanosleep(&t, NULL);
}//sleep function

////find empty buffer item in buffer
int empty_buffer_item(){

    for(i = 0; i < buffer_size; i++){
	if(strcmp(buffer[i].string, "empty") == 0)
		return i;
    }//if an empty buffer item is found, return its index
    
		return -1; //if not found, return -1

}//find empty buffer item in buffer

////find a filled buffer item in buffer
int filled_buffer_item(){

    for(i = 0; i < buffer_size; i++){
	if(strcmp(buffer[i].string, "filled") == 0){
		return i;
	}//if loop

    }//if a filled buffer item is found, return its index
    
		return -1; //if not found, return -1

}//find a filled buffer item in buffer

// the thread process for IN threads
void *in(void *arg){

sleep_wait(); //thread sleeps for a random time 

//local variables
int index = 0;
int in_thread_number = 0;
int in_offset = 0;
char in_byte = 0;

//increasing IN thread number
pthread_mutex_lock(&in_increaser_lock);
in_thread_number = n_IN++;
pthread_mutex_unlock(&in_increaser_lock);

do{

    //lock for reading and producing
    pthread_mutex_lock(&operations_lock);
    
    index = empty_buffer_item();

	while(index != -1){
	  //while there is space to insert into buffer

		//read info
		in_offset = ftell(file);		
		in_byte = fgetc(file);

		//write to log
		if(fprintf(log_record, "read_byte PT%d O%d B%d I-1\n", in_thread_number, in_offset, in_byte) < 0) {
			fprintf(stderr, "log file not valid\n");
			exit(-1);
		}//finished writing

		if(in_byte == EOF)
			break;

		//produce into buffer
		else{
			buffer[index].offset = in_offset;
			buffer[index].data = in_byte;
			strcpy(buffer[index].string, "filled");
		}//item placed into buffer

		//write to log
		if(fprintf(log_record, "produce PT%d O%d B%d I%d\n", in_thread_number, in_offset, in_byte, index) < 0) {
			fprintf(stderr, "log file not valid\n");
			exit(-1);
		}//finished writing

		index = empty_buffer_item();
		sleep_wait();

	   }//while there is space to insert item into buffer
		
     //unlock
     pthread_mutex_unlock(&operations_lock);
   } while (!feof(file)); ////while loop reading from file and writing to log file

   //decreasing IN thread number
pthread_mutex_lock(&in_decreaser_lock);
out_helper = out_helper - 1;
pthread_mutex_unlock(&in_decreaser_lock);

//lock for allowing main to know that function is done
pthread_mutex_lock(&done_lock);
can_in_join[in_thread_number] = 1;
pthread_mutex_unlock(&done_lock);

sleep_wait();
pthread_exit(0);

}//IN function

// the thread process for IN threads
void *out(void *arg){

sleep_wait();

//local variables
int out_thread_number = 0;
int out_offset = 0;
char out_byte = 0;
int index = 0;

int set_offset = 0;  //replaces val
int threads_working = 0; //replaces threads_running

//increasing OUT thread number
pthread_mutex_lock(&out_increaser_lock);
out_thread_number = n_OUT++;
pthread_mutex_unlock(&out_increaser_lock);

do {

   //get index of item to be consumed
   index = filled_buffer_item();

   if(index != -1){

	//lock for consuming
	pthread_mutex_lock(&operations_lock);
	index = filled_buffer_item();

	//consume item in buffer
	out_offset = buffer[index].offset;
	out_byte = buffer[index].data;

	//write to log
	if(fprintf(log_record, "consume CT%d O%d B%d I%d\n", out_thread_number, out_offset, out_byte, index) < 0) {
			fprintf(stderr, "log file not valid\n");
			exit(-1);
	}//finished writing

	strcpy(buffer[index].string, "empty");
	buffer[index].offset = 0;

	//unlock 
	pthread_mutex_unlock(&operations_lock);

	//lock for writing
	pthread_mutex_lock(&operations_lock);

	//write item to file
	set_offset = fseek(copy, out_offset, SEEK_SET); 
	   if(set_offset < 0){
		pthread_mutex_unlock(&operations_lock);
		fprintf(stderr, "something wrong with what you seek\n");
			exit(-1);
	   }//if loop checking for problem in set_offset

	   if(fputc(out_byte, copy) == EOF){
		pthread_mutex_unlock(&operations_lock);
		fprintf(stderr, "File to be written to not valid\n");
			exit(-1);
	   }//if loop checking for problem with copy file

	   
	   //write log
	if(fprintf(log_record, "write_byte CT%d O%d B%d I-1\n", out_thread_number, out_offset, out_byte) < 0) {
		fprintf(stderr, "log file not valid\n");
		exit(-1);
	}//finished writing

	//unlock
	pthread_mutex_unlock(&operations_lock);
	

   }//if there is an item to consume in the buffer

   sleep_wait();
   
   //checking if threads are done work
   threads_working = out_helper;

}while(threads_working > 0 || index != -1); //while loop writing to file or consuming and writing to log file


//lock for allowing main to know that function is done
pthread_mutex_lock(&done_lock);
can_out_join[out_thread_number] = 1;
pthread_mutex_unlock(&done_lock);

  pthread_exit(0);
}//OUT function
