#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include "hashtable.h"
#include "query1.h"
#include "debs_FIFO_Q1.h"
using namespace std;


//#define FILENAME "/home/asdf/Documents/DEBS2015/sorted_data.csv"
#define FILENAME "C:\\sorted_data.csv"

//hash table definitions
#define HT_SIZE 100000
#define HT_TOPS 10

//30min window
#define MIN30TOSEC 1800

//key <-> cell indexes    ,,   cells[4] = { x1,y1,x2,y2 }
long long int get_key(short int *cells);
void reverse_key(long long int key, short int *cells);

//hashtable
hashtable *ht;

//FIFO
debs_fifo_q1 *FIFO;

//timing
time_t current_time;
clock_t delay_start, delay_end;

clock_t start, ht_inc, ht_dec, FIFO_push, FIFO_pop, p_line, output;
long long int c_ht_inc=0, c_ht_dec=0, c_FIFO_push=0, c_FIFO_pop=0, c_p_line=0, c_output=0;


//counters, indicators, other variables
char line[2048];


int main(int argc, char** argv)
{
    ifstream inFile;
    if(argc==2)
    {
        inFile.open(argv[1], ios_base::in);
    }
    else
    {
        inFile.open(FILENAME, ios_base::in);
    }

    if(!(inFile.is_open()))
    {
        cout<<"bad filename"<<endl;
        return 1;
    }

    //create hashtable
    ht = new hashtable(HT_SIZE, HT_TOPS);

    //create FIFO
    FIFO = new debs_fifo_q1(&current_time);

    file_line=-1;

    //read line by line
    //int i=0;
    delay_start = clock();
    while(inFile.getline(line,2048) ) //&& i++<100
    {
        short int cells[4];
        long long int key;
        bool tops_changed=false;

        //increase line
        file_line++;

        //process line
        c_p_line++; start = clock();
        if(proc_line(line, &current_time, cells))
        {
            p_line += clock()-start; start=0;
            key = get_key(cells);
            //add to hashtable
            c_ht_inc++; start=clock(); tops_changed = ht->force_inc_counter(key); ht_inc+=clock()-start;

            //add to FIFO
            //push into FIFO
            c_FIFO_push++; start=clock(); FIFO->FIFO_push_front(current_time+MIN30TOSEC, key); FIFO_push+=clock()-start;
        }
        if(start!=0) p_line+=clock()-start;


        //pop for 30min window
        c_FIFO_pop++; start=clock();
        while( (key=FIFO->FIFO_pop_back_if_expired()) >= 0)
        {
            FIFO_pop+=clock()-start;

            c_ht_dec++; start=clock(); tops_changed = tops_changed || ht->force_dec_counter(key); ht_dec+=clock()-start;

            start=clock();
        }
        FIFO_pop+=clock()-start;

        if(tops_changed)
        {
            c_output++; start=clock();
            delay_end = clock();
            char buf[200];
            //int short l = sprintf(buf, TIME_SPECIFIER,&(line[PICKUP_TIME_OFFSET]), &(line[DROPOFF_TIME_OFFSET]));
            ht->get_tops(&(buf[0]));
            line[PICKUP_TIME_OFFSET+TIME_LENGTH] = '\0';
            line[DROPOFF_TIME_OFFSET+TIME_LENGTH] = '\0';
            cout << &line[PICKUP_TIME_OFFSET] << ", " << &line[DROPOFF_TIME_OFFSET] << ", " << buf << std::setprecision(10) << double(delay_end - delay_start)/CLOCKS_PER_SEC <<endl<<endl<<endl;
            //system("PAUSE");
            delay_start = clock();
            output+=clock()-start;
        }
    }

    cout << "TIMING: " <<endl;
    cout << "proc_line(): CALLS: " << c_p_line << "  AVG_TIME: " << ((double)p_line / c_p_line) / CLOCKS_PER_SEC <<endl;
    cout << "ht_inc(): CALLS: " << c_ht_inc << "  AVG_TIME: " << ((double)ht_inc / c_ht_inc) / CLOCKS_PER_SEC <<endl;
    cout << "ht_dec(): CALLS: " << c_ht_dec << "  AVG_TIME: " << ((double)ht_dec / c_ht_dec) / CLOCKS_PER_SEC <<endl;
    cout << "FIFO_push(): CALLS: " << c_FIFO_push << "  AVG_TIME: " << ((double)FIFO_push / c_FIFO_push) / CLOCKS_PER_SEC <<endl;
    cout << "FIFO_pop(): CALLS: " << c_FIFO_pop << "  AVG_TIME: " << ((double)FIFO_pop / c_FIFO_pop) / CLOCKS_PER_SEC <<endl;
    cout << "output: CALLS: " << c_output << "  AVG_TIME: " << ((double)output / c_output) / CLOCKS_PER_SEC <<endl;

    //ht->print_table();
    //system("PAUSE");
    //ht->print_list();

    //delete ht;
    //delete FIFO
    return 0;
}




inline long long int get_key(short int *cells)
{
    //cells[4] = { x1,y1,x2,y2 }
    return ((static_cast<long long int>(cells[0]))<<48) + ((static_cast<long long int>(cells[1]))<<32)
            + (static_cast<long long int>(cells[2])<<16) + static_cast<long long int>(cells[3]);
}


inline void reverse_key(long long int key, short int *cells)
{
    //cells[4] = { x1,y1,x2,y2 }
    cells[3] = static_cast<short int>(key);
    cells[2] = static_cast<short int>(key>>16);
    cells[1] = static_cast<short int>(key>>32);
    cells[0] = static_cast<short int>(key>>48);
}









/* NOTES
change incd and decd functions to swap only once

change hashtable to table of *buckets instead of table of buckets ==> less space when table is almost empty and less delete time
except if delete won't take place. In this case table of *buckets ==>less space when table is full
if delete won't take place must keep a first_zero index to list for push_back because counter is 1 when push_back and also from 0->1 avoid list search for first non 0

reduce list to real O(1) if equal values are grouped


use print_table() to count chain length to optimize hash function

make function that takes hashed key directly so threads can do the hashing and the timed-bound critical section can be as fast a possible
idea: 3 threads read the file, do preprocessing and push into a buffer (time must be respected but if time is the same order doesn't matter)
1 thread does the data structure update, reading from the buffer, but it must be very very fast to compete the 3 threads that do the preprocessing
1 thread does the input, it's io-bound
1 thread does the output, it's io-bound

*/
