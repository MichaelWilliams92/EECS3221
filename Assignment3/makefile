.PROGRAM : clean
alarm_mutex : New_Alarm_Cond.c
	cc -o alarm_mutex New_Alarm_Cond.c -D_POSIX_PTHREAD_SEMANTICS -lpthread
	./alarm_mutex
	clean :
	-rm *.o$(objects)
