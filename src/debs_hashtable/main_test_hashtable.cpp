#include <iostream>
#include <cstdlib>
#include <time.h>
#include "hashtable_counter.h"
using namespace std;


int main(int argc, char** argv)
{
    hashtable_counter *ht = new hashtable_counter();
    bool *tops_changed = ht->get_tops_changed_pointer();



    long long int key=1;
    int hkey;
    for(int i=0; i<15; i++)
    {
        hkey = ht->hash_func(key);
        ht->force_inc_counter(hkey,key,3);
        //ht->print_table();
        key+=1;
        //system("PAUSE");
    }
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(6),6,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(9),9,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(11),11,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(11),11,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;


    ht->dec_counter_update_list(ht->get_bucket_ptr(11));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->dec_counter_update_list(ht->get_bucket_ptr(11));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->dec_counter_update_list(ht->get_bucket_ptr(11));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->dec_counter_update_list(ht->get_bucket_ptr(6));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->dec_counter_update_list(ht->get_bucket_ptr(6));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->dec_counter_update_list(ht->get_bucket_ptr(14));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

 return 0;

    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(33),33,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(35),35,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->force_inc_counter(ht->hash_func(36),36,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;



    ht->force_inc_counter(ht->hash_func(35),35,9);
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;



    ht->dec_counter_update_list(ht->get_bucket_ptr(35));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;


    ht->dec_counter_update_list(ht->get_bucket_ptr(35));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;


    ht->dec_counter_update_list(ht->get_bucket_ptr(33));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    ht->dec_counter_update_list(ht->get_bucket_ptr(36));
    ht->print_list();
    system("PAUSE"); cout<<endl<<endl<<endl<<endl<<endl;

    return 0;
}














/*
    long long int key=4;
    int hkey;
    for(int i=0; i<10; i++)
    {
        hkey = ht->hash_func(key);

        ht->force_inc_counter(hkey,key,3);
        //ht->print_table();
        key+=7;
        //system("PAUSE");
    }

    key=4;
    for(int i=0; i<10; i++)
    {
        hkey = ht->hash_func(key);

        ht->force_inc_counter(hkey,key,3);
        //ht->print_table();
        key+=7;
        //system("PAUSE");
    }

    ht->print_table();
    system("PAUSE");

    ht->force_inc_counter(ht->hash_func(25),25,9);
    ht->force_inc_counter(ht->hash_func(60),60,9);
    ht->print_table();
    system("PAUSE");

    ht->force_inc_counter(ht->hash_func(25),25,9);
    ht->force_inc_counter(ht->hash_func(60),60,9);
    ht->print_table();
    system("PAUSE");
    */
