#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    //find("/", "mylib.h", "/home/mleko/sysopy/zestaw1/zad1/tmp");

    struct Free_queue* queue = init_queue();
    printf("%d\n", is_qempty(queue));
    

    qpush(queue, 3);
    printf("%d\n", is_qempty(queue));

    qpush(queue, 4);
    qpush(queue, 5);
    printf("%d\n", is_qempty(queue));
    qpush(queue, 1);
    printf("=====================\n");


    printf("%ld\n", qpop(queue));
    printf("%ld\n", qpop(queue));

    printf("%ld\n", qpop(queue));
    printf("%ld\n", qpop(queue));
    printf("=====================\n");

    printf("%d\n", is_qempty(queue));



    
}