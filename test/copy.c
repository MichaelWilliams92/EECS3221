/*
 * Family name: Williams 
 * Given Name: Michael 
 * Student Number: 211087798
 * EECS Login: mw1992 
*/

//fix up string stuff

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//included classes
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <semaphore.h>

#define TEN_MILLIS_IN_NANOS 10000000

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//buffer item structure
typedef  struct {
     char  data;
     off_t offset ;
     char string[7]; //state of buffer cell, change to 1, 0 state
} BufferItem ;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BufferItem *buffer;

//more variables
FILE *file;
FILE *copy;
FILE *log_record;

int i = 0; //counter
int buffer_size = 0;
int n_IN = 0;
int n_OUT = 0;
////////////////////////////////////////////////////////////////////////////////
int OUT_helper = 0; //helps for exiting OUT function

//int in_thread_count; //whats this for??

int *can_in_join; //what
int *can_out_join; //what

//mutex locks
pthread_mutex_t operations_lock; //lock for when reading/producing and writing/consuming
pthread_mutex_t done_lock;  //lock for when letting main know thread is finished
pthread_mutex_t IN_increaser_lock; //lock for when increasing IN thread number
pthread_mutex_t OUT_increaser_lock; //lock for when increasing OUT thread number
pthread_mutex_t IN_decreaser_lock; //lock for decreasing IN thread number

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


void sleep_wait(){
struct timespec t;

t.tv_sec = 0;
t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);

nanosleep(&t, NULL);
}//sleep function

////////////////////////////////////////////////////////////////////////////////////////////////////////////

////find empty buffer item in buffer
int empty_buffer_item(){

    for(i = 0; i < buffer_size; i++){
	if(strcmp(buffer[i].string, "empty") == 0)
		return i;
    }//if an empty buffer item is found, return its index
    
		return -1; //if not found, return -1

}//find empty buffer item in buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

////find a filled buffer item in buffer
int filled_buffer_item(){

    for(i = 0; i < buffer_size; i++){
	if(strcmp(buffer[i].string, "filled") == 0){
		return i;
	}//if loop
    }//if a filled buffer item is found, return its index
	
    
		return -1; //if not found, return -1

}//find a filled buffer item in buffer

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// the thread process for IN threads
void *IN(void *arg){

sleep_wait(); //thread sleeps for a random time 

//local variables
int index = 0;
int IN_thread_number = 0;
int IN_offset = 0;
char IN_byte = ' ';

//increasing IN thread number
pthread_mutex_lock(&IN_increaser_lock);
IN_thread_number = n_IN++;
//IN_thread_number = n_IN;
//n_IN = n_IN + 1;
pthread_mutex_unlock(&IN_increaser_lock);

do{

    //lock for reading and producing
    pthread_mutex_lock(&operations_lock);
    
    index = empty_buffer_item();

	while(index != -1){
	  //while there is space to insert into buffer

		//read info
		IN_offset = ftell(file);		
		IN_byte = fgetc(file);

		//write to log
		if(fprintf(log_record, "read_byte PT%d O%d B%d I-1\n", IN_thread_number, IN_offset, IN_byte) < 0) {
			fprintf(stderr, "log file not valid\n");
			exit(-1);
		}//finished writing

		if(IN_byte == EOF)
			break;

		//produce into buffer
		else{
			buffer[index].offset = IN_offset;
			buffer[index].data = IN_byte;
			strcpy(buffer[index].string, "filled");
		}//item placed into buffer

		//write to log
		if(fprintf(log_record, "produce PT%d O%d B%d I%d\n", IN_thread_number, IN_offset, IN_byte, index) < 0) {
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
pthread_mutex_lock(&IN_decreaser_lock);
OUT_helper = OUT_helper - 1;
pthread_mutex_unlock(&IN_decreaser_lock);

/////////////////////////////////////////////////////////////////////
//can_join stuff, is it needed??
pthread_mutex_lock(&done_lock);
can_in_join[IN_thread_number] = 1;
pthread_mutex_unlock(&done_lock);


////////////////////////////////////////////////////////////////////

sleep_wait();
pthread_exit(0);

}//IN function
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// the thread process for IN threads
void *OUT(void *arg){

sleep_wait();

//local variables
int OUT_thread_number = 0;
int OUT_offset = 0;
char OUT_byte = ' ';
int index = 0;

int set_offset = 0;  //replaces val
int threads_working = 0; //replaces threads_running

//increasing OUT thread number
pthread_mutex_lock(&OUT_increaser_lock);
OUT_thread_number = n_OUT++;
//OUT_thread_number = n_OUT;
//n_OUT = n_OUT + 1;
pthread_mutex_unlock(&OUT_increaser_lock);

do {

   //get index of item to be consumed
   index = filled_buffer_item();

   if(index != -1){

	//lock for consuming
	pthread_mutex_lock(&operations_lock);
	index = filled_buffer_item();////////////////////////is this neccesary??

	//consume item in buffer
	OUT_offset = buffer[index].offset;
	OUT_byte = buffer[index].data;

	//write to log
	if(fprintf(log_record, "consume CT%d O%d B%d I%d\n", OUT_thread_number, OUT_offset, OUT_byte, index) < 0) {
			fprintf(stderr, "log file not valid\n");
			exit(-1);
	}//finished writing

	//strcpy stuff, change MAKE SURRRE
	strcpy(buffer[index].string, "empty");
	buffer[index].offset = 0;

	//unlock 
	pthread_mutex_unlock(&operations_lock);

	//lock for writing
	pthread_mutex_lock(&operations_lock);

	//write item to file
	set_offset = fseek(copy, OUT_offset, SEEK_SET); //what is seek set??
	   if(set_offset < 0){
		pthread_mutex_unlock(&operations_lock);
		fprintf(stderr, "something wrong with what you seek\n");
			exit(-1);
	   }//if loop checking for problem in set_offset

	   if(fputc(OUT_byte, copy) == EOF){
		pthread_mutex_unlock(&operations_lock);
		fprintf(stderr, "File to be written to not valid\n");
			exit(-1);
	   }//if loop checking for problem with copy file

	   
	   //write log
	if(fprintf(log_record, "write_byte CT%d O%d B%d I-1\n", OUT_thread_number, OUT_offset, OUT_byte) < 0) {
		fprintf(stderr, "log file not valid\n");
		exit(-1);
	}//finished writing

	//unlock
	pthread_mutex_unlock(&operations_lock);
	

   }//if there is an item to consume in the buffer

   sleep_wait();
   
   //checking if threads are done work
   threads_working = OUT_helper;

}while(threads_working > 0 || index != -1); //while loop writing to file or consuming and writing to log file


////////////////////////////////////////////////////////////////////////
//can_join stuff, is it needed??
pthread_mutex_lock(&done_lock);
can_out_join[OUT_thread_number] = 1;
pthread_mutex_unlock(&done_lock);


//////////////////////////////////////////////////////////////

  pthread_exit(0);
}//OUT function
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*main will do all initial input checking and create IN and OUT threads
/Then main will send threads to the other function to do the other work
*/

int main(int argc, char *argv[]){
//cpy <nIN> <nOUT> <file> <copy> <bufSize> <Log>

//first check number of inputs
  if (argc != 7) {
	fprintf(stderr, "not enough arguments given\n");
	return -1;
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
	return -1;
  }

//no checking argv[3], will check when we run file open

//checking argv[4]
if (argv[4] == NULL){
	fprintf(stderr, "invalid name for copy file");
	exit(-1);
}

//chekcing arg5
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//intialize locks
pthread_mutex_init(&operations_lock, NULL);
pthread_mutex_init(&done_lock, NULL);
pthread_mutex_init(&IN_increaser_lock, NULL);
pthread_mutex_init(&OUT_increaser_lock, NULL);
pthread_mutex_init(&IN_decreaser_lock, NULL); //lock for decreasing IN thread number, this was left out!!!!!!!!
//pthread_mutex_init(&operations_lock, NULL);

//open source file, copy file, and log file

    file = fopen(argv[3], "r");             //MAY NEED TO PUT *
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

/////////////////////////////////////////////////////////////////////////////////////////

//create and initialize initial circular buffer and variables

buffer_size = atoi(argv[5]);
n_IN = atoi(argv[1]);
OUT_helper = n_IN; //OUT function helper
n_OUT = atoi(argv[2]);

//////////////////////////////////////////////////////////
//can join mallocs
can_in_join = malloc(n_IN * sizeof(int));
can_out_join = malloc(n_OUT * sizeof(int));

if(can_in_join == NULL || can_out_join == NULL){
	fprintf(stderr, "cant assign memory to threads!!\n");
	exit(-1);
}


////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
    pthread_t t_IN[n_IN];
    pthread_t t_OUT[n_OUT];
    pthread_attr_t  t_attribute; //should this be an array aswell??
//////////////////////////////////////////////////////////////////////////////

//allocate buffer memory
   buffer = (BufferItem*) malloc(buffer_size * sizeof(BufferItem));

   for(i = 0; i < buffer_size; i++){
	strcpy(buffer[i].string, "empty"); //initialize to empty, change later
	}//for loop for making empty

   //int travel = 0; //rename, what is this??
   //in_thread_count = atoi(argv[1]);
   
//can in/out join mallocs?

   //initialize mutex locks
   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//create threads
//shoudl attributes be NULL??

   for (i = 0; i < n_IN; i++) {

	pthread_attr_init(&t_attribute);           //MAYBE THIS IS WRONG
	pthread_create(&t_IN[i], NULL, (void *) IN, file);

    }// for loop, creating array of IN threads

  for (i = 0; i < n_OUT; i++) {

	//pthread_attr_init(&t_attribute);             //MAYBE ATTRIBUTE IS WRONG
	pthread_create(&t_OUT[i], NULL, (void *) OUT, copy);

    }// for loop, creating array of OUT threads

//wait for threads to finish
//should this be 1 loop???

/////////////////////////////////////////////////////////////////////////
//can_join checker

for(i = 0; i < n_IN; i++){
   while(can_in_join[i] == 0);
}

for(i = 0; i < n_OUT; i++){
   while(can_out_join[i] == 0);
}

//////////////////////////////////////////////////////////////////////

	//can_in/out_join loops go here, what for??

    for (i = 0; i < n_IN; i++) {

	pthread_join(t_IN[i], NULL);

    }//for loop waiting for IN threads to finish

   for (i = 0; i < n_OUT; i++) {

	pthread_join(t_OUT[i], NULL);

    }//for loop waiting for IN threads to finish

fclose(file);
fclose(copy);
fclose(log_record);

free(buffer);
free(can_in_join);
free(can_out_join);

//return 0;

exit(0); //program finished
}//main

//////////////////////////////////////////////////////////////////////////////////////////////////////////////











