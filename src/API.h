
//IO API
//must have max size of buffer in memory
//must open the file in binary    int fd = open(char* filename, O_RDONLY);
//must call the read(int fd, char* buffer, size_t bytes_to_read);
//
constructor(filename, sizes...)
{
	//start pthread
	//start reading blocks immediately
	//increase block size dynamicly
	//last few blocks decrease size to avoid extra cpu time
}

//get block function
void get_block(char** block, int* size);
//returns size=-1 if eof
//a block is marked as free to write on when the get_block() function is called to get the next block







//PARSING
//???

//must put the parsed and other preprocessed information into a buffer





//BUFFER
//?

//must have a get line function
//use the io block to write on it and free it once the preprocessed data is used?






//hash table API
#define KEY_TYPE long long int
#define COUNTER_TYPE short int
#define HASHED_KEY int

//create hashtable
hashtable(size, tops);

//hash table and doubly link list bucket
struct bucket
{
	KEY_TYPE key;
	COUNTER_TYPE counter;  //short int
	//..pointers...
};

void clear_tops_changed_flag();
//clears the tops_changed_flag

void force_inc_counter(HASHED_KEY key);
//if key exists => increase counter and update list
//if key doesn't exist =>puts it into bucket, sets counter to 1 and increases list
//if top ten change then tops_changed_flag is set
void force_inc_counter(HASHED_KEY, time_t); //so it can call push_front() without return anything here


void dec_counter(struct bucket*);
//bucket MUST point somewhere else segm fault
//decreases the bucket's pointer and updates list
//if top ten change then tops_chagned_flag is set



//FIFO
struct FIFO_bucket
{
	time_t expire_time;
	struct bucket* pointer;

	//NO NEXT AND PREV POINTERS SINCE THIS IS AN ARRAY
	//NO next pointer
	//NO prev pointer
};

//depends on the implementation but it should look like this
FIFO_bucket* head;
FIFO_bucket* tail;


void clean_expired();
//while there are expired FIFO_buckets at the end of the FIFO call dec_counter(struct bucket* pointer)

void push_front( struct bucket*, time_t );
//caled by force_inc










//output thread





