#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "mylib.h"

// block_arr
// {
//     char **data;
//     size_t block_no;
//     size_t* free_indeces;
// };



void qpush(Free_queue* queue, size_t index){
    Free_qnode *node = malloc(sizeof(Free_qnode));
    node->index = index;
    node->next = NULL;

    queue->last->next = node;
    queue->last = node;
}
size_t qpop(Free_queue* queue){
    
    Free_qnode* res = queue->first->next;
    queue->first->next = res->next;
    return res->index;
}

short is_qempty(Free_queue* queue){
    if(queue == NULL || queue->first == NULL){
        printf("Error - freelist not allocated properly\n");
        exit(EXIT_FAILURE);
    }

    if(queue->first->next == NULL) return 1;
    return 0;
}

Free_queue* init_queue(){
    Free_qnode* ward = malloc(sizeof(Free_qnode));
    ward->next = NULL;
    Free_queue* queue = malloc(sizeof(Free_queue));
    queue->first = ward;
    queue->last = ward;
    queue->ward = ward;

    return queue;
}


block_arr* init_array(size_t blk_no){

    if(blk_no == 0){
        return NULL;
    }

    block_arr* result = (block_arr*)calloc(1, sizeof (block_arr));

    result->block_no  = blk_no;
    result->data = (char**)calloc(blk_no, sizeof (char*));

    result->free_indeces = init_queue();
    

    return result;
}

int write_to_tmp(char* path_to_file, char* dir, char* filetofind){
    if(path_to_file == NULL || filetofind == NULL || dir == NULL) return -1;
    
    char buff[512];
    snprintf(buff, sizeof(buff), "find %s -name %s > %s", dir, filetofind, path_to_file);

    system(buff);

    return 0;
}

size_t find(block_arr* memory, char* dir, char* filetofind, char* file){
    write_to_tmp(file, dir, filetofind);
    size_t res = read_file_to_block(memory, file);
    remove(file);
    return res;

}


size_t reserve_next_free_block_idx(block_arr* memory){
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

size_t read_file_to_block(block_arr* memory, const char* path_to_file){
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

 int delete_block(block_arr* memory, size_t index){
     if(memory->data[index] == NULL){
         return 1;
     }
    free(memory->data[index]);
    memory->data[index] = NULL;
    qpush(memory->free_indeces, index);
    return 0;
 }

 void delete_block_arr (block_arr** mem){
    for(size_t i = 0; i < (*mem)->block_no ; i++){
        if((*mem)->data[i] != NULL){
            free((*mem)->data[i]);
            (*mem)->data[i] = NULL;
        }

    }
    free ( (*mem) );
    (*mem) = NULL;
}
