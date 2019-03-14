#include <stddef.h>
#ifndef ZESTAW1_LIBRARY_H
#define ZESTWA1_LIBRARY_H



typedef struct Free_qnode{
    size_t index;
    struct Free_qnode* next;

} Free_qnode;

typedef struct {
    Free_qnode* first;
    Free_qnode* last;
    Free_qnode* ward;
} Free_queue;

typedef struct 
{
    char **data;
    size_t block_no;
    Free_queue* free_indeces;
    size_t first_free;
} block_arr;



void qpush( Free_queue* queue, size_t index);
size_t qpop( Free_queue* queue);

size_t find( block_arr* memory, char* dir, char* filetofind, char* file);
Free_queue* init_queue();

short is_qempty(Free_queue* queue);

//void setPath(char* given_path);

block_arr* init_array(size_t blk_no);

int write_to_tmp(char* path_to_file, char* dir, char* filetofind);
size_t read_file_to_block(block_arr* memory, const char* path_to_file);// zwraca indeks


int delete_block( block_arr* memory, size_t index);

void delete_block_arr (block_arr**);

#endif