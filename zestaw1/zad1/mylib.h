#include <stddef.h>
#ifndef ZESTAW1_LIBRARY_H
#define ZESTWA1_LIBRARY_H


struct block_arr
{
    char **data;
    size_t block_no;
    size_t* free_indeces;
};


struct Free_qnode{
    size_t index;
    struct Free_qnode* next;
    
};

struct Free_queue{
    struct Free_qnode* first;
    struct Free_qnode* last;
    struct Free_qnode* ward;
};


void qpush(struct Free_queue* queue, size_t index);
size_t qpop(struct Free_queue* queue);

struct Free_queue* init_queue();

short is_qempty(struct Free_queue* queue);

void setPath(char* given_path);

struct block_arr* init_array(size_t blk_no);

int write_to_tmp(char* path_to_file);

size_t read_file_to_block(char* path_to_file); // zwraca indeks

int delete_block(size_t index);

#endif