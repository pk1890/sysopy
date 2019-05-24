#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<errno.h>
#include<sys/time.h>
#include<string.h>

#define ceil_div(X, Y) (((X) + (Y) - 1) / (Y))


void exitError(char* msg){
    printf(msg);
    exit(1);
}

void exitErrno(char* msg){
    perror(msg);
    exit(1);
}

struct timeval curr_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

long int time_diff(struct timeval t1, struct timeval t2)
{
    return (t2.tv_sec - t1.tv_sec) * 1000000
        + t2.tv_usec - t1.tv_usec;
}


typedef struct image{
    int width;
    int height;
    int **buff;
} image;

typedef struct filter{
    int size;
    double **buff;
} filter;

image *input, *output;
filter *fil;

int** alloc_buffer(int width, int height){
    int **buff = malloc(height*sizeof(int*));
    if(buff == NULL) exitErrno("Error in allocating image"); 
    for(int i = 0; i < height; i++){
        buff[i] = malloc(width*sizeof(int));
        if(buff[i] == NULL) exitErrno("Error in allocating image"); 
    }
    return buff;
}

double** alloc_buffer_filter(int size){
    double **buff = malloc(size*sizeof(double));
    if(buff == NULL) exitErrno("Error in allocating image"); 
    for(int i = 0; i < size; i++){
        buff[i] = malloc(size*sizeof(double));
        if(buff[i] == NULL) exitErrno("Error in allocating image"); 
    }
    return buff;
}

image* open_image(char* path){
    FILE* fp = fopen(path, "r");
    image *res = malloc(sizeof(image));
    fscanf(fp, "P2 %i %i 255", &res->width, &res->height);
    res->buff = alloc_buffer(res->width, res->height);
    for (int y = 0; y < res->height; y++) {
        for (int x = 0; x < res->width; x++) {
            fscanf(fp, "%i", &res->buff[y][x]);
        }
    }
    fclose(fp);
    return res;

}

filter* open_filter(char* path){
    FILE* fp = fopen(path, "r");
    filter *res = malloc(sizeof(filter));
    fscanf(fp, "%i", &res->size);
    res->buff = alloc_buffer_filter(res->size);
    for (int y = 0; y < res->size; y++) {
        for (int x = 0; x < res->size; x++) {
            fscanf(fp, "%lf", &res->buff[y][x]);
        }
    }
    fclose(fp);
    return res;

}

void save_image(image *img, char* img_name)
{
    FILE *fp = fopen(img_name, "w");
    if (fp == NULL) exitErrno("Unable to open output file");

    fprintf(fp, "P2\n%i %i\n255\n", img->width, img->height);

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            fprintf(fp, "%i", img->buff[y][x]);
            if (x+1 != img->width) fputc(' ', fp);
        }
        fputc('\n', fp);
    }
}


int m;

int clamp(int val, int a, int b){
    return val < a ? a : ( val > b ) ? b : val ;
}

void filter_column(int x){
    double sum;
    int tx, ty;
    // printf("MMMM %d\n", x);
    for(int y = 0; y < input->height; y++){
        sum = 0.0;
        for(int i = 0; i < fil->size; i++){
            for(int j = 0; j < fil->size; j++){
               ty  = clamp(y - ceil_div(fil->size, 2) + i - 1, 0, input->height - 1);
               tx  = clamp(x - ceil_div(fil->size, 2) + j - 1, 0, input->width - 1);
    // printf("MMMA %d %d %d %d %d %d\n", x, y, i, j, tx, ty);
               sum += input->buff[ty][tx] * fil->buff[i][j];
    // printf("MMMB %d %d %d %d %d %d\n", x, y, i, j, tx, ty);
            }
        }
        output->buff[y][x] = round(sum);
    }
}



void* block_thread(void* params){
    struct timeval start = curr_time();

    int k = (*(int*) params);
    int begin = ceil_div(k*input->width, m);
    int end = ceil_div((k+1)*input->width, m);
    for(int i =begin; i <end; i++){
    // printf("MKEEE\n");
        filter_column(i);
    }

    long *time = malloc(sizeof(long));
    *time = time_diff(start, curr_time());

    pthread_exit(time);
}

void* interleaved_thread(void *params){

    struct timeval start = curr_time();

    int k = (*(int*) params);
    for(int i =k ; i <input->width; i+=m){
        filter_column(i);
    }
    long *time = malloc(sizeof(long));
    *time = time_diff(start, curr_time());

    pthread_exit(time);
}

int main(int argc, char**argv){
    if(argc != 6) exitError("Usage: filtering <path_to_img> <path_to_filter> <output_path> <threads_no> <mode (B/I)>");
    char *path_in = argv[1];
    char *path_filter = argv[2];
    char *path_out = argv[3];
    m = atoi(argv[4]);
    char *mode = argv[5];
    if(m <=0) exitError("Thread number must be positive integer");

    // printf("MLEKO1\n");

    input = open_image(path_in);
    // printf("MLEKO2\n");
    fil = open_filter(path_filter);
    // printf("MLEKO3\n");
    output = malloc(sizeof(image));
    output->width = input->width;
    output->height = input->height;
    output->buff = alloc_buffer(output->width, output->height);
    // printf("MLEKO4\n");

    pthread_t *threads = calloc(m, sizeof(pthread_t));

    // printf("MLEKO5\n");
    for(int i = 0; i < m; i++){
        int *no = malloc(sizeof(int));
        *no = i;
        if(strncmp("B", mode, 1) == 0) {
            if(pthread_create(&threads[i], NULL, &(block_thread), no) != 0) exitErrno("Unable to create thread");
        } else{
            if(pthread_create(&threads[i], NULL, &(interleaved_thread), no) != 0) exitErrno("Unable to create thread");
        }
    }

    struct timeval start = curr_time();
    long total_time = 0;
    for (int k = 0; k < m; k++) {
        long *time;
        pthread_join(threads[k], (void *) &time);
        printf("thread %d:\t%ldus\n", k+1, *time);
        total_time += *time;
    }
    printf("total time:\t%ldus\n", time_diff(start, curr_time()));
    printf("sum time:\t%ldus\n", total_time);

    save_image(output, path_out);

    return 0;
}