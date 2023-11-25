#ifndef HASHTABLE_H
#define HASHTABLE_H

//templates sucks with header files
#define KEY_TYPE long long int
#define VALUE_TYPE int
#define SIZE_TYPE int

struct bucket
{
    KEY_TYPE key;
    VALUE_TYPE value;
    /* bucket chain */
    struct bucket *next_bucket=NULL;
    /* descending sorted counter list */
    /* counter means that MIN_VALUE=1 */
    struct bucket *list_next=NULL;
    struct bucket *list_prev=NULL;
    //SIZE_TYPE list_index=-1;
    bool is_in_tops;
};


class hashtable
{
private:
    //table
    SIZE_TYPE table_size;
    struct bucket **table;

    //list
    SIZE_TYPE list_size;
    struct bucket *list_head;
    struct bucket *list_end;
    SIZE_TYPE tops;
    struct bucket *last_top;

    //list functions
    bool update_list_incd_bucket(struct bucket *_bucket);
    bool push_back_list(struct bucket *_bucket);

    bool update_list_decd_bucket(struct bucket *_bucket);
    bool remove_from_list(struct bucket *_bucket);

    void swap_list_buckets(struct bucket *right_bucket);


public:
    hashtable( SIZE_TYPE _size, SIZE_TYPE _tops );
    ~hashtable();

    //@  conventional functions
    //insert_pair(KEY_TYPE _key, VALUE_TYPE _val);
    //inc_value(KEY_TYPE _key);


    //@  target specific function
    //init_counter(KEY_TYPE _key);

    //will create and initialize counter to 1 if it does not exist
    //returns true if tops changed
    int force_inc_counter(KEY_TYPE _key);

    //will remove (key,value) pair if counter reaches 0
    //will return false if (key,value) does not exist
    //will return false if tops didn't changed
    //will return true if tops changed
    int force_dec_counter(KEY_TYPE _key);

    //@ hash function
    SIZE_TYPE hash_func(KEY_TYPE _key);

    //@ get tops in char stream format, buf[181] or more
    void get_tops(char *buf);

    //@ print buckets
    void print_table();
    void print_list();
};

#endif // HASHTABLE_H
