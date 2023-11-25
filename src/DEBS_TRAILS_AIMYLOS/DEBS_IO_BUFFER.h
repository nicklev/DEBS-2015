#include <pthread.h> //pthread()

#ifndef DEBS_IO_BUFFER_H
#define DEBS_IO_BUFFER_H


#define EXTRA_BLOCK_SIZE 4
#define EXTRA_BLOCK_OFFSET 3

#define MIN_BLOCK_SIZE_PERC 1

#define DECENT_BLOCK 32768
#define FREQ 100

//#define BLOCK_HISTORY_NUM 0 //will always keep a freed block


/* A few details on debs_buffer

    This is only for one file.
    This is only for one sequential pass through the file.
    The main starting function start_reading_at() will return error -2 if called twice.
    The function get_ready_block() will return the oldest ready block.
    The function get_next_ready_block() will mark the oldest ready block as free and return the new oldest ready block.
    Both get_ready_block() and get_next_ready_block() are blocking functions.
    Once end-of-file reached the IO thread will close. The data left in the buffer can be read.
    */


class debs_buffer
{
private:
    //unsigned int memory_available; //bytes,  GB->2^30Bytes
    unsigned int block_size; //bytes, real size is block_size+5, first 1byte marks free or not and 4 bytes indicate data size-big endian
    unsigned int number_of_blocks;

    //blocks are written and read in modular cycle
    unsigned char **block_table;
    unsigned int current_block; //index to current block
    unsigned int current_block_size;
    unsigned int min_current_block_size;
    unsigned int current_ready_block; //index to oldest non-freed block
    unsigned int number_of_ready_blocks;

    int input_file_desc; //input file

    pthread_t IO_thread; //IO will run on a thread until eof
    pthread_mutex_t mutex; //inside object synchro
    pthread_cond_t cond_var; //inside object synchro

    void read_file(void);
    static void* staticFuction(void* arg);

public:
    debs_buffer(unsigned int blk_size, unsigned int num_of_blk, const char* filename);
    ~debs_buffer();

    //bool is_file_open();

    int start_reading_at(unsigned int start_reading_block_size);

    //returns current ready block of data
    void get_ready_block(unsigned char** ready_block, int* ready_block_size);

    //returns the next ready block of data and marks current as freed
    void get_next_ready_block(unsigned char** ready_block, int* ready_block_size);

    void print_buffer_info();
};
#endif
