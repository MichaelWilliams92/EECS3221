/*
 * alarm_cond.c
 *
 * This is an enhancement to the alarm_mutex.c program, which
 * used only a mutex to synchronize access to the shared alarm
 * list. This version adds a condition variable. The alarm
 * thread waits on this condition variable, with a timeout that
 * corresponds to the earliest timer request. If the main thread
 * enters an earlier timeout, it signals the condition variable
 * so that the alarm thread will wake up and process the earlier
 * timeout first, requeueing the later request.
 */
#include <limits.h>  //to make first insert
#include <pthread.h>
#include <time.h>
#include "errors.h"
#define DEBUG 1

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
typedef struct alarm_tag {
    struct alarm_tag    *link;
    int                 seconds;
    time_t              time;   /* seconds from EPOCH */
    char                message[64];
    int 		messageNum;
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t alarm_cond = PTHREAD_COND_INITIALIZER;
alarm_t *alarm_list = NULL;				//maybe use something similar to hold all PDthreads and look for 1 to delete or replace
time_t current_alarm = 0;

/*
 * Insert alarm entry on list, in order.
 */
void alarm_insert (alarm_t *alarm)
{
    int status;
    alarm_t **last, *next;
    pthread_t thread;



    /*
     * LOCKING PROTOCOL:
     * 
     * This routine requires that the caller have locked the
     * alarm_mutex!
     * pretty sure its been done, make sure
     */
    last = &alarm_list;  //starting at the beginning of the list
    next = *last;

	   if (next == NULL) {
        //*last = alarm;
	next = alarm;
        alarm->link = NULL;
    }

	while (next != NULL){
		if (next->messageNum > alarm->messageNum){ //now we want to stop and add at this point, check to make sure it works properly 
			alarm->link = next;
			*last = alarm;
			break;
		}
		else if (next->messageNum == alarm->messageNum){ //should replace ones with the same message number
			alarm->link = next->link;
			next->link = NULL;
			*last = alarm;
			break;
		}  // some testing to ensure that they always add where they are supposed to add the way they want them to
		   // 1 issue is that the first message made never gets deleted or shows up on the list
		   //Even when we replace a node on the list, and its shown as replaced from the list, it still prints out its message later

	last = &next->link;
	next = next->link;
	}

    /*
     * If we reached the end of the list, insert the new alarm
     * there.  ("next" is NULL, and "last" points to the link
     * field of the last item, or to the list header.)
     */
    if (next == NULL) {
        *last = alarm;
	next = alarm;
        alarm->link = NULL;
    }
#ifdef DEBUG
    printf ("[list: ");
    for (next = alarm_list; next != NULL; next = next->link)
        printf ("%d(%d)[\"%s\"] ", next->time,
            next->time - time (NULL), next->message);
    printf ("]\n");
#endif
    /*
     * Wake the alarm thread if it is not busy (that is, if
     * current_alarm is 0, signifying that it's waiting for
     * work), or if the new alarm comes before the one on
     * which the alarm thread is waiting.
     */
    if (current_alarm == 0 || alarm->time < current_alarm) {
        current_alarm = alarm->time;
        status = pthread_cond_signal (&alarm_cond);
        if (status != 0)
            err_abort (status, "Signal cond");
    }//in other words, if the newest request is at the front of the alarm list, or the alarm thread is looking for work
      //currently seesm to be based off of time instead of message ##
}

/*
 * Insert alarm entry on list, in order.
 */
void alarm_delete (alarm_t *alarm){

    int status;
    alarm_t **head, *next, *temp;

    /*
     * LOCKING PROTOCOL:
     * 
     * This routine requires that the caller have locked the
     * alarm_mutex!
     * pretty sure its been done, make sure
     */
    head = &alarm_list;  //starting at the beginning of the list
    next = temp = *head;

	//printf("Working so far");

	if(next == NULL)
		printf("empty list\n");

	while (next != NULL){

			 	if (alarm->messageNum == next->messageNum){ //should replace ones with the same message number

							if (next == (*head)){
								//printf("Node to cancel is at head\n");
								*head = next->link;
								free(next);
								free(alarm);
								printf("front Node Deleted\n");
							}else { 
								//printf("Node is not at head\n");
								temp->link = next->link;
								free(next);
								free(alarm);
								printf("Node deleted from within the list\n");	
							}

				}
					temp = next;
					next = next->link;
				}


		
				//Dosent delete last item
				//Same problem when adding first message under priority 1 as with insert
				//deleting dosent work after inserting 1st messed up input


		  // some testing to ensure that they always add where they are supposed to add the way they want them to
		   // 1 issue is that the first message made never gets deleted or shows up on the list
		   //Even when we replace a node on the list, and its shown as replaced from the list, it still prints out its message late


#ifdef DEBUG
    printf ("[list: ");
    for (next = alarm_list; next != NULL; next = next->link)
        printf ("%d(%d)[\"%s\"] ", next->time,
            next->time - time (NULL), next->message);
    printf ("]\n");
#endif

/*
     * Wake the alarm thread if it is not busy (that is, if
     * current_alarm is 0, signifying that it's waiting for
     * work), or if the new alarm comes before the one on
     * which the alarm thread is waiting.
     
    if (current_alarm == 0 || alarm->time < current_alarm) {
        current_alarm = alarm->time;
        status = pthread_cond_signal (&alarm_cond);
        if (status != 0)
            err_abort (status, "Signal cond");
    }//in other words, if the newest request is at the front of the alarm list, or the alarm thread is looking for work
      //currently seesm to be based off of time instead of message ##
*/
}
/*
 * The alarm thread's start routine.
 */


void *alarm_thread (void *arg) //this thread is supposed to check alarm requests in the alarm list whenever the alarm list has been changed
{
    alarm_t *alarm;
    struct timespec cond_time;
    time_t now;
    int status, expired;

    /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits. Lock the mutex
     * at the start -- it will be unlocked during condition
     * waits, so the main thread can insert alarms.
     */
    status = pthread_mutex_lock (&alarm_mutex);
    if (status != 0)
        err_abort (status, "Lock mutex");
    while (1) {
        /*
         * If the alarm list is empty, wait until an alarm is
         * added. Setting current_alarm to 0 informs the insert
         * routine that the thread is not busy.
         */
        current_alarm = 0;
        while (alarm_list == NULL) {
            status = pthread_cond_wait (&alarm_cond, &alarm_mutex);
            if (status != 0)
                err_abort (status, "Wait on cond");
            }
	
        alarm = alarm_list;
        alarm_list = alarm->link;
        now = time (NULL);
        expired = 0;
        if (alarm->time > now) {
#ifdef DEBUG
            printf ("[waiting: %d(%d)\"%s\"]\n", alarm->time,
                alarm->time - time (NULL), alarm->message);
#endif
            cond_time.tv_sec = alarm->time;
            cond_time.tv_nsec = 0;
            current_alarm = alarm->time;
            while (current_alarm == alarm->time) {
                status = pthread_cond_timedwait (
                    &alarm_cond, &alarm_mutex, &cond_time);
                if (status == ETIMEDOUT) {
                    expired = 1;
                    break;
                }
                if (status != 0)
                    err_abort (status, "Cond timedwait");
            }
            if (!expired)
                alarm_insert (alarm);

		//if(alarm->message == "")
		//	printf("delete this alarm");
		//	free (alarm);

		//if alarm line starts with Cancel keyword, run remove function and then free this alarm AFTER sending last message
        } else
            expired = 1;
        if (expired) {
            printf ("(%d) %s\n", alarm->seconds, alarm->message);
            free (alarm);
        }
    }
}

void *periodic_display_thread (void *alarm) {

alarm_t *next_alarm2 = alarm;
int status = 1;
int delete = 0;//while 1, this means that the alarm is in the list still

 alarm_t **head, *next, *temp;
 head = &alarm_list;  //starting at the beginning of the list
 next = temp = *head;

//printf("Create PD thread\n");
//printf("%d\n",next_alarm->seconds);

    /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits.
     */
    while (status == 1) {// while unchanged

            sleep(next_alarm2->seconds);
		head = &alarm_list;  //starting at the beginning of the list
 		next = temp = *head;
		delete = 0;

	
	//if(next == NULL){
	//	printf("empty list\n"); //if its empty, that means that the alarm associated with the thread is not there so terminate
	//	status == 0;
	//}


	while (next != NULL){
				//printf("Gets past while loop");

			 	if (next_alarm2->messageNum == next->messageNum){//if messageNum is found in list
								//printf("Message Num matches");
								delete = 1;//if delete == 1, this means that the alarm was not deleted as it was found 
							if(! (next_alarm2->time == next->time)){ //if creation time is not equal,but the messageNum is
											//this means replace occurs
								//printf("%d %d", next_alarm2->time, next->time);
								printf("Alarm changed at %d: %d Message(%d) %s\n", time(NULL), next->seconds, next->messageNum, next->message);
								status = 0; 
							
							}//if MessageNum is equal but creation time isnt	
								 

							

				}
					temp = next;
					next = next->link;
				}//end  of while loop checking through the alarm list

		if (delete == 0){ //if alarm was never found
			printf("Display thread exiting at %d\n", time(NULL));
			status = 0;
			}

		//now check if alarm is still in list.  If its not in the list, its been deleted, so we stop this thread.  
		//if we find the Message number but with different info, we remove the thread once again
		//only error is when new request is the same as the one it is replacing.( wont be because time will be different)


	
//If list reaches null, item was not found, so terminate thread.
	//printf(status);
	if(status == 1){
            printf ("Alarm displayed at %d: %d Message(%d) %s\n", (time(NULL) + next_alarm2->seconds), next_alarm2->seconds, next_alarm2->messageNum, next_alarm2->message);
	}
	

        }//while loop

}//This thread will keep printing out statements repeatedly

//if i can send the alarm info here to check to see if there is another one with an identical messageNum, i can effectively check if this thread needs to be terminated.  
//If it does, i simply print out the correct statement and then stop the loop and let the thread terminate itself.
//checking through the alarm list, if this messageNum isnt in the alarm list, its been deleted, so end the thread
//if it is in the list, but any data has changed, end it 


//loop should stop if any change is made OR IF DELETED
//when deleted, thread should be freed, after processing the statement required
//when changed, we must look for a thread containing same info.  If same info exists, print change statement then change to new 1
//maybe create a PD_thread list, and check for existing lists in the same manner as the the insert/delete methods.  once we find 1, we could just replace it, then free it



int main (int argc, char *argv[])
{
    int status, status2;
    char line[128];
    alarm_t *alarm;
    pthread_t thread, thread2;

  //  status = pthread_create (
   //     &thread, NULL, alarm_thread, NULL);  //creates thread and executes the alarm_thread function
   // if (status != 0)
    //    err_abort (status, "Create alarm thread");

    while (1) {
        printf ("Alarm> ");
        if (fgets (line, sizeof (line), stdin) == NULL) exit (0);
        if (strlen (line) <= 1) continue;
        alarm = (alarm_t*)malloc (sizeof (alarm_t));
        if (alarm == NULL)
            errno_abort ("Allocate alarm");

        /*
         * Parse input line into seconds (%d) and a message
         * (%64[^\n]), consisting of up to 64 characters
         * separated from the seconds by whitespace.
         

        if (sscanf (line, "%d %64[^\n]", 
            &alarm->seconds, alarm->message) < 2) {
            fprintf (stderr, "Bad command\n");
            free (alarm);
        }
	*/

	//parse input line into 1 of the 2 formats below.  If neithor format is matched, then it is a bad command.
	
	if (sscanf (line, "Cancel: Message(%d)", 
            &alarm->messageNum) == 1) {
	   // printf("Cancel message picked up and working\n");
           // free (alarm); might have to free it in the alarm thread, not here
	  printf ("Alarm request Received at %d: %s\n", time(NULL), line); //alarm recieved message

	  status = pthread_mutex_lock (&alarm_mutex);
          	 if (status != 0)
          	 err_abort (status, "Lock mutex");
          	// alarm->time = time(NULL) + 999;
          	 // printf("SHould delete this message num now\n");
		alarm_delete(alarm);
          	 status = pthread_mutex_unlock (&alarm_mutex);
          	 if (status != 0)
         	  err_abort (status, "Unlock mutex");	
		
		
        }
	//do cancel function
 

        

	else if (sscanf (line, "%d Message(%d) %64[^\n]",    //test this part to ensure it only accepts up to 128 characters and truncates the rest
		&alarm->seconds, &alarm->messageNum, alarm->message) == 3){
		//printf("Add message picked up and working\n");  
		//free(alarm);   //now add alarm 

		printf ("Alarm request Received at %d: %s\n", time(NULL), line); //alarm recieved message
         
         	  status = pthread_mutex_lock (&alarm_mutex);
          	 if (status != 0)
          	 err_abort (status, "Lock mutex");

          	 alarm->time = time (NULL) + alarm->seconds;
          	  /*
          	   * Insert the new alarm into the list of alarms,            
          	   * sorted by expiration time.
               * 
          	   */
		//create thread

			status2 = pthread_create (&thread2, NULL, periodic_display_thread, alarm);  //creates thread and executes the PD_thread function
    		if (status2 != 0)
    		    err_abort (status2, "Create PD thread");
			status2 = 0;
		
          	 alarm_insert (alarm);  //check this function

          	 status = pthread_mutex_unlock (&alarm_mutex);
          	 if (status != 0)
         	  err_abort (status, "Unlock mutex");
	}	
	
	else {
            fprintf (stderr, "Bad command\n");
        }

	//Make sure the format of each request is consistent with the specs given in pdf
	//MAke message length at most 128 characters, else truncate it
	//alarm requests must be placed in the alarm list in the order of their message number
	//If message number dosent exist in current alarm list, just insert it normally
	//If it does exist, replace it with the new message info
	//immediately after recieving alarm request, print info (use solution in assignment 2)
	
	// else {
         //   status = pthread_mutex_lock (&alarm_mutex);
          //  if (status != 0)
           //     err_abort (status, "Lock mutex");
            //alarm->time = time (NULL) + alarm->seconds;
            /*
             * Insert the new alarm into the list of alarms,            //set up message types to do corresponding actions
             * sorted by expiration time.
             */
            //alarm_insert (alarm);
            //status = pthread_mutex_unlock (&alarm_mutex);
            //if (status != 0)
             //   err_abort (status, "Unlock mutex");
        //}
    }
}
