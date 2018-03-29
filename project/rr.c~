/*
* Family name: Williams 
* Given Name: Michael 
* Student Number: 211087798
* EECS Login: mw1992 
* 
*
*The RR is an algorithm in which all processes are assigned to the same quantum time or time limit, in conjunction with a FIFO, or first in first out, *structure.  When the quantum time duration has expired, the CPU performs a context switch, in which the state of the current process is saved and placed at the *end of the queue for later processing.  A new process is then started and the cycle continues.  More information is available in the report.
*
* This program runs multiple processes from a given .dat file and after running all processes
* prints out information answering the following questions:
*
* What is the average waiting time?
* What is the average turnaround time?
* When does the CPU finish all these processes? 
* What is average CPU utilization by this time point? 
* How many context switches occur in total during the execution? 
* Which process is the last one to finish?
*/

//////////////////////////////////////////////////////////////////////////////////////////////////

//included files
#include<string.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

//include helper file given
#include"sch-helpers.h"

//initialize processes and variables.

process_queue readyQ;   		//ready Queue
process_queue waitQ;			//Waiting Queue
process *CPU[NUMBER_OF_PROCESSORS];     //CPUs for the program
process *tempQ[MAX_PROCESSES + 1];      //used for sorting, contains processes going to readyQ
process processes[MAX_PROCESSES + 1];   //used for storing processes of the file

process requests[NUMBER_OF_PROCESSORS]; //new process added for requests
process *storedProcesses[NUMBER_OF_PROCESSORS]; //contains stored processes

int i = 0;
int lastTime = 0;   
int processSize = 0; //number of processes
int tempQSize = 0;   
int cpuTime = 0;    //utilization time
int simulationTime = 0; 
int processEntering = 0;
int contextSwitches = 0;
int quantumTime = 0; //change name, what does this do?

//Function implementations begin here

//increases the work done by all CPUs running processes
void increaseWorkCPU(void){ 

	for(i = 0; i < NUMBER_OF_PROCESSORS; i++){  //traverse through all processors

		//if CPU[i] is currently running, take a step and reducec quantum time remaining
		if (CPU[i] != NULL){
			CPU[i]->bursts[CPU[i]->currentBurst].step++;
			 CPU[i]->quantumRemaining--;

		}
	}//for loop

}//increaseWorkCPU function

//increases the work done by each waiting process
void increaseWorkIO(void){

	int size = waitQ.size;
	
	for( i = 0; i < size; i++){  //for each process in the waitQ

	process *x = waitQ.front->data; //get front process
	dequeueProcess(&waitQ);
	x->bursts[x->currentBurst].step++; //take a step in the current burst
	enqueueProcess(&waitQ, x);
	}//for loop	

}//increaseWorkIO

//initialize processes to the temporary starting place, tempQ
void tempProcessInitialization(void){

	while(processEntering < processSize && processes[processEntering].arrivalTime <= simulationTime) {
		tempQ[tempQSize] = &processes[processEntering]; //store process into tempQ

		tempQ[tempQSize]->quantumRemaining = quantumTime; //initialize the quantum time the process will for
		tempQSize++;
		processEntering++;
	}//while loop

}//tempProcessInitialization


//get the next process for the cpu
process *getScheduledProcess(void){
	int size = readyQ.size;

	//check if ready queue is empty
	if(size == 0){
		return NULL;
	}

	//if not empty return the front process
	process *x = readyQ.front->data;
	dequeueProcess(&readyQ);
	return x;

}//*getScheduleProcess


//moves processes from the waiting queue to the tempQ, which will later take it to the ready queue
void moveToReadyQueue(void){

	int size = waitQ.size;

	for(i = 0; i < size; i++){ //for each process in the waitQ

		process *x = waitQ.front->data;
		dequeueProcess(&waitQ);
		
		//check to see if steps have finished
		//if so, move them to the tempQ
		if(x->bursts[x->currentBurst].step == x->bursts[x->currentBurst].length){
			x->currentBurst++;
			tempQ[tempQSize] = x;
			tempQSize++;
			x->quantumRemaining = quantumTime; //assigns the quantum time
		} else{

			//if I/O bursts aren't finished, put it back into waitQ
			enqueueProcess(&waitQ, x);

		}//if else statement end

	}//for loop

}//moveToReady function

//compare 2 process ids
int compareProcesses(const void *a, const void *b){

	process *processA = *((process**) a);
	process *processB = *((process**) b);

	//if processA < processB
	if(processA->pid < processB->pid)
		return -1;

	//if processA > processB
	if(processA->pid > processB->pid )
		return 1;

	//if they contain the same process id
	return 0;
	

}//compareProcesses


//add processes from tempQ to the readyQ and put it in available CPUs
void getReadyProcess(void){

	qsort(tempQ, tempQSize, sizeof(process*), compareProcesses); //sort based on the PID

	//for loop to enqueue tempQ processes to readyQ
	for(i = 0; i < tempQSize; i++){
		enqueueProcess(&readyQ, tempQ[i]);
	}

	tempQSize = 0;

	//give each free CPU a process to work on
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++){
		if(CPU[i] == NULL){CPU[i] = getScheduledProcess();}
	}//for loop


}//getReadyProcess

//obtain number of cpus currently running
int processesWorking(void){

int num = 0;

for(i = 0; i < NUMBER_OF_PROCESSORS; i++){
	if(CPU[i] != NULL){num++;}
}

return num;

}//processesWorking function

//update the CPU processes in the ready queue
void updateProcesses(void){

int size = readyQ.size;

for(i = 0; i < size; i++){

process *x = readyQ.front->data;
dequeueProcess(&readyQ);
x->waitingTime++;
enqueueProcess(&readyQ, x);

}//for loop

}//updateProcesses

//move running processes that are not finished their bursts to waitQ 
void moveToWaitingQueue(void){

int a = 0;
for( i = 0; i < NUMBER_OF_PROCESSORS; i++){

	if(CPU[i] != NULL){

		//if current burst is finished, move to the next burst in the process
		if(CPU[i]->bursts[CPU[i]->currentBurst].step == CPU[i]->bursts[CPU[i]->currentBurst].length){
			CPU[i]->currentBurst++;

			//if not finished, move to waitQ
			if(CPU[i]->currentBurst < CPU[i]->numberOfBursts){
				enqueueProcess(&waitQ, CPU[i]);
	
			} else {				
				//otherwise is finished, so record time information
				CPU[i]->endTime = simulationTime;
				lastTime = CPU[i]->endTime;
			}//if else statement

			CPU[i] = NULL; //free CPU

		//if process has not completed within the quantum time, store them and re-initialize their quantum times remaining
		}else if(CPU[i]->quantumRemaining == 0){
		
		 storedProcesses[a] = CPU[i];
		storedProcesses[a]->quantumRemaining = quantumTime;
		 contextSwitches++;
		a++;
		CPU[i] = NULL;
		}//end of if statement
			
	}//outer if statement
				
}//for loop going through each processor

	//next we sort the stored processess and enqueue them into the readyQ
     qsort(storedProcesses, a, sizeof (process*), compareProcesses); 

     for (i = 0; i < a; i++) { 
         enqueueProcess(&readyQ, storedProcesses[i]); 
     } 


}//moveToWaitingQueue


//main method for running program and computing answers
int main(int argc, char **argv){

//initialize variables

int i = 0;
int status = 0;
int processesLeft = 0;

quantumTime = atoi(argv[1]);

//initialize process Queues

initializeProcessQueue(&readyQ);
initializeProcessQueue(&waitQ);
initializeProcessQueue(&requests);

//initialize all CPUs to NULL
for(i = 0; i < NUMBER_OF_PROCESSORS; i++){CPU[i] = NULL;}

//read pids, arrival times and bursts to an array of processes
while((status = (readProcess(&processes[processSize])))){
	if (status == 1){processSize++;}
}//while loop

qsort(processes, processSize, sizeof (process), compareByArrival);

//error checking

if(processSize > MAX_PROCESSES || processSize == 0){return -1;}

//new lines added below
    if (argc < 2) { 
        fprintf(stderr, "time quantam not properly inserted \n"); 
        exit(1); 
    } 

 if (quantumTime <= 0) { 
      fprintf(stderr, "program specificcation not properly declared\n"); 
       exit(1); 
   } 



//next we do the actual computations and function calls to obtain information

while(1){ //while true

	moveToReadyQueue();
	tempProcessInitialization();
	moveToWaitingQueue();
	getReadyProcess();
	updateProcesses();
	increaseWorkIO();
	increaseWorkCPU();

	cpuTime = cpuTime + processesWorking();
	processesLeft = processSize - processEntering;	

	//once all processes are finished and waitQ is empty, exit while loop
	if(processesLeft == 0 && processesWorking() == 0 && waitQ.size == 0){
		break; 
	}//if loop
	simulationTime++;

}//while true loop

int totalWaitTime = 0;
int totalProcessTime = 0;

//sum all process times and all waiting times
for( i = 0; i < processSize; i++){
totalProcessTime = totalProcessTime + (processes[i].endTime - processes[i].arrivalTime);
totalWaitTime = totalWaitTime + processes[i].waitingTime;

}//for loop


//print out information
printf("--------------------------------------- rr q=%d -----------------------------\n", atoi(argv[1]));
    printf("Average waiting Time\t\t     : %.2f units\n", totalWaitTime / (double) processSize);
    printf("Average turnaround time\t\t     : %.2f units\n", totalProcessTime / (double) processSize);
    printf("Time all processes finished\t     : %d\n", simulationTime);
    printf("Average CPU utilization\t\t     : %.1f%c\n", (double) (cpuTime * 100.0) / (double) (simulationTime), (int) 37);
    printf("Number of context switches\t     : %d\n",contextSwitches); //changed because switches are actually counted now
    printf("PID(s) of last process(es) to finish ");
    for (i = 0; i < processSize; i++) {
        if (processes[i].endTime == simulationTime) {
            printf(": %d\n", processes[i].pid);
        }
    }
    printf("--------------------------------------------------------------------------\n");

     return 0; 
 


}//main method
