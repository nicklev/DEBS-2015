#include <iostream>
#include <cstring>
#include <stdio.h>
#include "hashtable_counter.h"
using namespace std;


hashtable_counter::hashtable_counter()
{
    //initialize leaders to NULL
    memset(leaders, 0, sizeof(leaders[0][0])*HT_COUNTER_TABLER_SIZE*2);

    //outside limits values
    list_head.value = HT_COUNTER_TABLER_SIZE;
    list_tail.value = -1;

    //put head and tail at leaders' table borders
    leaders[HT_COUNTER_TABLER_SIZE-1][0] = &list_head;
    leaders[HT_COUNTER_TABLER_SIZE-1][1] = &list_head;
    leaders[0][0] = &list_tail;
    leaders[0][1] = &list_tail;

    //link head and tail
    list_head.list_prev = &list_tail;
    list_tail.list_next = &list_head;
}

bool* hashtable_counter::get_tops_changed_pointer()
{
    return &tops_changed;
}

struct bucket** hashtable_counter::get_modified_bucket_pointer()
{
    return &temp_bucket;
};

HT_SIZE_TYPE hashtable_counter::hash_func(HT_KEY_TYPE _key)
{
    //return _key%table_size;
    _key = (1485500463*_key + 1179660437) % 2147483647;
    _key = (_key<0)?-_key:_key;
    return _key % HT_SIZE;
}


void hashtable_counter::force_inc_counter(HT_SIZE_TYPE hashed_key, HT_KEY_TYPE _key, time_t expire_time_for_FIFO_call)
{
    //check hashed_key
    //if(hashed_key<0 || hashed_key>=HT_SIZE) {cout<<"BAD HASHED KEY\n"<<endl; return;}

    temp_bucket = &(table[hashed_key]);

    //find correct bucket and update
    while(1)
    {
        if(temp_bucket->key == _key)
        {
            //key exists
            //@@@@@ increase value and update list
            inc_bucket_update_list(temp_bucket);
            break;
        }
        else if(temp_bucket->next_bucket == NULL)
        {
            //key doesn't exist, create bucket if needed
            if( temp_bucket->key != HT_GUARD_VALUE )
            {
                temp_bucket = (temp_bucket->next_bucket = new struct bucket);
            }
            //init new bucket and put infront of first counter=1 of the list
            temp_bucket->key = _key;
            temp_bucket->value = 1;
            //@@@@@ put in front of leader counter=1
            push_back_list(temp_bucket);
            break;
        }
        else
        {
            temp_bucket = temp_bucket->next_bucket;
        }
    }

    //@@@@@ call FIFO_push
    //FIFO_push_front(expire_time_for_FIFO_call, temp_bucket);
    return;
}


inline void hashtable_counter::push_back_list(struct bucket *_bucket)
{
    //could also put it at list_end and call update_list_incd_bucket()

    //if counter 1 has a leader
    if(leaders[1][0] != NULL)
    {
        squeeze_new_leader(leaders[1][0], _bucket);
    }
    //else counter 0 has at least one leader for sure (list_tail)
    else
    {
        squeeze_new_leader(leaders[0][0], _bucket);
    }

    //check for tops
    if(!tops_reached)
    {
        //do this for HT_TOPS first buckets
        list_size++;
        if(list_size >= HT_TOPS) tops_reached=true;

        _bucket->is_in_tops = true;
        last_top = list_tail.list_next;

        tops_changed = true;
    }
    else if( (_bucket->list_prev)->is_in_tops )
    {
        _bucket->is_in_tops = true;
        last_top->is_in_tops=false;
        last_top = last_top->list_next;

        tops_changed = true;
    }
}

inline void hashtable_counter::inc_bucket_update_list(struct bucket *_bucket)
{
    //swap needed
    if( (_bucket->list_next)->value <= (_bucket->value+1) )
    {
        //close gap
        close_gap(_bucket);

        //increase counter
        _bucket->value++;

        //new leader is not NULL
        if( leaders[_bucket->value][0] != NULL )
        {
            //update last top if needed
            update_last_top_incd(leaders[_bucket->value][0], _bucket);
            //squeeze
            squeeze_new_leader(leaders[_bucket->value][0], _bucket);
        }
        //new leader is NULL
        //that means there is still a leader of _bucket's old value since swap was needed
        else
        {
            //update last top if needed
            update_last_top_incd(leaders[_bucket->value-1][0], _bucket);
            //squeeze
            squeeze_new_leader(leaders[_bucket->value-1][0], _bucket);
        }
        return;
    }
    //else swap not needed
    else
    {
        //swap not needed means that
        //_bucket is leader of old counter and new counter is NULL

        //_bucket is the only one in this counter
        if(leaders[_bucket->value][0] == leaders[_bucket->value][1])
        {
            //update leader and tail
            leaders[_bucket->value][0] = NULL;
            leaders[_bucket->value][1] = NULL;
        }
        //else there's at least one more bucket in this counter
        else
        {
            leaders[_bucket->value][0] = _bucket->list_prev;
        }

        //increase counter
        _bucket->value++;

        //update new leader and tail
        leaders[_bucket->value][0] = _bucket;
        leaders[_bucket->value][1] = _bucket;
        return;
    }
}

void hashtable_counter::dec_counter_update_list(struct bucket *_bucket)
{
    //check if decrease is possible
    if(_bucket->value == 0) return;

    //swap needed
    if( (_bucket->list_prev)->value > (_bucket->value-1) )
    {
        //close gap
        close_gap(_bucket);

        //decrease counter
        _bucket->value--;

        //update last top if needed
        update_last_top_decd((leaders[_bucket->value+1][1])->list_prev, _bucket);

        //there's a tail for sure in _bucket's old counter else swap is not needed
        squeeze_new_leader( (leaders[_bucket->value+1][1])->list_prev, _bucket );
        return;
    }
    else
    {
        //swap not needed means that _bucket is the tail of it's counter
        //and it justs needs to be the leader of the previous counter

        //this bucket is the only in this counter
        if(leaders[_bucket->value][0] == leaders[_bucket->value][1])
        {
            //update leader and tail
            leaders[_bucket->value][0] = NULL;
            leaders[_bucket->value][1] = NULL;
        }
        //else there's at least one more bucket in this counter
        else
        {
            leaders[_bucket->value][1] = _bucket->list_next;
        }

        //decrease counter
        _bucket->value--;

        //update new leader and tail
        //counter is empty
        if(leaders[_bucket->value][0] == NULL)
        {
            leaders[_bucket->value][0] = _bucket;
            leaders[_bucket->value][1] = _bucket;
        }
        //there's at least one bucket in counter
        else
        {
            leaders[_bucket->value][0] = _bucket;
        }
        return;
    }
}

inline void hashtable_counter::close_gap(struct bucket *_bucket)
{
    //link gap
    (_bucket->list_next)->list_prev = _bucket->list_prev;
    (_bucket->list_prev)->list_next = _bucket->list_next;

    //update leader and tail
    //_bucket is the only bucket in this counter
    if(leaders[_bucket->value][0] == leaders[_bucket->value][1])
    {
        leaders[_bucket->value][0] = NULL;
        leaders[_bucket->value][1] = NULL;
    }
    //_bucket was the leader in this counter
    else if(leaders[_bucket->value][0] == _bucket)
    {
        leaders[_bucket->value][0] = _bucket->list_prev;
    }
    //_bucket was the tail in this counter
    else if(leaders[_bucket->value][1] == _bucket)
    {
        leaders[_bucket->value][1] = _bucket->list_next;
    }
}

inline void hashtable_counter::squeeze_new_leader(struct bucket *prev_bucket, struct bucket *_bucket)
{
    //link bucket
    _bucket->list_prev = prev_bucket;
    _bucket->list_next = prev_bucket->list_next;

    //link prev_bucket and _bucket->list_next
    prev_bucket->list_next = _bucket;
    (_bucket->list_next)->list_prev = _bucket;

    //update leader and tail
    //prev_bucket was leader in this counter
    if(_bucket->value == prev_bucket->value)
    //if(leaders[_bucket->value][0] == prev_bucket)
    {
        leaders[_bucket->value][0] = _bucket;
    }
    //else there's no leader in this counter
    else
    {
        leaders[_bucket->value][0] = _bucket;
        leaders[_bucket->value][1] = _bucket;
    }
}

inline void hashtable_counter::update_last_top_incd(struct bucket *prev_bucket, struct bucket *_bucket)
{
    //_bucket is in tops
    if(_bucket->is_in_tops)
    {
        //_bucket is last top
        if(_bucket == last_top)
        {
            last_top = _bucket->list_next;
        }
        tops_changed=true;
    }
    //_bucket is not in tops
    //prev_bucket is in tops
    else if(prev_bucket->is_in_tops)
    {
        _bucket->is_in_tops = true;
        last_top->is_in_tops = false;

        //move last top towards head
        //last top is prev_bucket
        if(last_top == prev_bucket)
        {
            last_top = _bucket;
        }
        //else last top is not prev_bucket
        else
        {
            last_top = last_top->list_next;
        }
        tops_changed=true;
    }
}

inline void hashtable_counter::update_last_top_decd(struct bucket *prev_bucket, struct bucket *_bucket)
{
    //prev bucket is in tops
    if(prev_bucket->is_in_tops)
    {
        tops_changed = true;
    }
    //_bucket is in tops
    else if(_bucket->is_in_tops)
    {
        //prev_bucket is the previous of last top
        if(prev_bucket->list_next == last_top)
        {
            //new last top is _bucket
            last_top = _bucket;
        }
        //else just move last top towards tail
        else
        {
            _bucket->is_in_tops = false;
            //move last top towards tail
            last_top = last_top->list_prev;
            last_top->is_in_tops = true;
        }
        tops_changed = true;
    }





}


void hashtable_counter::print_table()
{
    short int max_chain=0;
    const short int ch_table_size=20;
    HT_SIZE_TYPE chains_population[ch_table_size] = {0};
    bool print_t=false;

    if(HT_SIZE>40)
    {
        cout<<"Table size has " <<HT_SIZE << " indexes. Are you sure you want to print it? y/n: ";
        char c;
        cin >> c;
        if(c=='y') print_t=true;
        else print_t = false;
    }

    for(int i=0; i<HT_SIZE; i++)
    {
        temp_bucket = &table[i];
        short int chain_len=0;
        if(print_t) cout << "TABLE INDEX: " << i <<endl;

        if(temp_bucket->key != HT_GUARD_VALUE )
        {
            do
            {
                chain_len++;
                if(print_t)
                {
                    cout << "MEM_ADD: " << temp_bucket;
                    cout << "  KEY: " << temp_bucket->key << "  VALUE: " << temp_bucket->value
                         << "  NEXT_BUCKET: " <<temp_bucket->next_bucket <<endl;
                }
                temp_bucket = temp_bucket->next_bucket;
            }
            while(temp_bucket!=NULL);
        }
        else
        {
            if(print_t) cout << "MEM_ADD: " << temp_bucket << "EMPTY INDEX";
        }
        if(print_t) cout<<endl;

        if(chain_len<ch_table_size) chains_population[chain_len]++;
        if(max_chain<chain_len) max_chain=chain_len;
    }

    cout<<"BUCKET CHAIN COUNTERS: "<<endl;
    cout<<"MAX BUCKET CHAIN: " << max_chain <<endl<<endl;
    for(int i=0; i<ch_table_size; i++)
    {
        cout<<"CHAIN SIZE: " <<i<<"  POPULATION: "<<chains_population[i]<<endl;
    }
    cout<<endl;
}

void hashtable_counter::print_list()
{
    temp_bucket = list_head.list_prev;
    HT_SIZE_TYPE i = 1;
    int c=0;
    HT_VALUE_TYPE val=-1;


    while(temp_bucket!=NULL)//&& c<HT_TOPS+5)
    {
        if(temp_bucket->value<0) break;

        if(val != temp_bucket->value)
        {
            val = temp_bucket->value;
            cout << "#COUNTER VALUE: " << temp_bucket->value << "  COUNTER LEADER: " << leaders[val];
            cout << endl<<endl;
        }

        cout << i << ". MEM_ADD: " << temp_bucket;
        cout << "  KEY: " << temp_bucket->key << "  VALUE: " << temp_bucket->value << "  IN_TOPS: " << temp_bucket->is_in_tops;
        cout << "  PREV_LIST_BUCKET: " << temp_bucket->list_prev <<"  NEXT_LIST_BUCKET: " << temp_bucket->list_next;
        if(temp_bucket==last_top) cout <<"  LAST_TOP";
        cout <<endl<<endl;
        i++;
        c++;
        temp_bucket = temp_bucket->list_prev;
        cout <<endl<<endl;
    }
    cout << "LIST_END MEM: " << list_tail.list_next <<endl <<endl;
}


struct bucket *hashtable_counter::get_bucket_ptr(HT_KEY_TYPE _key)
{
    temp_bucket = &(table[hash_func(_key)]);

    //find correct bucket and update
    while(1)
    {
        if(temp_bucket->key == _key)
        {
            return temp_bucket;
        }
        else if(temp_bucket->next_bucket == NULL)
        {
            return NULL;
        }
        else
        {
            temp_bucket = temp_bucket->next_bucket;
        }
    }
}


inline void reverse_key(HT_KEY_TYPE key, short int *cells)
{
    //cells[4] = { x1,y1,x2,y2 }
    cells[3] = static_cast<short int>(key);
    cells[2] = static_cast<short int>(key>>16);
    cells[1] = static_cast<short int>(key>>32);
    cells[0] = static_cast<short int>(key>>48);
}

void hashtable_counter::get_tops(char *buf)
{
    // "xxx.xxx, " 20 times ==> 20*9+1=181
    //empty list
    if(list_head.list_prev == list_tail.list_prev)
    {
        buf[0]='\0';
        return;
    }

    struct bucket* _bucket = list_head.list_prev;
    short int i=0;
    short int cells[4];
    short int c=0;

    while(_bucket != last_top->list_prev && c<HT_TOPS)
    {
        reverse_key(_bucket->key, cells);
        i += sprintf (&(buf[i]), "%hd.%hd, %hd.%hd, ", cells[0], cells[1], cells[2], cells[3]);
        _bucket = _bucket->list_prev;
        c++;
    }
}



/*
//since this should be a very efficient hashtable, there's a
    //high probability that the key will be on the first bucket
    if(table[hashed_key].key == _key)
    {
        table[hashed_key].value++;
        //@@@@@ update_list_incd_bucket();
        return;
    }
    //else find the correct bucket or init one
    */


/*
constructor check
hashtable_counter::hashtable_counter()
{
    if(list_head != NULL || list_end != NULL || last_top != NULL || list_size != 0) cout << "bad var init" <<endl;

    for(int i=0; i<HT_COUNTER_TABLER_SIZE; i++)
    {
        if( leaders[i][0] != NULL ) cout<<"dasda"<<endl;
        if( leaders[i][1] != NULL ) cout<<"dasda"<<endl;
    }

    for(int i=0; i<HT_SIZE; i++)
    {
        if(table[i].key!=HT_GUARD_VALUE || table[i].value!=HT_COUNTER_INIT || table[i].next_bucket!=NULL || table[i].list_prev!=NULL || table[i].list_prev!=NULL || table[i].is_in_tops!=false)
        cout << "bad table init"<<endl;
    }
}
*/
