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

#define WAITING 0
#define RIDING 1
                                                                                                                                                                               
                                                                                                                                                                                   
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

typedef struct cart{
    int free_space;
    int is_button_pressed;
}cart;

int *client_state;
cart *carts_data;

queue *clients_queue;
queue *carts_queue;

int CLIENT_COUNT, CART_COUNT, CART_CAPACITY;
int the_chosen_one = -1;
int has_client_entered;

pthread_t *clients;
pthread_t *carts;
pthread_mutex_t client_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t carts_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cart_to_platform_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t button_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t entering_to_cart_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t is_this_crate_first = PTHREAD_COND_INITIALIZER;
pthread_cond_t is_button_pressed = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_client_entered_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_client_enter_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_someone_been_chosen_cond = PTHREAD_COND_INITIALIZER;

queue *init_queue(size_t size){
    queue *q = malloc(sizeof(queue));
    if(q == NULL) exitErrno("Error in allocating queue");
    q->buff = malloc(sizeof(int) * (size+1));
    if(q->buff == NULL) exitErrno("Error in allocating queue buff");
    q->first = 0;
    q->last = 0;
    q->size = size+1;
    return q;
}

void add_to_queue(queue *q, int item){
    if((q->last + 1) % q->size == q->first) exitError("Trying to put item in full queue");

    q->buff[q->last] = item;
    q->last = (q->last + 1) % q->size;
}

int get_from_queue(queue *q){
    if(q->first == q->last) exitError("Trying to get item from empty queue");
    int res = q->buff[q->first];
    q->first = (q->first+1) % q->size;
    return res;
}

int seek_from_queue(queue *q){
    if(q->first == q->last) exitError("Trying to get item from empty queue");
    return q->buff[q->first];
}

void* cart_thread(void* args){
    int ID = *(int*)args;
    int *passenger = malloc(CART_CAPACITY * sizeof(int));
    int count = 0;
    int curr_passenger_id;

    pthread_mutex_lock(&cart_to_platform_mutex);
    while(seek_from_queue(carts_queue) != ID){
        pthread_cond_wait(&is_this_crate_first, &cart_to_platform_mutex);
    }
    printf("[CART %d ]: Opening door\n");
    pthread_mutex_unlock(&cart_to_platform_mutex);
    pthread_mutex_lock(&entering_to_cart_mutex);
    whlie(carts_data[ID].free_space > 0){
        curr_passenger_id = get_from_queue(clients_queue);
        client_state[curr_passenger_id] = RIDING;
        passenger[count] = curr_passenger_id;
        has_client_entered = 0;
        while(has_client_entered == 0){
            pthread_cond_wait(&has_client_entered_cond, &entering_to_cart_mutex);
        }
        has_client_entered = 0;
        carts_data[i].free_space--;
        count++;
    }
    pthread_mutex_unlock(&entering_to_cart_mutex);

    pthread_mutex_lock(&button_mutex);

    the_chosen_one = passenger[rand() % CART_CAPACITY];

    while(carts_data[ID].is_button_pressed == 0){
        pthread_cond_wait(&is_button_pressed, &button_mutex);
    }
    
    printf("[CART %d ]: Closing door\n");
    the_chosen_one = -1;
    pthread_mutex_unlock(&button_mutex);
    printf("[CART %d ]: Starting the wonderful adventure\n");
    get_from_queue(carts_queue);
}

void* client_thread(void * args){
    int ID = *(int*) args;

    pthread_mutex_lock(&client_queue_mutex);
    add_to_queue(clients_queue, ID);
    pthread_mutex_unlock(&client_queue_mutex);

    pthread_mutex_lock(&entering_to_cart_mutex);

    while(client_state[ID] == WAITING){
        pthread_mutex_wait(&can_client_enter_cond, &entering_to_cart_mutex);
    }
    int current_cart = seek_from_queue(carts_queue);
    printf("[CLIENT %d]: Entering to cart n: %d, already %d in", ID, current_cart, CART_CAPACITY - carts_data[current_cart].free_space);
    has_client_entered = 1;
    pthread_mutex_unlock(&entering_to_cart_mutex);
    
    pthread_mutex_lock(&button_mutex);
    while(the_chosen_one == -1){
        pthread_cond_wait()
    }
    


}

int main(int argc, char **argv){
    
    srand(time(NULL));

    CLIENT_COUNT = atoi(argv[1]);
    CART_COUNT = atoi(argv[2]);
    CART_CAPACITY = atoi(argv[3]);

    clients_queue = init_queue(CLIENT_COUNT);
    carts_queue = init_queue(CART_COUNT);

    clients = malloc(sizeof(pthread_t) * CLIENT_COUNT);    
    if(clients == NULL) exitErrno("Error in allocating clients");
    client_state = calloc(CLIENT_COUNT, sizeof(int));
    if(client_state == NULL) exitErrno("Error in allocating clients");

    carts = malloc(sizeof(pthread_t) * CLIENT_COUNT);    
    if(carts == NULL) exitErrno("Error in allocating carts");
    carts_data = malloc(CART_COUNT * sizeof(cart));
    if(carts_data == NULL) exitErrno("Error in allocating carts");
    for(int i = 0; i < CART_COUNT; i++){
        carts_data[i].free_space = CART_CAPACITY;
        carts_data[i].is_button_pressed = 0;
    }

    for(int i = 0; i < CLIENT_COUNT; i++){
        int *client_id = malloc(sizeof(int));
        *client_id = i;
        if(pthread_create(clients+i, NULL, client_thread, (void *) client_id) != 0) exitErrno("Cannot create client thread");
    }    

    for(int i = 0; i < CART_COUNT; i++){
        int *cart_id = malloc(sizeof(int));
        *cart_id = i;
        if(pthread_create(clients+i, NULL, cart_thread, (void *) cart_id) != 0) exitErrno("Cannot create client thread");
    }    

    exit(0);

}