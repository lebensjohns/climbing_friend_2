/*
\\ Sources used:
// 	https://c-for-dummies.com/blog/?p=3379
\\ 	https://www.youtube.com/watch?v=nVESQQg-Oiw
//  https://stackoverflow.com/questions/8223742/how-to-pass-multiple-parameters-to-a-thread-in-c
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <pthread.h>
#include <unistd.h>

#define TRUE 1

//    This code uses a producer-consumer pipeline with unbounded buffers to process some text from
//    std in and sends the reformatted msg to stdout. It uses 4 threads, 3 buffers and mutexes to 
//    accomplish all this.

// Initialize the mutex
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct function_1{
	int end_loop;
	char* buff_1;
};

//This function just take in the input line by line and stores each in the buffer until a line reads DONE\n
void *input_thread(void *void_ptr){ 
	//cast void pointer to struct pointer
	struct function_1 *fun1_ptr = (struct function_1*)void_ptr;      
	//1000 is big enough according to the instructions
	size_t length = 1000;

	//pthread_mutex_lock(&mutex);

	//This grabs inout line by line including the trailing \n
	fgets(fun1_ptr->buff_1, length, stdin);

	//pthread_mutex_unlock(&mutex);

	fflush(stdin);
	if (strcmp(fun1_ptr->buff_1, "DONE\n") == 0){
		//This will stop the DONE string from being printed
		strcpy(fun1_ptr->buff_1, "\0");
		//Setting this to 1 turns of while loop in main
		fun1_ptr->end_loop = 1;
	}
	return NULL;
}

struct function_2{
	char* buff_1;
	char* buff_2;
};

//This function removes the trailing \n and replaces with a space char
void *line_separator_thread(void *void_ptr){
	//cast void pointer to struct pointer
	struct function_2 *fun2_ptr = (struct function_2*)void_ptr;

	int len = strlen(fun2_ptr->buff_1);

	len = len - 1;

	//pthread_mutex_lock(&mutex);

	//this replaces trailing \n with a space
	//(fun2_ptr->buff_1)[(strlen(fun2_ptr->buff_1) - 1)] = ' ';

	//(fun2_ptr->buff_1)[(len - 1)] = ' ';

	//This cause the core dump
	(fun2_ptr->buff_1)[len] = ' ';

	//This prints with no problems
	printf("Contents of buff1: %c\n", (fun2_ptr->buff_1)[len]);

	//This works as expected
	strcpy((fun2_ptr->buff_2), (fun2_ptr->buff_1));

	//pthread_mutex_unlock(&mutex);

	return NULL;
}

struct function_3{
	char* buff_2;
	char* buff_3;
};

//this function replaces ++ with ^ and then copies output to the third buffer
void *plus_sign_thread(char* buff2, char* buff3){
	//these two indexes allow two chars to become one
	int b2 = 0;
	int b3 = 0;

	//this needs to iterate through entire line
	while (b2 < strlen(buff2)){
		//this is where the work is being done
		if ((buff2[b2] == '+') && (buff2[b2+1] == '+')){
			buff3[b3] = '^';
			b2++;
		}
		else{
			buff3[b3] = buff2[b2];
		}
		//b2 is for buffer2 and b3 is buffer3
		b2++;
		b3++;
	}
	return NULL;
}

struct function_4{
	char* the_goods;
	char* buff_3;
};

//This function hollds a fake buffer that will only print 80 chars at a time
void *output_thread(char* buff3, char* print_str){
	//p is the index of the print_str buffer and i is the index of buffer3
	int p = 0;
	int i = 0;
	//If the print string is partially full then get the index of the trailing \0
	if (print_str[0] != '\0'){
		p = strlen(print_str);
	}
	//This iterates through each line filling the print_str
	while (i < strlen(buff3)){
		print_str[p] = buff3[i];
		i++;
		p++;
		//This is a safeguard to make sure the string always has a trailing \0
		print_str[p] = '\0';
		//when we have 80 chars it's time to print
		if (p == 80){
			//reset iterator
			p = 0;
			printf("%s", print_str);
			fflush(stdout);
			//amother safeguard that is less work than memset but accomplishes the same task
			print_str[0] = '\0';
		}
	}
	return 0;
}

int main(){
	int iter = 0;
	//printstring need only be 81 in size but kept having issues when it was that size
	char print_string[90];
	//allocate memory for buffers
	//These are all dynamically allocated so that I will know if they fail
	char **buffer1 = malloc(10*sizeof(char*));
	char **buffer2 = malloc(10*sizeof(char*));
	char **buffer3 = malloc(10*sizeof(char*));
	if((buffer1 == NULL) || (buffer2 == NULL) || (buffer3 == NULL)){
		fprintf(stderr, "Error: Failed to allocate memory for buffer array!\n");
	}
	int i = 0;
	//each one of these holds an input line
	while (i < 10){
		buffer1[i] = malloc(1001*sizeof(char));
		buffer2[i] = malloc(1001*sizeof(char));
		buffer3[i] = malloc(1001*sizeof(char));
		if((buffer1[i] == NULL) || (buffer2[i] == NULL) || (buffer3[i] == NULL)){
			fprintf(stderr, "Error: Failed to allocate memory for buffers!\n");
		}
		i++;
	}
	//create the struct pointer and allocate its memory
	struct function_1 *f1 = malloc(sizeof(struct function_1));
	struct function_2 *f2 = malloc(sizeof(struct function_2));
	struct function_3 *f3 = malloc(sizeof(struct function_3));
	struct function_4 *f4 = malloc(sizeof(struct function_4));

	//if malloc fails
	if((f1 == NULL) || (f2 == NULL) || (f3 == NULL) || (f4 == NULL)){
		fprintf(stderr, "Error: Failed to allocate memory for struct pointers!\n");
	}

	//this controls main while loop
	f1->end_loop = 0;

	//All the pthreads made here
	pthread_t pthread_input;
	pthread_t pthread_line;
	//pthread_t pthread_plus;
	//pthread_t pthread_output;

	//set print buffer up for doing work
	memset(print_string, '\0', 90);
		
	//main loop for doing work
	while(f1->end_loop == 0){
		//assign the struct char pointer buffer1
		f1->buff_1 = buffer1[iter];

		pthread_create(&pthread_input, NULL, input_thread, f1);

		pthread_join(pthread_input, NULL);

		//assign funtion 2 struct char pointers to the correct buffers
		f2->buff_1 = buffer1[iter];
		f2->buff_2 = buffer2[iter];

		pthread_create(&pthread_line, NULL, line_separator_thread, f2);

		pthread_join(pthread_line, NULL);

		//line_separator_thread(buffer1[iter], buffer2[iter]);

		//printf("%s", buffer2[iter]);

		//plus_sign_thread(buffer2[iter], buffer3[iter]);

		//printf("%s", buffer3[iter]);

		//output_thread(buffer3[iter], print_string);

		//This iter allows never-ending cycle through the arrays that each hold 10 lines max
		iter++;
		//go back to first index when at the end
		if (iter == 10){
			iter = 0;
		}
	}

	//de-allocate all those buffers
	i = 0;
	while (i < 10){
		free(buffer1[i]);
		free(buffer2[i]);
		free(buffer3[i]);
		i++;
	}
	free(buffer1);
	free(buffer2);
	free(buffer3);

	//free the struct pointers
	free(f1);
	free(f2);
	free(f3);
	free(f4);

	return EXIT_SUCCESS;
}