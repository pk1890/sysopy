#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define WAITING 0
#define RIDING 1
// #define QUEUE_DEBUG_MODE
// #define THREAD_DEBUG_MODE
long get_timestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec;
}                                                                                                                                                       
                                                                                                                                                                                   
void exitError(char* msg){                                                                                                                                                         
    printf(msg);                                                                                                                                                                   
    exit(1);                                                                                                                                                                       
}                                                                                                                                                                                  
                                                                                                                                                                                   
void exitErrno(char* msg){                                                                                                                                                         
    perror(msg);                                                                                                                                                                   
    exit(1);                                                                                                                                                                       
}                     

typedef struct queue
{
    int *buff;
    size_t first;
    size_t last;
    size_t size;
}queue;


int *client_state;
int *carts_free_space;

queue *clients_queue;
queue *carts_queue;

int CLIENT_COUNT, CART_COUNT, CART_CAPACITY, RIDES_COUNT;
int the_chosen_one = -1;
int has_client_entered, has_client_exited;
int is_button_pressed;

pthread_t *clients;
pthread_t *carts;
pthread_mutex_t client_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t carts_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cart_to_platform_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t button_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t entering_to_cart_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t exiting_from_cart_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t is_this_crate_first = PTHREAD_COND_INITIALIZER;
pthread_cond_t is_button_pressed_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_client_entered_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_client_exited_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_client_enter_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_client_exited_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_someone_been_chosen_cond = PTHREAD_COND_INITIALIZER;

sem_t active_carts_sem;

queue *init_queue(size_t size){
    queue *q = malloc(sizeof(queue));
    if(q == NULL) exitErrno("Error in allocating queue");
    q->buff = malloc(sizeof(int) * 5*(size+1));
    if(q->buff == NULL) exitErrno("Error in allocating queue buff");
    q->first = 0;
    q->last = 0;
    q->size = 5*size+1;
    return q;
}

void add_to_queue(queue *q, int item){
    if((q->last + 1) % q->size == q->first) exitError("Trying to put item in full queue");

    q->buff[q->last] = item;
    q->last = (q->last + 1) % q->size;
    #ifdef QUEUE_DEBUG_MODE
    printf("\033[1;34m Adding %d \033[0m\n", item );
    #endif
}

int get_from_queue(queue *q){
    if(q->first == q->last) exitError("Trying to get item from empty queue");
    int res = q->buff[q->first];
    q->first = (q->first+1) % q->size;
    #ifdef QUEUE_DEBUG_MODE
    printf("\033[1;32m Getting %d \033[0m\n", res );
    #endif
    return res;
}

int seek_from_queue(queue *q){
    if(q->first == q->last) exitError("Trying to seek item from empty queue");
    return q->buff[q->first];
}

void* cart_thread(void* args){
    // printf("MLEKO\n");
    int ID = *(int*)args;
    int *passenger = malloc(CART_CAPACITY * sizeof(int));
    int count = 0;
    int curr_passenger_id;
    for(int i = 0; i < RIDES_COUNT+1; i++){
        pthread_mutex_lock(&cart_to_platform_mutex);
        add_to_queue(carts_queue, ID);
        printf("\033[1;33m<%ld>[CART %d ]: AM I THE FIRST?\n\033[0m ",get_timestamp(), ID);
        while(seek_from_queue(carts_queue) != ID){
            printf("\033[1;33m<%ld>[CART %d ]: Waiting in queue\n\033[0m ",get_timestamp(), ID);
            pthread_cond_wait(&is_this_crate_first, &cart_to_platform_mutex);
        }
        printf("\033[1;33m<%ld>[CART %d ]: Opening door\n\033[0m ",get_timestamp(), ID);
        pthread_mutex_unlock(&cart_to_platform_mutex);

        if(count != 0){ 
            #ifdef THREAD_DEBUG_MODE
             printf("CART NOT EMPTY\n");
            #endif
            pthread_mutex_lock(&exiting_from_cart_mutex);
            while(count > 0){
                count--;
                curr_passenger_id = passenger[count];
                passenger[count] = -1;
                client_state[curr_passenger_id] = WAITING;
                has_client_exited = 0;
                pthread_cond_broadcast(&can_client_exited_cond);
                while(has_client_exited == 0){
                    #ifdef THREAD_DEBUG_MODE
                    printf("===WAITING %d TO EXIT \n", curr_passenger_id);
                    #endif
                    pthread_cond_wait(&has_client_exited_cond, &exiting_from_cart_mutex);
                }
                carts_free_space[ID]++;
                #ifdef THREAD_DEBUG_MODE
                printf("Letting %d passenger exit\n", curr_passenger_id);
                #endif
            }
            pthread_mutex_unlock(&exiting_from_cart_mutex);

        }
        if(i == RIDES_COUNT) {
            get_from_queue(carts_queue);
            if(sem_trywait(&active_carts_sem) == -1) exitError("More carts ended than started");
            printf("\033[1;33m<%ld>[CART %d ]: Ending thread\n\033[0m ",get_timestamp(), ID);
            pthread_cond_broadcast(&is_this_crate_first);
            pthread_cond_broadcast(&can_client_enter_cond);
            return NULL;
        }
        pthread_mutex_lock(&entering_to_cart_mutex);
        the_chosen_one = -1;
        while(carts_free_space[ID] > 0){
            curr_passenger_id = get_from_queue(clients_queue);
            passenger[count] = curr_passenger_id;
            printf("\033[1;33m<%ld>[CART %d ]: telling client %d to enter, current passenger,: %d, \n\033[0m ",get_timestamp(), ID, passenger[count], curr_passenger_id);
            client_state[curr_passenger_id] = RIDING;
            has_client_entered = 0;
            pthread_cond_broadcast(&can_client_enter_cond);
            while(has_client_entered == 0){
                pthread_cond_wait(&has_client_entered_cond, &entering_to_cart_mutex);
            }
            printf("\033[1;33m<%ld>[CART %d ]: Client %d entered\n\033[0m ",get_timestamp(), ID, passenger[count]);
            has_client_entered = 0;
            carts_free_space[ID]--;
            count++;
        }
        pthread_mutex_unlock(&entering_to_cart_mutex);

        pthread_mutex_lock(&button_mutex);
        printf("\033[1;33m<%ld>[CART %d ]: CHOOSING THE CHOSEN ONE\n\033[0m ",get_timestamp(), ID);
        the_chosen_one = passenger[rand() % CART_CAPACITY];
        is_button_pressed = 0;
        printf("<%ld>[CART %d ]: THE CHOSEN ONE IS %d\n\033[0m ",get_timestamp(), ID, the_chosen_one);
        pthread_cond_broadcast(&has_someone_been_chosen_cond);
        while(is_button_pressed == 0){
            pthread_cond_wait(&is_button_pressed_cond, &button_mutex);
        }
        
        printf("\033[1;33m<%ld>[CART %d ]: Closing door\n\033[0m ",get_timestamp(), ID);
        printf("\033[1;33m<%ld>[CART %d ]: Starting the wonderful adventure\n\033[0m ",get_timestamp(), ID);
        get_from_queue(carts_queue);
        pthread_cond_broadcast(&is_this_crate_first);
        pthread_mutex_unlock(&button_mutex);
        printf("\033[1;33m<%ld>[CART %d ]: Ending the wonderful adventure :( \n\033[0m ",get_timestamp(), ID);
    }
    if(sem_trywait(&active_carts_sem) == -1) exitError("More carts ended than started");
    return NULL;
}

void* client_thread(void * args){
    int ID = *(int*) args;
    int active_carts;
    while(1){
        pthread_mutex_lock(&client_queue_mutex);
        pthread_mutex_unlock(&client_queue_mutex);

        pthread_mutex_lock(&entering_to_cart_mutex);

        while(client_state[ID] == WAITING){
            if(sem_getvalue(&active_carts_sem, &active_carts) == -1) exitErrno("Error in getting value of semaphore");
            if(active_carts == 0) {
                pthread_mutex_unlock(&entering_to_cart_mutex);
                printf("<%ld>[CLIENT %d]: Ending thread\n", get_timestamp(), ID);
                return NULL;
            }
            pthread_cond_wait(&can_client_enter_cond, &entering_to_cart_mutex);
        }
        int current_cart = seek_from_queue(carts_queue);
        printf("<%ld>[CLIENT %d]: Entering to cart n: %d, already %d in\n",get_timestamp(), ID, current_cart, CART_CAPACITY - carts_free_space[current_cart]);
        has_client_entered = 1;
        pthread_cond_signal(&has_client_entered_cond);
        pthread_mutex_unlock(&entering_to_cart_mutex);
        
        pthread_mutex_lock(&button_mutex);
        while(the_chosen_one == -1){
            // printf("waiting to point chosen one, currently %d\n", the_chosen_one);
            pthread_cond_wait(&has_someone_been_chosen_cond, &button_mutex);
        }

        if(the_chosen_one == ID){
            is_button_pressed = 1;
            printf("<%ld>[CLIENT %d]: PUSHING THE BUTTON IN CART: %d, already %d in\n",get_timestamp(), ID, current_cart, CART_CAPACITY - carts_free_space[current_cart]);
            pthread_cond_signal(&is_button_pressed_cond);
        }
        
        pthread_mutex_unlock(&button_mutex);

        pthread_mutex_lock(&exiting_from_cart_mutex);

        while(client_state[ID] == RIDING){
            // printf("[CLIENT %d ]: WAITING TO PERMISSION TO EXIT\n", ID);
            pthread_cond_wait(&can_client_exited_cond, &exiting_from_cart_mutex);
        }
        printf("<%ld>[CLIENT %d]:   EXITING FROM CART: %d, %d remained \n",get_timestamp(), ID, current_cart, CART_CAPACITY - carts_free_space[current_cart]-1);
        add_to_queue(clients_queue, ID);
        has_client_exited = 1;
        pthread_cond_signal(&has_client_exited_cond);
        
        pthread_mutex_unlock(&exiting_from_cart_mutex);

    }   

    return NULL;

}

int main(int argc, char **argv){
    
    srand(time(NULL));

    CLIENT_COUNT = atoi(argv[1]);
    CART_COUNT = atoi(argv[2]);
    CART_CAPACITY = atoi(argv[3]);
    RIDES_COUNT = atoi(argv[4]);
    clients_queue = init_queue(CLIENT_COUNT);
    carts_queue = init_queue(CART_COUNT);
    if(sem_init(&active_carts_sem, 0, CART_COUNT) == -1) exitErrno("Could not initialize semaphore");

    clients = malloc(sizeof(pthread_t) * CLIENT_COUNT);    
    if(clients == NULL) exitErrno("Error in allocating clients");
    client_state = calloc(CLIENT_COUNT, sizeof(int));
    if(client_state == NULL) exitErrno("Error in allocating clients");

    carts = malloc(sizeof(pthread_t) * CART_COUNT);    
    if(carts == NULL) exitErrno("Error in allocating carts");
    carts_free_space = malloc(CART_COUNT * sizeof(int));
    if(carts_free_space == NULL) exitErrno("Error in allocating carts");
    for(int i = 0; i < CART_COUNT; i++){
        carts_free_space[i] = CART_CAPACITY;
    }

    for(int i = 0; i < CLIENT_COUNT; i++){
        int *client_id = malloc(sizeof(int));
        *client_id = i;
        add_to_queue(clients_queue, *client_id);
        if(pthread_create(clients+i, NULL, client_thread, (void *) client_id) != 0) exitErrno("Cannot create client thread");
    }    

    for(int i = 0; i < CART_COUNT; i++){
        int *cart_id = malloc(sizeof(int));
        *cart_id = i;
        if(pthread_create(carts+i, NULL, cart_thread, (void *) cart_id) != 0) exitErrno("Cannot create client thread");
    }    
    for(int i = 0; i < CLIENT_COUNT; i++){
        pthread_join(clients[i], NULL);
        printf("client %d joined \n", i);
    }    
    for(int i = 0; i < CART_COUNT; i++){
        pthread_join(carts[i], NULL);
        printf("cart %d joined \n", i);
    }    

    pthread_cond_destroy(&is_this_crate_first);
    pthread_cond_destroy(&is_button_pressed_cond); 
    pthread_cond_destroy(&has_client_entered_cond);
    pthread_cond_destroy(&has_client_exited_cond);
    pthread_cond_destroy(&can_client_enter_cond);
    pthread_cond_destroy(&can_client_exited_cond);
    pthread_cond_destroy(&has_someone_been_chosen_cond);

    exit(0);

}