/*
* Family name: Williams 
* Given Name: Michael 
* Student Number: 211087798
* EECS Login: mw1992 
* 
*
* The fcfs program acts as a multi processing scheduler, that is built to run up to 4
* homogeneous CPUs in parallel.  This particular program acts on a first come first serve basis.
* This means that whichever CPU is sent to the ready queue first is processed first, like a 
* FIFO(first in first out) structure.  
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

//include helper file given
#include"sch-helpers.h"

//initialize processes and variables.

process_queue readyQ;   		//ready Queue
process_queue waitQ;			//Waiting Queue
process *CPU[NUMBER_OF_PROCESSORS];     //CPUs for the program
process *tempQ[MAX_PROCESSES + 1];      //used for sorting, contains processes going to readyQ
process processes[MAX_PROCESSES + 1];   

int i = 0;
int lastTime = 0;   
int processSize = 0; //number of processes
int tempQSize = 0;   
int cpuTime = 0;    //utilization time
int simulationTime = 0; 
int processEntering = 0;

//Function implementations begin here

//increases the work done by all CPUs running processes
void increaseWorkCPU(void){ 

	for(i = 0; i < NUMBER_OF_PROCESSORS; i++){

		//if CPU[i] is currently running, go to the next burst
		if (CPU[i] != NULL){CPU[i]->bursts[CPU[i]->currentBurst].step++;}
	}//for loop

}//increaseWorkCPU function

//increases the work done by each waiting process
void increaseWorkIO(void){

	int size = waitQ.size;
	
	for( i = 0; i < size; i++){
	process *x = waitQ.front->data; //get front process
	dequeueProcess(&waitQ);
	x->bursts[x->currentBurst].step++; //go to the next burst
	enqueueProcess(&waitQ, x);
	}//for loop	

}//increaseWorkIO

//initialize processes to the temporary starting place, tempQ
void tempProcessInitialization(void){

	while(processEntering < processSize && processes[processEntering].arrivalTime <= simulationTime) {
		tempQ[tempQSize] = &processes[processEntering]; //store process into tempQ
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

	process *x = readyQ.front->data;
	dequeueProcess(&readyQ);
	return x;

}//*getScheduleProcess


//moves processes from the waiting queue to the ready queue
void moveToReadyQueue(void){

	int size = waitQ.size;

	for(i = 0; i < size; i++){

		process *x = waitQ.front->data;
		dequeueProcess(&waitQ);
		
		//check to see if I/O bursts are finished
		//if so, move them to the tempQ
		if(x->bursts[x->currentBurst].step == x->bursts[x->currentBurst].length){
			x->currentBurst++;
			tempQ[tempQSize] = x;
			tempQSize++;
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


//get ready processes and put it in available CPUs
void getReadyProcess(void){

	qsort(tempQ, tempQSize, sizeof(process*), compareProcesses);

	//for loop to enqueue processes to readyQ
	for(i = 0; i < tempQSize; i++){
		enqueueProcess(&readyQ, tempQ[i]);
	}

	tempQSize = 0;

	//get the next scheduled process from the ready queue for each CPU not currently working
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

//move running processes that are finished their burst to waitQ
void moveToWaitingQueue(void){

for( i = 0; i < NUMBER_OF_PROCESSORS; i++){

	if(CPU[i] != NULL){

		//if current burst is finished, start next burst
		if(CPU[i]->bursts[CPU[i]->currentBurst].step == CPU[i]->bursts[CPU[i]->currentBurst].length){
			CPU[i]->currentBurst++;

			//if not finished, move to waitQ
			if(CPU[i]->currentBurst < CPU[i]->numberOfBursts){
				enqueueProcess(&waitQ, CPU[i]);
	
			} else {
				//otherwise, don't put it back into a queue
				CPU[i]->endTime = simulationTime;
				lastTime = CPU[i]->endTime;
			}//if else statement

			CPU[i] = NULL;
		}//if statement
	}//outer if statement
				
}//for loop going through each processor

}//moveToWaitingQueue

//main method for running program and computing answers
int main(void){

//initialize variables

int i = 0;
int status = 0;
int processesLeft = 0;

//initialize process Queues

initializeProcessQueue(&readyQ);
initializeProcessQueue(&waitQ);

//initialize all CPUs to NULL
for(i = 0; i < NUMBER_OF_PROCESSORS; i++){CPU[i] = NULL;}

//read pids, arrival times and bursts to an array of processes
while((status = (readProcess(&processes[processSize])))){
	if (status == 1){processSize++;}
}//while loop

qsort(processes, processSize, sizeof (process), compareByArrival);

//error checking

if(processSize > MAX_PROCESSES || processSize == 0){return -1;}

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
printf("--------------------------------------- fcfs -----------------------------\n");
    printf("Average waiting Time\t\t     : %.2f units\n", totalWaitTime / (double) processSize);
    printf("Average turnaround time\t\t     : %.2f units\n", totalProcessTime / (double) processSize);
    printf("Time all processes finished\t     : %d\n", simulationTime);
    printf("Average CPU utilization\t\t     : %.1f%c\n", (double) (cpuTime * 100.0) / (double) (simulationTime), (int) 37);
    printf("Number of context switches\t     : %d\n",0); //should be 0 right now, because we never use context switches
    printf("PID(s) of last process(es) to finish ");
    for (i = 0; i < processSize; i++) {
        if (processes[i].endTime == simulationTime) {
            printf(": %d\n", processes[i].pid);
        }
    }
    printf("--------------------------------------------------------------------------\n");

     return 0; 
 


}//main method
