#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 

pthread_t thread_id, thread_id_2; 

void *f1(void *args) 
{ 
    for(int i = 0; i < 5; i++){
        sleep(1); 
        printf("Print from thread 1\n"); 
    } 
    return (void*)2;
} 


void *f2(void *args) 
{ 
    for(int i = 0; i < 5; i++){
        sleep(1); 
        printf("Print from thread 2\n"); 
    } 
    return (void*)3;
} 

int main() 
{ 
	printf("Before Thread\n"); 
	pthread_create(&thread_id, NULL, f1, NULL); 
	pthread_create(&thread_id_2, NULL, f2, NULL);
    int a, b; 
	pthread_join(thread_id, (void**)&a);
	pthread_join(thread_id_2, (void**)&b);
     
	printf("After Thread\n");
    printf("Sum: %d\n", a+b);
	exit(0); 
}
