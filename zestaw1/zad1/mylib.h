#include <stddef.h>
#ifndef ZESTAW1_LIBRARY_H
#define ZESTWA1_LIBRARY_H


struct Free_qnode{
    size_t index;
    struct Free_qnode* next;
    
};

struct Free_queue{
    struct Free_qnode* first;
    struct Free_qnode* last;
    struct Free_qnode* ward;
};

struct block_arr
{
    char **data;
    size_t block_no;
    struct Free_queue* free_indeces;
    size_t first_free;
};



void qpush(struct Free_queue* queue, size_t index);
size_t qpop(struct Free_queue* queue);

struct Free_queue* init_queue();

short is_qempty(struct Free_queue* queue);

void setPath(char* given_path);

struct block_arr* init_array(size_t blk_no);

int write_to_tmp(char* path_to_file);

size_t read_file_to_block(struct block_arr* memory, char* path_to_file);// zwraca indeks

int delete_block(struct block_arr* memory, size_t index);

#endif