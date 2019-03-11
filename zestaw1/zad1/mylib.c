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
    if(queue == NULL || queue->first == NULL){
        printf("Error - freelist not allocated properly\n");
        exit(EXIT_FAILURE);
    }

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

    result->free_indeces = init_queue();
    

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


size_t reserve_next_free_block_idx(struct block_arr* memory){
    size_t idx;

    if(memory->first_free < memory->block_no){
        idx = memory->first_free;
        memory->first_free++;
    } else if(!is_qempty(memory->free_indeces)){
        idx = qpop(memory->free_indeces);
    }
    else
    {
        printf("out of memory");
        exit(EXIT_FAILURE);
    }
    return idx;
    
}

size_t read_file_to_block(struct block_arr* memory, char* path_to_file){
    FILE *tmp_file = fopen(path_to_file, "r");
    size_t file_len = 0;
    if (tmp_file == NULL)
    {
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }

    fseek(tmp_file, 0L , SEEK_END);
    file_len = (size_t)ftell(tmp_file);
    rewind(tmp_file);

    size_t block_index = reserve_next_free_block_idx(memory);

    memory->data[block_index] = calloc(file_len+1, sizeof(char));

    if( memory->data[block_index] == NULL ){       fclose(tmp_file);
        fputs("memory alloc fails",stderr);
        exit(EXIT_FAILURE);
    }

    size_t res_code = fread(memory->data[block_index], file_len, sizeof(char), tmp_file);

    if(res_code != 1){
        free (memory->data[block_index]);
        fclose(tmp_file);
        printf("Error while reading file");
        exit(EXIT_FAILURE);
    }
    fclose(tmp_file);
    return block_index;
 }

 int delete_block(struct block_arr* memory, size_t index){
     if(memory->data[index] == NULL){
         return 1;
     }
    free(memory->data[index]);
    memory->data[index] = NULL;
    return 0;
 }