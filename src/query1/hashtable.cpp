#include <iostream>
#include <string.h>
#include <stdio.h>

#include "hashtable.h"

using namespace std;


inline void reverse_key(KEY_TYPE key, short int *cells)
{
    //cells[4] = { x1,y1,x2,y2 }
    cells[3] = static_cast<short int>(key);
    cells[2] = static_cast<short int>(key>>16);
    cells[1] = static_cast<short int>(key>>32);
    cells[0] = static_cast<short int>(key>>48);
}


hashtable::hashtable(SIZE_TYPE _size, SIZE_TYPE _tops)
{
    if(_size<=0) table_size = 512;
    else table_size = _size;
    if(_tops<0) tops = 0;
    else tops = _tops;

    //create hashtable and init to 0
    table = new struct bucket*[table_size];

    for(int i=0; i<table_size; i++)
    {
        table[i] = NULL;
    }

    //init list
    list_head = NULL;
    list_end = NULL;
    last_top = NULL;
    list_size = 0;

//   memset(table, 0, sizeof(bucket)*table_size);
//   for(int i=0; i<table_size; i++)
//   {
//       if(table[i].key != -1) cout<<"key " << i<< " ain't zero" <<endl;
//       //if(table[i].value != 0) cout<<"value " << i<< " ain't zero" <<endl;
//       if(table[i].next_bucket != NULL) cout<<"pointer " << i<< " ain't null" <<endl;
//   }
}



int hashtable::force_inc_counter(KEY_TYPE _key)
{
    //get index and first bucket
    SIZE_TYPE index = hash_func(_key);

    //no bucket here ==> no _key here, create one and init counter to 1
    if(table[index] == NULL)
    {
        table[index] = new struct bucket;
        table[index]->key = _key;
        table[index]->value = 1;
        return push_back_list(table[index]);
    }

    //else there's at least one bucket
    struct bucket *temp_bucket = table[index];

    //search chain and if key exists increase counter
    while(1)
    {
        if(temp_bucket->key == _key)
        {
            temp_bucket->value++;
            return update_list_incd_bucket(temp_bucket);
        }
        else if(temp_bucket->next_bucket == NULL)
        {
            break;
        }
        else
        {
            temp_bucket = temp_bucket->next_bucket;
        }
    }

    //key does't exist, create pair
    temp_bucket->next_bucket = new struct bucket;
    (temp_bucket->next_bucket)->key = _key;
    (temp_bucket->next_bucket)->value = 1;
    return push_back_list(temp_bucket->next_bucket);
}



int hashtable::force_dec_counter(KEY_TYPE _key)
{
    //get index and first bucket
    SIZE_TYPE index = hash_func(_key);

    //if key doesn't exist return false
    if(table[index] == NULL) return false;

    struct bucket *temp_bucket = table[index];
    struct bucket *temp_bucket_prev = NULL;

    //search chain and if key exists decrease counter
    while(1)
    {
        //key found
        if(temp_bucket->key == _key)
        {
            //check for counter==0
            if( temp_bucket->value <= 1 )
            {
                //remove bucket from list
                bool tops_changed = remove_from_list(temp_bucket);

                //check if the bucket is pointed by the table
                if( temp_bucket_prev == NULL ) table[index] = temp_bucket->next_bucket;
                //else connect the gap
                else temp_bucket_prev->next_bucket = temp_bucket->next_bucket;
                delete temp_bucket;
                return tops_changed;
            }
            //counter>0
            else
            {
                temp_bucket->value--;
                return update_list_decd_bucket(temp_bucket);
            }
        }
        else if(temp_bucket->next_bucket == NULL)
        {
            //key does not exist
            return false;
        }
        else
        {
            temp_bucket_prev = temp_bucket;
            temp_bucket = temp_bucket->next_bucket;
        }
    }
}


void hashtable::print_table()
{
    struct bucket *temp_bucket;

    for(int i=0; i<table_size; i++)
    {
        cout << "TABLE INDEX: " << i <<endl;

        temp_bucket = table[i];
        while(temp_bucket!=NULL)
        {
            cout << "MEM_ADD: " << temp_bucket;
            cout << "  KEY: " << temp_bucket->key << "  VALUE: " << temp_bucket->value << "  NEXT_BUCKET: " <<temp_bucket->next_bucket <<endl;
            temp_bucket = temp_bucket->next_bucket;
        }
        cout<<endl;
    }
}

void hashtable::print_list()
{
    struct bucket *temp_bucket = list_head;
    SIZE_TYPE i = 1;
    int c=0;
    while(temp_bucket!=NULL && c<tops+10)
    {
        cout << i << ". MEM_ADD: " << temp_bucket;
        if(temp_bucket==last_top) cout <<"  LAST_TOP";
        cout << "  KEY: " << temp_bucket->key << "  VALUE: " << temp_bucket->value << "  IN_TOPS: " << temp_bucket->is_in_tops;
        cout << "  PREV_LIST_BUCKET: "<<temp_bucket->list_prev << "  NEXT_LIST_BUCKET: " << temp_bucket->list_next <<endl<<endl;

        temp_bucket = temp_bucket->list_next;
        i++;
        c++;
    }
    cout<<endl;
}




inline bool hashtable::update_list_incd_bucket(struct bucket *right_bucket)
{
    bool tops_changed = false;

    // counter was increased by 1 so only check towards head
    while(right_bucket->list_prev != NULL)
    {
        //check for swap
        if( (right_bucket->list_prev)->value < right_bucket->value )
        {
            swap_list_buckets(right_bucket);

            //check if tops changed
            if(right_bucket->is_in_tops) tops_changed=true;
        }
        //no more swaps will happen
        else
        {
            break;
        }
    }
    // return if tops changed
    return tops_changed;
}

inline bool hashtable::update_list_decd_bucket(struct bucket *left_bucket)
{
    bool tops_changed = false;

    // counter was decreased by 1 so only check towards end
    while(left_bucket->list_next != NULL)
    {
        //check for swap
        if( (left_bucket->list_next)->value > left_bucket->value )
        {
            //check if tops changed
            if(left_bucket->is_in_tops) tops_changed=true;

            swap_list_buckets(left_bucket->next_bucket);
        }
        //no more swaps will happen
        else
        {
            break;
        }
    }

    // check if tops changed
    return tops_changed;
}


inline void hashtable::swap_list_buckets(struct bucket *right_bucket)
{
    struct bucket *left_bucket = right_bucket->list_prev;

    //update tops, if both left_bucket and _bucket are in tops and _bucket is the last top
    if(right_bucket == last_top)
    {
        last_top = left_bucket;
    }
    //if only temp_bucket is in tops
    else if(left_bucket == last_top)
    {
        left_bucket->is_in_tops = false;
        right_bucket->is_in_tops = true;
        last_top = right_bucket;
    }

    //swap links
    right_bucket->list_prev = left_bucket->list_prev;
    left_bucket->list_prev = right_bucket;
    if(right_bucket->list_prev!=NULL) (right_bucket->list_prev)->list_next = right_bucket;

    left_bucket->list_next = right_bucket->list_next;
    right_bucket->list_next = left_bucket;
    if(left_bucket->list_next!=NULL) (left_bucket->list_next)->list_prev = left_bucket;

    //check if head or end changed
    if(right_bucket->list_prev == NULL) list_head = right_bucket;
    if(left_bucket->list_next == NULL) list_end = left_bucket;
}





inline bool hashtable::push_back_list(struct bucket *_bucket)
{
    //check if list is empty
    if(list_end==NULL)
    {
        list_head = list_end = _bucket;
    }
    //append to end of list
    else
    {
        //link the bucket
        list_end->list_next = _bucket;
        _bucket->list_prev = list_end;

        //update new list_end
        list_end = _bucket;
    }

    //increase list size
    list_size++;

    //check for tops, if yes the update last tops
    if(list_size <= tops)
    {
        last_top = list_end;
        _bucket->is_in_tops = true;
        return true;
    }
    else
    {
        _bucket->is_in_tops = false;
        return false;
    }
}


inline bool hashtable::remove_from_list(struct bucket *_bucket)
{
    //check if it has left neighbor
    if(_bucket->list_prev != NULL)
    {
        (_bucket->list_prev)->list_next = _bucket->list_next;
    }
    //else _bucket was the head
    else
    {
        list_head = _bucket->list_next;
    }

    //check if it has right neighbor
    if(_bucket->list_next != NULL)
    {
        (_bucket->list_next)->list_prev = _bucket->list_prev;
    }
    //else _bucket was the end
    else
    {
        list_end = _bucket->list_prev;
    }

    //decrease list size
    list_size--;

    //check if in tops and if yes move last_top towards the end
    if(_bucket->is_in_tops)
    {
        //check next if is available
        if(last_top->list_next != NULL)
        {
            last_top = last_top->list_next;
            last_top->is_in_tops = true;
        }
        //check if last_top is _bucket, if yes just move towards the left
        else if(last_top == _bucket)
        {
            last_top = last_top->list_prev;
        }
        //else last top stays as is
        return true;
    }
    else
    {
        return false;
    }
}



void hashtable::get_tops(char *buf)
{
    // "xxx.xxx, " 20 times ==> 20*9+1=181
    if(list_head==NULL)
    {
        buf[0]='\0';
        return;
    }

    struct bucket *_bucket = list_head;
    short int i=0;
    short int cells[4];
    short int c=0;

    while(_bucket != last_top->list_next && c<tops)
    {
        reverse_key(_bucket->key, cells);
        i += sprintf (&(buf[i]), "%hd.%hd, %hd.%hd, ", cells[0], cells[1], cells[2], cells[3]);
        _bucket = _bucket->list_next;
        c++;
    }
}





inline SIZE_TYPE hashtable::hash_func(KEY_TYPE _key)
{
    //return _key%table_size;
    _key = (1485500463*_key + 1179660437) % 2147483647;
    _key = (_key<0)?-_key:_key;
    return _key % table_size;
}










































/*
int hashtable::force_inc_counter(KEY_TYPE _key)
{
    //get index and first bucket
    SIZE_TYPE index = hash_func(_key);
    struct bucket *temp_bucket = &(table[index]);

    //search chain and if key exists increase counter
    while(1)
    {
        if(temp_bucket->key == _key)
        {
            return ++(temp_bucket->value);
        }
        else if(temp_bucket->next_bucket == NULL)
        {
            break;
        }
        else
        {
            temp_bucket = temp_bucket->next_bucket;
        }
    }

    //key doesn't exist, create pair
    if( temp_bucket->key != KEY_GUARD_VALUE )
    {
        temp_bucket = temp_bucket->next_bucket = new struct bucket;
    }
    temp_bucket->key = _key;
    return temp_bucket->value = 1;
}





int hashtable::force_dec_counter(KEY_TYPE _key)
{
    //get index and first bucket
    SIZE_TYPE index = hash_func(_key);
    struct bucket *temp_bucket = &(table[index]);
    struct bucket *temp_bucket_prev;

    //search chain and if key exists decrease counter
    while(1)
    {
        if(temp_bucket->key == _key)
        {
            //check for counter==0
            if( temp_bucket->value <= 1 )
            {
                //check if the bucket is part of the table
                if( temp_bucket == &(table[index]) )
                {
                    if(temp_bucket->next_bucket == NULL)
                    {
                        temp_bucket->key = KEY_GUARD_VALUE;
                    }
                    else
                    {
                        temp_bucket = table[index].next_bucket;
                        table[index].key = temp_bucket->key;
                        table[index].value = temp_bucket->value;
                        table[index].next_bucket = temp_bucket->next_bucket;
                        delete temp_bucket;
                    }
                }
                //else remove the bucket and connect the gap
                else
                {
                    temp_bucket_prev->next_bucket = temp_bucket->next_bucket;
                    delete temp_bucket;
                }
                return 0;
            }
            else
            {
                return --(temp_bucket->value);
            }
        }
        else if(temp_bucket->next_bucket == NULL)
        {
            return -1;
        }
        else
        {
            temp_bucket_prev = temp_bucket;
            temp_bucket = temp_bucket->next_bucket;
        }
    }
}
*/


