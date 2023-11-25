#ifndef HASHTABLE_COUNTER_H
#define HASHTABLE_COUNTER_H

#define HT_KEY_TYPE long long int
#define HT_VALUE_TYPE short int
#define HT_SIZE_TYPE int

#define HT_SIZE 1000000
#define HT_TOPS 10
#define HT_COUNTER_TABLER_SIZE 300

#define HT_GUARD_VALUE -1
#define HT_COUNTER_INIT 0

struct bucket
{
    HT_KEY_TYPE key=HT_GUARD_VALUE;
    HT_VALUE_TYPE value=HT_COUNTER_INIT;

    /* bucket chain */
    struct bucket *next_bucket=NULL;

    /* descending sorted counter list */
    /* counter means that MIN_VALUE=0 */
    struct bucket *list_next=NULL;
    struct bucket *list_prev=NULL;
    bool is_in_tops=false;
};

class hashtable_counter
{
    //table
    //HT_SIZE_TYPE table_size; //predefined
    struct bucket table[HT_SIZE];

    //list
    HT_SIZE_TYPE list_size=0; //no delete no size decrease, will go up to HT_TOPS no more
    bool tops_reached=false;
    struct bucket list_head; //=NULL;
    struct bucket list_tail; //=NULL;
    struct bucket *last_top=NULL;
    //HT_SIZE_TYPE tops; //predefined
    bool tops_changed=0;
    //[i][0]->counter leader,  [i][1]->counter tail
    struct bucket* leaders[HT_COUNTER_TABLER_SIZE][2];

    //temp variables
    struct bucket *temp_bucket;

    //list functions
    void inc_bucket_update_list(struct bucket *_bucket);
    void push_back_list(struct bucket *_bucket);

    void close_gap(struct bucket *_bucket);
    void squeeze_new_leader(struct bucket *prev_bucket, struct bucket *_bucket);

    void update_last_top_incd(struct bucket *prev_bucket, struct bucket *_bucket);
    void update_last_top_decd(struct bucket *prev_bucket, struct bucket *_bucket);

    //void update_list_decd_bucket(struct bucket *_bucket);
    //void remove_from_list(struct bucket *_bucket);
    //void swap_list_buckets(struct bucket *right_bucket);

public:
    hashtable_counter();
    //~hashtable_counter();

    //returns the pointer to tops_changed bool. NOT SAFE but efficient. one less function call
    bool* get_tops_changed_pointer();

    //increases a counter by 1, updates list, and calls push_front(struct bucket*, time_t)
    //if key doesn't exist then it inits the bucket and sets counter to 1, updates list and calls FIFO_push
    //also updates tops_changed bool
    void force_inc_counter(HT_SIZE_TYPE hashed_key, HT_KEY_TYPE _key, time_t expire_time_for_FIFO_call);

    //decreases a counter by 1, and updates list
    //also updates tops_changed bool
    void dec_counter_update_list(struct bucket *_bucket);

    //hash function
    HT_SIZE_TYPE hash_func(HT_KEY_TYPE _key);

    //get tops in char stream format, buf[181] or more
    void get_tops(char *buf);

    //print buckets
    void print_table();
    void print_list();

    //debugging function
    struct bucket *get_bucket_ptr(HT_KEY_TYPE _key);
    struct bucket **get_modified_bucket_pointer();

    //friend push_front(struct bucket*, time_t)
};
#endif
