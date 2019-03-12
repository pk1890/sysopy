#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct block_arr block_arr;
typedef struct Free_queue Free_queue;
typedef struct Free_qnode Free_qnode;


#define PATH "/home/mleko/sysopy/zestaw1/zad1/tmp"

int main(){

    // struct Free_queue* queue = init_queue();
    // printf("%d\n", is_qempty(queue));
    

    // qpush(queue, 3);
    // printf("%d\n", is_qempty(queue));

    // qpush(queue, 4);
    // qpush(queue, 5);
    // printf("%d\n", is_qempty(queue));
    // qpush(queue, 1);
    // printf("=====================\n");


    // printf("%ld\n", qpop(queue));
    // printf("%ld\n", qpop(queue));

    // printf("%ld\n", qpop(queue));
    // printf("%ld\n", qpop(queue));
    // printf("=====================\n");

    // printf("%d\n", is_qempty(queue));

    block_arr* mem = init_array(5);
    delete_block(mem, 2);

    for(int i = 0; i < 20; i++){
    size_t index = find(mem, "/home/mleko", "mylib.h", "/home/mleko/sysopy/zestaw1/zad1/tmp"); 
    //printf("===================");
    printf("%s, %ld\n+++++i=%d++\n", mem->data[index], index, i);
    if (i %2 == 0) delete_block(mem, index);
    }


    
}