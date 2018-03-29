/*
* Family name: Williams 
* Given Name: Michael 
* Student Number: 211087798
* EECS Login: mw1992 
* 
*
*The Feedback Queue is an algorithm in which all the processes are allocated to different queues based on the CPU bursts of the process. This algorithm allows a *process to move between queues depending on what should be executed first.  It prioritizes lowers bursts, or shorter jobs into the higher queues so they are *finished first while the higher bursts, or longer jobs are placed in lower queues so they are finished last.  The FBQ implemented in our program consists of *three different queues. The first two queues use a RR type scheduling algorithm, while the third uses a normal FCFS. The two RR queues use a quantum time that *determines which of the processes is short enough for the first queue, too long for the first queue but short enough for the second queue, or just too long for *either. More information is available in the report. 
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

#define MAXVAL 0xffffffff

//initialize processes and variables.

process_queue readyQ;   		//ready Queue
process_queue waitQ;			//Waiting Queue
process_queue initialRR;		//First round robin Queue, ie the top priority level
process_queue secondaryRR;		//Second round robin Queue, ie the second priority level

process *CPU[NUMBER_OF_PROCESSORS];     //CPUs for the program
process *tempQ[MAX_PROCESSES + 1];      //used for some sorting and contains processes going to readyQ
process processes[MAX_PROCESSES + 1];   //used for storing processes of the file
process *initialRR_tempQ[NUMBER_OF_PROCESSORS]; //temporary processes going to initialRR queue
process *secondaryRR_tempQ[NUMBER_OF_PROCESSORS]; //temporary processes going to secondaryRR queue
process *promotedToReadyQ[NUMBER_OF_PROCESSORS]; //temporary processes being promoted to the ReadyQ
process *promotedToReadyQ_2[NUMBER_OF_PROCESSORS]; //temporary processes being promoted to the ReadyQ

int i = 0;
int lastTime = 0;   
int processSize = 0; //number of processes
int tempQSize = 0;   
int cpuTime = 0;    //utilization time
int simulationTime = 0; 
int processEntering = 0;
int contextSwitches = 0;
int level1QuantumTime = 0; 
int level2QuantumTime = 0;

//Function implementations begin here

//increases the work done by all CPUs running processes
void increaseWorkCPU(void){ 

	for(i = 0; i < NUMBER_OF_PROCESSORS; i++){  //traverse through all processors

		//if CPU[i] is currently running, take a step and reduce quantum time remaining
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

		tempQ[tempQSize]->currentQueue = 0; //initialize first level for process

		tempQ[tempQSize]->quantumRemaining = level1QuantumTime; //initialize the quantum time in which the process will run
		tempQSize++;
		processEntering++;
	}//while loop

}//tempProcessInitialization


//get the next process for the cpu
process *getScheduledProcess(void){

	process *x;

    //if processes available in the readyQ
    if (readyQ.size > 0) {
        x = readyQ.front->data;
        dequeueProcess(&readyQ);
        return x;

    //else if processes available in the initialRR
    } else if (initialRR.size > 0) {
        x = initialRR.front->data;
        dequeueProcess(&initialRR);
        return x;

    //else if processes available in the secondaryRR
    } else if (secondaryRR.size > 0) {
        x = secondaryRR.front->data;
        dequeueProcess(&secondaryRR);
        return x;

    } else {
        return NULL;
    }

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
		
            if (x->bursts[x->currentBurst].length < level1QuantumTime) { 
                x->quantumRemaining = MAXVAL;  
            } else if (x->bursts[x->currentBurst].length < level2QuantumTime) { 
               x->quantumRemaining = MAXVAL; 
            }

	    //set Quantum times
            if (x->currentQueue == 0) {
                x->quantumRemaining = level1QuantumTime;
            } else if (x->currentQueue == 1) {
                x->quantumRemaining = level2QuantumTime;
 		contextSwitches++;
            } else {
                x->quantumRemaining = MAXVAL;
            }

            tempQ[tempQSize++] = x;
        } else {
            enqueueProcess(&waitQ, x);
        }

}

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

 int a, b, c, d, x;
i = 0;
a = 0;
b = 0;
c = 0;
d = 0;
x = 0;

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

               //process dosen't finish within initialRR 
            } else if (CPU[i]->quantumRemaining == 0 && CPU[i]->currentQueue == 0) {

                CPU[i]->quantumRemaining = level2QuantumTime;
                CPU[i]->currentQueue = 1;
                initialRR_tempQ[a] = CPU[i];
                contextSwitches++;
                a++;
                CPU[i] = NULL; //free the CPU for another process

                ////process dosen't finish within secondaryRR 
            } else if (CPU[i]->quantumRemaining == 0 && CPU[i]->currentQueue == 1) {
          
                CPU[i]->quantumRemaining = MAXVAL;
                CPU[i]->currentQueue = 2;
                contextSwitches++;
                secondaryRR_tempQ[b] = CPU[i];
                b++;
                CPU[i] = NULL; //free the cpu for another process

            } else if (CPU[i]->bursts[CPU[i]->currentBurst].length < level1QuantumTime) {
                CPU[i]->currentQueue = 0; // priority level dosent change
                promotedToReadyQ[c] = CPU[i];
                c++;
                CPU[i] = NULL;

           } else if (CPU[i]->bursts[CPU[i]->currentBurst].length < level2QuantumTime) {
               CPU[i]->currentQueue = 0; // priority level dosent change
                promotedToReadyQ_2[d] = CPU[i];
                d++;
                CPU[i] = NULL;
            }
	
	}//outer if statement
				
}//add processes to their appropriate queues

  qsort(initialRR_tempQ, a, sizeof (process*), compareProcesses);
    for (x = 0; x < a; x++) {
        enqueueProcess(&initialRR, initialRR_tempQ[x]);
    }
    qsort(secondaryRR_tempQ, b, sizeof (process*), compareProcesses);
    for (x = 0; x < b; x++) {
        enqueueProcess(&secondaryRR, secondaryRR_tempQ[x]);
    }
    qsort(promotedToReadyQ, c, sizeof (process*), compareProcesses);
    for (x = 0; x < c; x++) {
        enqueueProcess(&readyQ, promotedToReadyQ[x]);
    }
    qsort(promotedToReadyQ_2, d, sizeof (process*), compareProcesses);
    for (x = 0; x < d; x++) {
        enqueueProcess(&readyQ, promotedToReadyQ_2[x]);
    }

}//moveToWaitingQueue


//main method for running program and computing answers
int main(int argc, char **argv){

//initialize variables

int i = 0;
int status = 0;
int processesLeft = 0;
level1QuantumTime = atoi(argv[1]);
level2QuantumTime = atoi(argv[2]);

//initialize process Queues

initializeProcessQueue(&readyQ);
initializeProcessQueue(&waitQ);
initializeProcessQueue(&initialRR);
initializeProcessQueue(&secondaryRR);

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
   if (argc < 3) {
        fprintf(stderr, "Please enter a time quantums . . . . \n");
        exit(1);
    }

   if (level1QuantumTime <= 0 || level2QuantumTime <= 0) {
        fprintf(stderr, "Error! program usage rr < positive time quantum> < data file . . .\n");
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
for( i = 1; i < processSize; i++){
totalProcessTime = totalProcessTime + (processes[i ].endTime - processes[i ].arrivalTime);
totalWaitTime = totalWaitTime + processes[i ].waitingTime;

}//for loop


//print out information
printf("----------------------------------- fbq q1=%d q2=%d -----------------------------\n", atoi(argv[1]),  atoi(argv[2]));
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

