#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "mylib.h"

// struct block_arr
// {
//     char **data;
//     size_t block_no;
//     size_t* free_indeces;
// };


char* CURRENT_DIRECTORY;
char* filename;


void qpush(struct Free_queue* queue, size_t index){
    struct Free_qnode *node = malloc(sizeof(struct Free_qnode));
    node->index = index;
    node->next = NULL;

    queue->last->next = node;
    queue->last = node;
}
size_t qpop(struct Free_queue* queue){
    
    struct Free_qnode* res = queue->first->next;
    queue->first->next = res->next;
    return res->index;
}

short is_qempty(struct Free_queue* queue){
    if(queue->first->next == NULL) return 1;
    return 0;
}

struct Free_queue* init_queue(){
    struct Free_qnode* ward = malloc(sizeof(struct Free_qnode));
    ward->next = NULL;
    struct Free_queue* queue = malloc(sizeof(struct Free_queue));
    queue->first = ward;
    queue->last = ward;
    queue->ward = ward;

    return queue;
}


struct block_arr* init_array(size_t blk_no){

    if(blk_no == 0){
        return NULL;
    }

    struct block_arr* result = (struct block_arr*)calloc(1, sizeof (struct block_arr));

    result->block_no  = blk_no;
    result->data = (char**)calloc(blk_no, sizeof (char*));

    return result;
}

int write_to_tmp(char* path_to_file){
    if(path_to_file == NULL || filename == NULL || CURRENT_DIRECTORY == NULL) return -1;
    
    char buff[512];
    snprintf(buff, sizeof(buff), "find %s -name %s > %s", CURRENT_DIRECTORY, filename, path_to_file);

    system(buff);
    return 0;
}

void find(char* dir, char* filetofind, char* file){
    CURRENT_DIRECTORY = dir;
    filename = filetofind;
    write_to_tmp(file);
}


 size_t read_file_to_block(char* path_to_file){
     FILE *tmp_file;

 }