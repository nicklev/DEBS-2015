#include <time.h>

#ifndef DEBS_FIFO_Q1_H
#define DEBS_FIFO_Q1_H


#define ROUTE_KEY_TYPE long long int


struct route_queue
{
    struct route_queue *prev;
    struct route_queue *next;
    time_t expire_time;
    ROUTE_KEY_TYPE route_key;
};


class debs_fifo_q1
{
private:
    //FIFO queue, time-sorted
    struct route_queue *FIFO_head;
    struct route_queue *FIFO_tail;

    time_t *current_time;
public:
    debs_fifo_q1(time_t *cur_time);
    ~debs_fifo_q1();

    //FIFO functions
    void FIFO_push_front(time_t expire_time, ROUTE_KEY_TYPE route_key);
    ROUTE_KEY_TYPE FIFO_pop_back_if_expired();
};

#endif
