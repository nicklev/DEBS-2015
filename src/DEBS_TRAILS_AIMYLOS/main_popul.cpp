#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <iomanip>
#include <math.h>
#include <pthread.h>

using namespace std;

#include "DEBS_IO_BUFFER.h"
#include "miniBINheap.h"

#define NUM_OF_THREADS 4

#define MAX_BLOCK_SIZE 1048576
#define NUMBER_OF_BLOCKS 6656
#define STARTING_BLOCK_SIZE 8192

#define STARTING_CHUNK_SIZE 2048

#define COORD_STEP 0.002
#define TAIL_SAFE_OFFSET 20
#define START_SAFE_OFFSET 106
#define START_MIDDLE_SAFE_OFFSET 145
#define BUF_SIZE 22

void *debs_thread(void *args);
//IO_buffer
debs_buffer *myBuffer;
//cpu_thread variables
pthread_mutex_t cpu_thread_mutex;
pthread_mutex_t leftover_mutex;
pthread_cond_t next_block_cond_var;
short int completed_previous_block;
//variables for parallel processing
unsigned char *data_block;
int i, ii, block_size, chunk_size;
unsigned char buf[BUF_SIZE]; short int last_comma, buf_index;

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
        return 0;
    }

    //get first block
    myBuffer->get_ready_block(&data_block, &block_size);

    //results
    long int sum1[250001]= {0};
    long int sum2[250001]= {0};
    long int sum3[250001]= {0};
    long int sum4[250001]= {0};

    //process first line
    i=START_SAFE_OFFSET;
    short int k=0;
    int longcell, latcell;
    float longitude,latitude;
    while(k<2)
    {
        if(data_block[i++]==',') k++;
    }
    if(data_block[i]=='-' && data_block[i+11]=='4')
    {
        data_block[i+10]='\0';
        data_block[i+20]='\0';
        longitude = atof(&(((char*)data_block)[i+1]));
        latitude = atof(&(((char*)data_block)[i+11]));

        if(longitude>73.5 && latitude>40)
        {
            longcell = floor( (longitude-73.5)/COORD_STEP );
            latcell = floor( (latitude-40)/COORD_STEP );
            if(longcell<500 && latcell<500)
            {
                //if(longcell<0 || latcell<0) cout<<"ADADA"<<endl;
                sum1[longcell*500 + latcell]++;
            }
        }
    }
    i+=TAIL_SAFE_OFFSET;
    sum1[250000]=1;

    //initialize
    pthread_mutex_init(&cpu_thread_mutex, NULL);
    pthread_mutex_init(&leftover_mutex, NULL);
    pthread_cond_init(&next_block_cond_var, NULL);
    chunk_size = STARTING_CHUNK_SIZE;  //block_size/100; //fix dynamically with timing to guess cash
    completed_previous_block=0; ii=0; last_comma=-1; buf_index=0;

    pthread_t threads[NUM_OF_THREADS];
    pthread_create(&threads[0], NULL, debs_thread, (void *)sum1);
    pthread_create(&threads[1], NULL, debs_thread, (void *)sum2);
    pthread_create(&threads[2], NULL, debs_thread, (void *)sum3);
    pthread_create(&threads[3], NULL, debs_thread, (void *)sum4);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);


    miniBINheap myHEAP;
    for(i=0; i<250000; i++)
    {
        myHEAP.replaceRootAndHeapify(sum1[i]+sum2[i]+sum3[i]+sum4[i], i);
        //ŝif(sum1[i]!=0) cout<<sum1[i] << "  " << i<<endl;
    }

    myHEAP.replaceRootAndHeapify(9999999,0);

    //for(i=0; i<30; i++)
    //{
     // cout << i+1 << ": (" << (myHEAP.heapArray[1]/500)+1 << "," << (myHEAP.heapArray[1]%500)+1 << ")"
     //               << "VALUE: " << myHEAP.heapArray[0] << "  ARRAY INDEX: " << myHEAP.heapArray[1] << endl;
     //   myHEAP.replaceRootAndHeapify(9999999,0);
    //}

    unsigned int tops[30];
    for(i=0; i<30; i++)
    {
        tops[i] = myHEAP.heapArray[1];
        myHEAP.replaceRootAndHeapify(9999999,0);
    }
    for(i=0; i<30; i++)
    {
        cout << i+1 << ": (" << (tops[i]/500)+1 << "," << (tops[i]%500)+1 << ")" << endl;
    }


    //for(i=0; i<2*HEAPSIZE; i++)
    //cout << myHEAP.heapArray[i] << endl;

    /*cout << "LINES: " << sum1[250000]+sum2[250000]+sum3[250000]+sum4[250000] << endl;
    cout << "SAMPLE FILE: 1,999,999 LINES" << endl << "COMPLETE FILE: 173,185,091 LINES" <<endl;
    cout<<"there is minor syncro (cache line?) problem and some lines (<15 for sample and <130) for complete will be missed"<<endl;
    cout<<"for sample file run a few times until 1,999,999 is reached"<<endl;*/



    delete myBuffer;
    pthread_mutex_destroy(&cpu_thread_mutex);
    pthread_mutex_destroy(&leftover_mutex);
    pthread_cond_destroy(&next_block_cond_var);
    pthread_exit(NULL);
}





void *debs_thread(void *args)
{
    int j,my_chunk_size;
    int longcell, latcell;
    float longitude,latitude;
    short int k;
    long int *sum;
    sum=(long int*)args;

    //long int file_size=block_size;

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
        if( last_comma > -1 && j==0)
        {
            pthread_mutex_lock(&leftover_mutex);
            j=ii;
            //complete reading
            while(last_comma<2)
            {
                if(data_block[j]==',') last_comma++;
                j++;
            }
            while(buf_index<BUF_SIZE)
            {
                buf[buf_index] = data_block[j];
                j++; buf_index++;
            }
            //calc   exp:  -73.956528,40.716976
            if(buf[0]=='-' && buf[11]=='4')
            {
                buf[10]='\0';
                buf[20]='\0';
                //cout << &buf[1] << "  "  << &buf[11] <<endl; fflush(stdout);
                longitude = atof(&(((char*)buf)[1]));        //strtof(&(((char*)buf)[1]), NULL);
                latitude = atof(&(((char*)buf)[11]));        //strtof(&(((char*)buf)[11]), NULL);
                //cout << longitude << "  "  << latitude <<endl; fflush(stdout);
                if(longitude>73.5 && latitude>40)
                {
                    longcell = floor( (longitude-73.5)/COORD_STEP );
                    latcell = floor( (latitude-40)/COORD_STEP );
                    //cout<<longcell << "  " <<latcell <<endl;
                    if(longcell<500 && latcell<500 && longcell>0 && latcell>0)
                    {
                        //if(longcell<0 || latcell<0) cout<<"ADADA1"<<endl;
                        sum[longcell*500 + latcell]++;
                    }
                }
            }
            j+=TAIL_SAFE_OFFSET;
            sum[250000]++;
            last_comma=-1;
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
                last_comma=0;
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
                    while(last_comma<2 && j<block_size)
                    {
                        if(data_block[j]==',') last_comma++;
                        j++;
                    }
                    if(j==block_size){pthread_mutex_unlock(&leftover_mutex); break;}

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
                while(k<2)
                {
                    if(data_block[j]==',') k++;
                    j++;
                }
                //calc   exp:  -73.956528,40.716976
                if(data_block[j]=='-' && data_block[j+11]=='4')
                {
                    data_block[j+10]='\0';
                    data_block[j+20]='\0';
                    //cout << &data_block[j+1] << "  "  << &data_block[j+11] <<endl; fflush(stdout);
                    longitude = atof(&(((char*)data_block)[j+1]));
                    latitude = atof(&(((char*)data_block)[j+11]));
                    //cout << setprecision(8) << longitude << "  "  << setprecision(8) << latitude <<endl; fflush(stdout);
                    if(longitude>73.5 && latitude>40)
                    {
                        longcell = floor( (longitude-73.5)/COORD_STEP );
                        latcell = floor( (latitude-40)/COORD_STEP );
                        if(longcell<500 && latcell<500)
                        {
                            //if(longcell<0 || latcell<0) cout<<"ADADA2"<<endl;
                            sum[longcell*500 + latcell]++;
                        }
                    }
                }
                sum[250000]++;
                j+=TAIL_SAFE_OFFSET;
            }
        }
    }
}
