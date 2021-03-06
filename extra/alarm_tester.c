/*
 * alarm_mutex.c
 *
 * This is an enhancement to the alarm_thread.c program, which
 * created an "alarm thread" for each alarm command. This new
 * version uses a single alarm thread, which reads the next
 * entry in a list. The main thread places new requests onto the
 * list, in order of absolute expiration time. The list is
 * protected by a mutex, and the alarm thread sleeps for at
 * least 1 second, each iteration, to ensure that the main
 * thread can lock the mutex to add new work to the list.
 */
#include <pthread.h>
#include <time.h>
#include "errors.h"
#include <ctype.h>

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
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_t *alarm_list = NULL;
///////////////
void *rings (void *arg); 
void *snooze(char *message, int seconds);
///////////////

/*
 * The alarm thread's start routine.
 */
void *alarm_thread (void *arg)
{
    alarm_t *alarm;
    int sleep_time;
    time_t now;
    int status;
    int i;

    /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits.
     */
    while (1) {
        status = pthread_mutex_lock (&alarm_mutex);
        if (status != 0)
            err_abort (status, "Lock mutex");
        alarm = alarm_list;

        /*
         * If the alarm list is empty, wait for one second. This
         * allows the main thread to run, and read another
         * command. If the list is not empty, remove the first
         * item. Compute the number of seconds to wait -- if the
         * result is less than 0 (the time has passed), then set
         * the sleep_time to 0.
         */
        
        if (alarm == NULL)
            sleep_time = 1;
        else {
        	///////////////
            printf("\nAlarm Retrieved at %d: %d %s\n", alarm->time, 
            	alarm->seconds, alarm->message);
        	///////////////
            alarm_list = alarm->link;
            now = time (NULL);
            if (alarm->time <= now)
                sleep_time = 0;
            else
                sleep_time = alarm->time - now;
#ifdef DEBUG
            printf ("[waiting: %d(%d)\"%s\"]\n", alarm->time,
                sleep_time, alarm->message);
#endif
            }

        /*
         * Unlock the mutex before waiting, so that the main
         * thread can lock it to insert a new alarm request. If
         * the sleep_time is 0, then call sched_yield, giving
         * the main thread a chance to run if it has been
         * readied by user input, without delaying the message
         * if there's no input.
         */
        status = pthread_mutex_unlock (&alarm_mutex);
        if (status != 0)
            err_abort (status, "Unlock mutex");
        /*if (sleep_time > 0)
        	 sleep (sleep_time);*/
        if (sleep_time > 0 && alarm != NULL){
        	///////////////
        	for(i = 0 ; i < sleep_time; i++) {
        		sleep (1);
        		printf("Alarm: >: %d %s\n", alarm->seconds, 
        			alarm->message);
        	}
        	///////////////
        } else
            sched_yield ();

        /*
         * If a timer expired, print the message and free the
         * structure.
         */
        /*if (alarm != NULL) {
            printf ("(%d) %s\n", alarm->seconds, alarm->message);
            free (alarm);
        }*/
        
        ///////////////
         if (alarm != NULL) {
            printf ("Alarm Expired at %d: %d %s\n", alarm->time, 
            	alarm->seconds, alarm->message);
            free (alarm);
            pthread_t thread2;
            pthread_create(&thread2,NULL,rings,(void *)alarm);
        }
        ///////////////
    }
}

/*rings every 3 seconds*/

///////////////
void *rings (void *arg)
{
    time_t now;
    struct tm* tm_now;
    alarm_t *newalarm = (alarm_t *)arg;
    int count = 0;
    //char snoozeCommand[128];
    
    while(count < 8)
    {
	time(&now); 
	tm_now = localtime(&now);    
	/*printf ("\"Snooze time\":(%d) Message: %s Alarm time & Date: 
	%s \n", newalarm->seconds, newalarm->message, asctime(tm_now));*/
	printf("\n");	
	printf("Number of rings: %d\n", count);
	//printf("Alarm time: %d secs \n", newalarm ->seconds);
	printf("Message: %s\n", newalarm->message);
	printf("Alarm time and Date: %s", asctime(tm_now));
        sleep(newalarm->seconds);
	count = count + 1;
	
	/*if (fgets (snoozeCommand, sizeof (snoozeCommand), stdin) == 
		"snooze"){
		
		snooze(newalarm->message, newalarm->seconds);
		}*/ 
    }
    
}
///////////////

void *snooze(char *message, int seconds){
	

}

int main (int argc, char *argv[])
{
    int status;
    char line[128];
    alarm_t *alarm, **last, *next;
    pthread_t thread;
    int final_sec;
    
    char snoozeCommand[128];
    int i;
    

    status = pthread_create (
        &thread, NULL, alarm_thread, NULL);
    if (status != 0)
        err_abort (status, "Create alarm thread");
    while (1) {
        printf ("alarm> ");
        if (fgets (line, sizeof (line), stdin) == NULL) exit (0);
        
        if (strlen (line) <= 1) continue;
        alarm = (alarm_t*)malloc (sizeof (alarm_t));
        if (alarm == NULL)
            errno_abort ("Allocate alarm");
        ////////////////
        time(&alarm->time);
        printf ("Alarm Received at %d: %s", alarm->time, line);
        ///////////////
        
        /*
         * Parse input line into seconds (%d) and a message
         * (%64[^\n]), consisting of up to 64 characters
         * separated from the seconds by whitespace.
         */
        strcpy(snoozeCommand, "snooze\n");
         int input = sscanf (line, "%d %64[^\n]", 
            &alarm->seconds, alarm->message);

        /*for(i = 0; i < strlen(line); i++) {
        	printf("$%c$", *(line + i));
        }*/
	
        if(strcmp(line, snoozeCommand) == 0) {
        	
        	printf("SNOOZE\n");
        	
        } 

	
	else if(alarm->seconds < '0' || alarm->seconds > '9')
	{
	//	fprintf (stderr, "Please use digits for the number of seconds\n");				
//		free(alarm);	

		/*while(alarm->seconds > '0' || alarm->seconds < '9')
		{
			alarm->seconds = alarm->seconds*10;
			if(alarm->seconds < '0' || alarm->seconds > '9')
			{
				fprintf (stderr, "Please use digits for the number of seconds\n");				
				free(alarm);		
			}		
	
		}*/
	}
		
	
	else if (input < 2) 
	{
         if(sscanf(line, "%d") != 1){
                fprintf (stderr, "inproper digit placed\n");
                free (alarm);
            }else{
              fprintf (stderr, "Bad command\n");//aybe replace with message error, since thats the only kind of error we can have if not the integer error
              free (alarm);
		}
	 }else {
            status = pthread_mutex_lock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Lock mutex");
            alarm->time = time (NULL) + alarm->seconds;

            /*
             * Insert the new alarm into the list of alarms,
             * sorted by expiration time.
             */
            last = &alarm_list;
            next = *last;
            while (next != NULL) {
                if (next->time >= alarm->time) {
                    alarm->link = next;
                    *last = alarm;
                    break;
                }
                last = &next->link;
                next = next->link;
            }
            /*
             * If we reached the end of the list, insert the new
             * alarm there. ("next" is NULL, and "last" points
             * to the link field of the last item, or to the
             * list header).
             */
            if (next == NULL) {
                *last = alarm;
                alarm->link = NULL;
            }
#ifdef DEBUG
            printf ("[list: ");
            for (next = alarm_list; next != NULL; next = next->link)
                printf ("%d(%d)[\"%s\"] ", next->time,
                    next->time - time (NULL), next->message);
            printf ("]\n");
#endif
            status = pthread_mutex_unlock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");
        }
    }
}

