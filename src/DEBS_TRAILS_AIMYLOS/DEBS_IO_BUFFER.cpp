//C++
#include <iostream>

//C
#include <fcntl.h> //open(), macros
#include <unistd.h> //close()
#include <pthread.h> //pthread()   linker -pthread
#include <stdio.h>

#include "DEBS_IO_BUFFER.h"

using namespace std;




debs_buffer::debs_buffer(unsigned int blk_size, unsigned int num_of_blk, const char* filename)
{
    if(blk_size<4)
    {
        blk_size=64;
    }
    if(num_of_blk<2)
    {
        num_of_blk = 8;
    }
    block_size = blk_size;
    number_of_blocks = num_of_blk;

    //allocate block talbe
    block_table = new unsigned char*[number_of_blocks];

    //allocate first block
    block_table[0] = new unsigned char[block_size+EXTRA_BLOCK_SIZE];

    //mark it as not fully allocated
    block_table[2] = NULL;

    //update counters
    current_block = 0;
    //current_ready_block; non need
    number_of_ready_blocks = 0;
    current_block_size=block_size;
    min_current_block_size = MIN_BLOCK_SIZE_PERC*block_size;

    //initialize mutex
    pthread_mutex_init(&mutex, NULL);

    //initialize condition variable
    pthread_cond_init (&cond_var, NULL);

    //init pointer
    input_file_desc = open(filename, O_RDONLY);
}


debs_buffer::~debs_buffer()
{
    //OS will take care of this
//    for(int i=0; i<number_of_blocks; i++)
//    {
//        delete[] block_table[i];
//    }
//    delete[] block_table;

    if(input_file_desc>-1)
    {
        close(input_file_desc);
    }
    pthread_cancel(IO_thread);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);
}

/*bool debs_buffer::is_file_open()
{
    return input_file_desc>=0?true:false;
}*/




int debs_buffer::start_reading_at(unsigned int start_reading_block_size)
{
    static bool called = false;

    if(called) return -1;
    if(start_reading_block_size>block_size) return -2;
    if(input_file_desc<0) return -3;

    current_block_size = start_reading_block_size;

    //take mutex so the thread can't do anything stupid before allocation is finished
    pthread_mutex_lock(&mutex);

    if( pthread_create(&IO_thread, NULL, staticFuction, (void*)this) != 0)
    {
        return -4;
    }

    //allocate other blocks
    for(unsigned int i=1; i<number_of_blocks; i++)
    {
        block_table[i] = new unsigned char[block_size+EXTRA_BLOCK_SIZE];
    }
    //allocation is finished, give back mutex
    pthread_mutex_unlock(&mutex);

    called = true;
    return 0;
}



void* debs_buffer::staticFuction(void* arg)
{
    static_cast<debs_buffer*>(arg)->read_file();
    pthread_exit(NULL);
}

void debs_buffer::read_file(void)
{
    //number_of_ready_blocks is 0 anyway
    current_ready_block = current_block;
    bool c=false;

    //read the while file
    while(true)
    {
        //read data to current_block
        int data_read = read(input_file_desc, &(block_table[current_block][EXTRA_BLOCK_SIZE]), current_block_size);
        if( data_read<= 0)
        {
            //possibly end-of-file, if it's anything else idc
            close(input_file_desc);
            input_file_desc = -1;
            //wake up blocked thread, if it's blocked
            pthread_cond_signal(&cond_var);
            //cout << "IO_THREAD: Possibly end of file" << endl;
            return;
        }

        //write actual size on the block and mark as not free
        block_table[current_block][3] = (unsigned char)data_read;
        block_table[current_block][2] = (unsigned char)(data_read>>8);
        block_table[current_block][1] = (unsigned char)(data_read>>16);
        block_table[current_block][0] = (unsigned char)(data_read>>24);

        //taking mutex now will means allocation has finished, also protects the counters
        pthread_mutex_lock(&mutex);

        //increase counter
        number_of_ready_blocks++;

        //check if block table is full of unread data
        if(number_of_ready_blocks >= number_of_blocks)
        {
            //wait until a block is freed
            pthread_cond_wait(&cond_var, &mutex);

            //increase block size to keep the hdd spinnig
            if(current_block_size!=block_size)
            {
                current_block_size = block_size;
            }
            c=true;
        }

        //in case get_ready_block() or get_next_ready_block are blocked
        //if more than one call to get_ready_block()/get_next_ready_block() is done consider pthread_cond_broadcast()?
        if(number_of_ready_blocks==1)
        {
            //wake up blocked thread
            pthread_cond_signal(&cond_var);

            //decrease block size to keep CPU hot
            if(current_block_size>min_current_block_size)
            {
                current_block_size = 80*current_block_size/100;
            }
            c=true;
        }

        if(c)
        {
            c=false;
        }
        else if(current_block%FREQ==0)
        {
            current_block_size = DECENT_BLOCK;
        }
        //update modular counter
        current_block = (current_block+1) % number_of_blocks;

        //return mutex
        pthread_mutex_unlock(&mutex);

    }
}



void debs_buffer::get_ready_block(unsigned char** ready_block, int* ready_block_size)
{
    // get mutex
    pthread_mutex_lock(&mutex);

    //check if no ready blocks
    if(number_of_ready_blocks==0)
    {
        //check for end of file
        if(input_file_desc == -1)
        {
            *ready_block = NULL;
            *ready_block_size = 0;
            pthread_mutex_unlock (&mutex);
            return;
        }
        //wait until a block is freed
        pthread_cond_wait(&cond_var, &mutex);
        //check again. There's a tiny chance this is the end-of-file safery signal
        if(number_of_ready_blocks==0)
        {
            *ready_block = NULL;
            *ready_block_size = 0;
            pthread_mutex_unlock (&mutex);
            return;
        }
    }

    //give the ready_block and it's size
    *ready_block = &(block_table[current_ready_block][EXTRA_BLOCK_SIZE]);
    *ready_block_size = ((unsigned int)block_table[current_ready_block][0] << 24) + ((unsigned int)block_table[current_ready_block][1] << 16)
                        + ((unsigned int)block_table[current_ready_block][2] << 8) + (unsigned int)block_table[current_ready_block][3];


    //return mutex
    pthread_mutex_unlock(&mutex);
}




void debs_buffer::get_next_ready_block(unsigned char** ready_block, int* ready_block_size)
{
    // get mutex
    pthread_mutex_lock(&mutex);

    //decrease counter to free a block
    if(number_of_ready_blocks!=0) number_of_ready_blocks--;
    //block_table[current_ready_block][0]=1;

    //let IO_thread know there's an free block
    if(number_of_ready_blocks==number_of_blocks-1)
    {
            //wake up blocked thread
            pthread_cond_signal(&cond_var);
    }

    //check if no ready blocks
    if(number_of_ready_blocks==0)
    {
        //check for end of file
        if(input_file_desc < 0)
        {
            *ready_block = NULL;
            *ready_block_size = 0;
            pthread_mutex_unlock (&mutex);
            //cout << "reached1" <<endl;
            return;
        }
        //wait until a block is freed
        pthread_cond_wait(&cond_var, &mutex);
        //check again. There's a tiny chance this is the end-of-file safery signal
        //increase latency at cost of also safety
        if(number_of_ready_blocks==0)
        {
            *ready_block = NULL;
            *ready_block_size = 0;
            pthread_mutex_unlock (&mutex);
            //cout << "reached2" <<endl;
            return;
        }
    }

    //update modular counter
    current_ready_block = (current_ready_block+1) % number_of_blocks;

    //give the ready_block and it's size
    *ready_block = &(block_table[current_ready_block][EXTRA_BLOCK_SIZE]);
    *ready_block_size = ((unsigned int)block_table[current_ready_block][0] << 24) + ((unsigned int)block_table[current_ready_block][1] << 16)
                        + ((unsigned int)block_table[current_ready_block][2] << 8) + (unsigned int)block_table[current_ready_block][3];

    //return mutex
    pthread_mutex_unlock(&mutex);
}


































void debs_buffer::print_buffer_info()
{
    cout << "\nBUFFER INFORMATION\n";
    cout << "INIT number of blocks: " << number_of_blocks << endl;
    cout << "INIT block size: " << block_size << endl;
    cout << "current_block: " << current_block << endl;
    cout << "current block size: " << current_block_size << endl;
    cout << "current ready block: " << current_ready_block << endl;
    cout << "number of ready blocks: " << number_of_ready_blocks << endl;
    cout << "END OF BUFFER INFORMATION\n\n";
}

























/*
int debs_buffer::start_reading_at(unsigned int start_reading_block_size)
{
    //check input arguments
    if(start_reading_block_size>block_size)
    {
        return -1;
    }

    //read 'non-blocking' start_reading_block_size to block 0
    off_t input_file_position = lseek( input_file_desc, 0, SEEK_CUR );
    if(read(input_file_desc, block_table[0]+EXTRA_BLOCK_SIZE-1, start_reading_block_size) < 0)
    {
        //possibly end-of-file, if it's anything else idc
        return -2;
    }

    //allocate other blocks
    for(int i=1; i<number_of_blocks; i++)
    {
        block_table[i] = new char[block_size+EXTRA_BLOCK_SIZE];
    }

    //select() until IO is ready
    fd_set read_fd_set;
    FD_ZERO(&read_fd_set);
    FD_SET(input_file_desc, &read_fd_set);

    if( select(input_file_desc+1, &read_fd_set, NULL, NULL, NULL) <=0 )
    {
        return -3;
    }

    //write number on the block and mark as not free
    unsigned int data_read = lseek( input_file_desc, 0, SEEK_CUR )-input_file_position;
    cout<< "sdas   " << data_read << endl;
    block_table[0][4] = (char)data_read;
    block_table[0][3] = (char)(data_read>>8);
    block_table[0][2] = (char)(data_read>>16);
    block_table[0][1] = (char)(data_read>>24);
    block_table[0][0] = (char)0;

    //start new thread for IO and return this function



    return 0;
}
*/
