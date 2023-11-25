#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <pthread.h>
#include <iomanip>

using namespace std;

#include "DEBS_IO_BUFFER.h"

#define NUM_OF_THREADS 4

#define MAX_BLOCK_SIZE 1048576   //1MB
#define NUMBER_OF_BLOCKS 6656    //6.5GB
#define STARTING_BLOCK_SIZE 8096
#define STARTING_CHUNK_SIZE 2048

#define TAIL_SAFE_OFFSET 55
#define START_SAFE_OFFSET 106
#define START_MIDDLE_SAFE_OFFSET 121
#define BUF_SIZE 16

void *debs_thread(void *args);
//IO_buffer
debs_buffer *myBuffer;
//cpu_thread variables
pthread_mutex_t cpu_thread_mutex;
pthread_mutex_t leftover_mutex;
//pthread_barrier_t cpu_thread_barrier;
pthread_cond_t next_block_cond_var;
short int completed_previous_block;
//variables for parallel processing
unsigned char *data_block;
int i, ii, block_size, chunk_size;
unsigned char buf[BUF_SIZE]; short int buf_index;

int main(int argc, char** argv)
{
    if(argc<2)
    {
        cout << "Give filename." << endl;
        return 0;
    }

    //make buffer
    myBuffer = new debs_buffer(MAX_BLOCK_SIZE,NUMBER_OF_BLOCKS,argv[1]);

    //start IO
    if(myBuffer->start_reading_at(STARTING_BLOCK_SIZE) < 0)
    {
        cout << "Bad filename(most likely)  or a thread problem." << endl;
    }

    //get first block
    myBuffer->get_ready_block(&data_block, &block_size);

    //results
    long int sum1[3]= {0,0,0}; //first two are sum the 3rd is line counter
    long int sum2[3]= {0,0,0};
    long int sum3[3]= {0,0,0};
    long int sum4[3]= {0,0,0};

    //process first line
    i=106;
    int k=0;
    while(data_block[i]!=',')
    {
        k = 10*k + (int)data_block[i] - 48;
        i++;
    }
    sum1[0]+=k;
    k=0;
    i++;
    while(data_block[i]!=',')
    {
        if(data_block[i]!='.')
        {
            k = 10*k + (int)data_block[i] - 48;
        }
        i++;
    }
    sum1[1]+=k;
    sum1[2]++;
    i+=TAIL_SAFE_OFFSET;

    //initialize
    pthread_mutex_init(&cpu_thread_mutex, NULL);
    pthread_mutex_init(&leftover_mutex, NULL);
    //pthread_barrier_init(&cpu_thread_barrier, NULL, NUM_OF_THREADS);
    pthread_cond_init(&next_block_cond_var, NULL);
    chunk_size = STARTING_CHUNK_SIZE;  //block_size/100; //fix dynamically with timing to guess cash
    completed_previous_block=0; ii=0; buf_index=-1;

    pthread_t threads[NUM_OF_THREADS];
    pthread_create(&threads[0], NULL, debs_thread, (void *)sum1);
    pthread_create(&threads[1], NULL, debs_thread, (void *)sum2);
    pthread_create(&threads[2], NULL, debs_thread, (void *)sum3);
    pthread_create(&threads[3], NULL, debs_thread, (void *)sum4);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);

    sum1[0]=sum1[0]+sum2[0]+sum3[0]+sum4[0];
    sum1[1]=sum1[1]+sum2[1]+sum3[1]+sum4[1];
    sum1[2]=sum1[2]+sum2[2]+sum3[2]+sum4[2];

    cout << "AVG1: " << sum1[0] << "  AVG2: " << setprecision(15)<< (double)sum1[1]/100.0 << "   LINES: " << sum1[2] << endl;
    //cout << "CHECK" << omp_get_thread_num() << endl;

    delete myBuffer;
    pthread_mutex_destroy(&cpu_thread_mutex);
    pthread_mutex_destroy(&leftover_mutex);
    //pthread_barrier_destroy(&cpu_thread_barrier);
    pthread_cond_destroy(&next_block_cond_var);
    pthread_exit(NULL);
}




void *debs_thread(void *args)
{
    int j,k,my_chunk_size;
    long int *sum;
    sum=(long int*)args;
    //long int bsum=block_size;

    while(1)
    {
        //enter critical region
        pthread_mutex_lock(&cpu_thread_mutex);

        if(i>=block_size)
        {
            completed_previous_block++;
            if(completed_previous_block<NUM_OF_THREADS)
            {
                //wait for the others
                pthread_cond_wait(&next_block_cond_var, &cpu_thread_mutex);
                if(block_size==0)
                {
                    pthread_mutex_unlock(&cpu_thread_mutex);
                    pthread_exit(NULL);
                }
                pthread_mutex_unlock(&cpu_thread_mutex);
                continue;
            }
            else
            {
                //get new block
                myBuffer->get_next_ready_block(&data_block, &block_size);
                i=0;    //file_size+=block_size; cout<<file_size<<endl;
                completed_previous_block=0;
                //signal other threads
                pthread_cond_broadcast(&next_block_cond_var);
                //check for exit
                if(block_size==0)
                {
                    pthread_mutex_unlock(&cpu_thread_mutex);
                    pthread_exit(NULL);
                }
                pthread_mutex_unlock(&cpu_thread_mutex);
                continue;
            }
        }


/*
        if(block_size==0)
        {
            pthread_mutex_unlock(&cpu_thread_mutex);
            pthread_exit(NULL);
        }
        if(i>=block_size)
        {
            completed_previous_block++;
            if(completed_previous_block<NUM_OF_THREADS)
            {
                //wait for the others
                pthread_mutex_unlock(&cpu_thread_mutex);
                pthread_barrier_wait(&cpu_thread_barrier);
                //pthread_cond_wait(&next_block_cond_var, &cpu_thread_mutex);

                //pthread_mutex_unlock(&cpu_thread_mutex);
                //usleep(100);
                //continue;
            }
            else
            {
                //get new block
                myBuffer->get_next_ready_block(&data_block, &block_size);
                i=0;    //file_size+=block_size; cout<<file_size<<endl;
                completed_previous_block=0;
                //signal other threads
                pthread_mutex_unlock(&cpu_thread_mutex);
                pthread_barrier_wait(&cpu_thread_barrier);
                //pthread_cond_broadcast(&next_block_cond_var);
                //check for exit
                //continue;
            }
            continue;
        }
*/

        //starting index
        j=i;
        //ending index
        if( i > (block_size-512) )
        {
            i = block_size;
            my_chunk_size = block_size;
        }
        else
        {
            i = i+chunk_size;
            my_chunk_size = i;
        }

        //exit critical region
        pthread_mutex_unlock(&cpu_thread_mutex);

        //proccess chunk
        //check if this is the first chunk of a block
        if( buf_index > -1 && j==0)
        {
            pthread_mutex_lock(&leftover_mutex);
            j=ii;
            //complete reading
            while(buf_index<BUF_SIZE)
            {
                buf[buf_index] = data_block[j];
                j++; buf_index++;
            }
            //calc
            k=0;
            buf_index=0;
            while(buf[buf_index]!=',' && buf_index<BUF_SIZE)
            {
                k = 10*k + (int)buf[buf_index] - 48;
                buf_index++;
            }
            sum[0]+=k;
            k=0;
            buf_index++;
            while(buf[buf_index]!=',' && buf_index<BUF_SIZE)
            {
                if(buf[buf_index]!='.')
                {
                    k = 10*k + (int)buf[buf_index] - 48;
                }
                buf_index++;
            }
            sum[1]+=k;
            sum[2]++;
            j+=TAIL_SAFE_OFFSET;
            buf_index=-1;
            ii=0;
            pthread_mutex_unlock(&leftover_mutex);
        }

        //process normally
        for(; j<my_chunk_size; j++)
        {
            //find a new line
            if(data_block[j]!='\n') continue;
            j++;

            //check if new line is too close the the end of the block
            if( (block_size-j) <= START_MIDDLE_SAFE_OFFSET )
            {
                pthread_mutex_lock(&leftover_mutex);
                //set comma counter and buf_index to 0
                buf_index=0;

                //too close too the end, no interesting data
                if( (block_size-j) <= START_SAFE_OFFSET)
                {
                    //start of new block
                    ii = START_SAFE_OFFSET-(block_size-j);
                }
                //some or all of middle part is here
                else
                {
                    j+=START_SAFE_OFFSET;
                    ii=0;
                    while(buf_index<BUF_SIZE && j<block_size)
                    {
                        buf[buf_index] = data_block[j];
                        j++; buf_index++;
                    }
                }
                pthread_mutex_unlock(&leftover_mutex);
                break;
            }
            //read line normally
            else
            {
                j+=START_SAFE_OFFSET;
                k=0;
                while(data_block[j]!=',')
                {
                    k = 10*k + (int)data_block[j] - 48;
                    j++;
                }
                sum[0]+=k;
                k=0;
                j++;
                while(data_block[j]!=',')
                {
                    if(data_block[j]!='.')
                    {
                        k = 10*k + (int)data_block[j] - 48;
                    }
                    j++;
                }
                sum[1]+=k;
                sum[2]++;
                j+=TAIL_SAFE_OFFSET;
            }
        }
    }
}
















/*
    long int sum=0;
    long int num_of_blocks=1;
    while(block_size!=0)
    {
        cout << block_size << "  " << num_of_blocks++ << endl;
        sum+=block_size;
        myBuffer->get_next_ready_block(&data_block, &block_size);
    }
    cout<<sum << endl;
    return 0;
    */
