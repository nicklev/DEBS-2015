#include <time.h>
#include "hashtable_counter.h"
#include "debs_FIFO_Q1.h"
using namespace std;



debs_fifo_q1::debs_fifo_q1(time_t *cur_time)
{
    current_time = cur_time;
    FIFO_head = NULL;
    FIFO_tail = NULL;
}


void debs_fifo_q1::FIFO_push_front(time_t expire_time, struct bucket *route_bucket)
{
    //make new route
    struct route_queue *route;
    route = new route_queue;
    route->expire_time = expire_time;
    route->route_bucket = route_bucket;

    //link to FIFO
    if(FIFO_head == NULL)
    {
        //empty FIFO
        FIFO_head = FIFO_tail = route;
        route->next=NULL;
        route->prev=NULL;
    }
    else
    {
        FIFO_head->prev = route;
        route->next = FIFO_head;
        route->prev = NULL;
        FIFO_head = route;
    }
}


struct bucket* debs_fifo_q1::FIFO_pop_back_if_expired()
{
    //check for empty list
    if(FIFO_tail == NULL) return NULL;

    if(FIFO_tail->expire_time <= *current_time)
    {
        struct bucket *_bucket = FIFO_tail->route_bucket;

        //remove FIFO_tail
        if(FIFO_tail->prev == NULL)
        {
            //one item left
            delete FIFO_tail;
            FIFO_tail = FIFO_head = NULL;
        }
        else
        {
            FIFO_tail = FIFO_tail->prev;
            delete FIFO_tail->next;
            FIFO_tail->next = NULL;
        }
        return _bucket;
    }
    else
    {
        return NULL;
    }
}
