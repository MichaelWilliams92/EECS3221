#include <stdlib.h>
#include <stdio.h> 
#include <sys/types.h>

#define READ 0
#define WRITE 1



int main(int argc, char *argv[]){
	
	FILE *fr;
	pid_t  pid;
	int FileNum = 0;
	float Maximum = 0;
	float Minimum = 0;
	
	/*Pipes*/
	int sumpipe[2];
	int diffpipe[2];
	
	for (FileNum = 1; FileNum < argc; FileNum++){/*File loop*/
	
		
		
	if (pipe(sumpipe) == -1 || pipe(diffpipe) == -1) {/*Pipe creation*/
		fprintf(stderr,"Pipe failed");
		return 1;
	}
	
	
	pid = fork();
	
	
	if (pid < 0){ 
		
		fprintf(stderr, "Fork Failed");
		return 1;
		
	}else if (pid == 0){/*Child pricess*/
		
		float num;
		float max = 0;
		float min = 0;
		float sum;
		float diff;
		
		
		fr = fopen (argv[FileNum], "r");
		
		if (fr == NULL) {
			printf("%d\n", argv[FileNum]);
			continue;
		}
		
		while(fscanf(fr, "%f", &num) > 0){
			
			if (max == 0){
				max = num;
			}else if (num > max){
				max = num;
				
			}
			
			if (min == 0){
				min = num;
			}else if (num < min){
				min = num;
				
			}
			
		}
		
		sum = max + min;
		diff = max - min;
		
		close(sumpipe[READ]);
		write(sumpipe[WRITE],&sum,sizeof(sum));
		close(sumpipe[WRITE]);
		
		close(diffpipe[READ]);
		write(diffpipe[WRITE],&diff,sizeof(diff));
		close(diffpipe[WRITE]);

		fclose(fr);
	
		exit(1);
		
		}else{/*Parent process*/
			
			int returnStatus;
			float max;
			float min;
			float sum;
			float diff;
			
			wait(NULL);
			
			close(sumpipe[WRITE]);
			read(sumpipe[READ],&sum,sizeof(sum));
			close(sumpipe[READ]);
			
			close(diffpipe[WRITE]);
			read(diffpipe[READ],&diff,sizeof(diff));
			close(diffpipe[READ]);
			
			max = (sum + diff)/2;
			min = (sum - diff)/2;
			
			if (Maximum == 0){
				Maximum = max;
			}else if (max > Maximum){
				Maximum = max;
				
			}
			
			if (Minimum == 0){
				Minimum = min;
			}else if (min < Minimum){
				Minimum = min;
				
			}
			
			printf("%s SUM=%0.5f DIF=%0.5f MIN=%0.5f MAX=%0.5f\n",argv[FileNum], sum, diff, min, max);		

		}

	}
	printf("MINIMUM=%0.5f MAXIMUM=%0.5f\n", Minimum, Maximum);
	return 0;
}

