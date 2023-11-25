#include <iostream>
#include <map>
#include <list>
#include <time.h>
#include <string.h>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "debs2015_trials.h"

using namespace std;

//structure for empty taxi
struct empty_taxi
{
    char* medalion;
    int long_cell_index;
    int lat_cell_index;
    time_t expire_time;
    //struct tm expire_time;
    list<empty_taxi*>::iterator position_in_list;
};

//compare function for <map>
bool comp(char* lhs, char* rhs)
{
    int i=0;
    while( ((i<32) && lhs[i]==rhs[i]) )i++;
    if(i==32) return false;
    else return lhs[i] < rhs[i];
}

inline void read_time(char* t_str, struct tm *out_time);

inline int get_cell_longitude(char* c_str);
inline int get_cell_latitude(char* c_str);

//inline void update_empty_taxi(struct empty_taxi* strct, struct tm* time, int long_cell, int lat_cell);


/* file line counter */
long int file_line=-1;

/* 600x600 grid, counters for empty taxi's...[longitude][latitude] */
int map_grid[GRID_SIZE][GRID_SIZE] = {0};


/* for proc_line_B,  map for medalions, list fifo for 30minutes timeout, float for time */
map<char*,empty_taxi*,bool(*)(char*,char*)> medalion_map(comp);
list<empty_taxi*> empty_taxi_fifo;
//struct tm current_time;
time_t current_time;

//DAYLIGHT SAVING TIME?  If sorted then no problem






void proc_line_B(char* line)
{
    bool medal, timeb;
    int cell_long, cell_lat;
    struct tm temp_time;
    int i,j;

    //increase line counter
    file_line++;

    //verify medalion
    i=0;
    while(line[i]!=',')
    {
        if(line[i]=='\n') return;
        i++;
    }
    if(i!=32) medal=false;
    else medal=true;
    i++;

    //consume one comma
    while(line[i]!=',')
    {
        if(line[i]=='\n') return;
        i++;
    }
    i++; //2 commas
    //consume one comma
    while(line[i]!=',')
    {
        if(line[i]=='\n') return;
        i++;
    }
    i++; //3 commas

    //verify and load dropoff_datetime
    //,2013-01-01 00:00:00,
    j=0;
    while(line[i]!=',')
    {
        if(line[i]=='\n') return;
        j++;
        i++;
    }
    i++; //4 commas
    //j must be 19
    if(j==19)
    {
        fprintf(stdout,"L %ld %.19s\n", file_line, &line[i-20]);
        read_time(&line[i-20],&temp_time);
        if(temp_time.tm_year!=0) timeb=true;
        else timeb=false;
    }
    else
    {
        fprintf(stdout,"L %ld bad_dropoff_time\n", file_line);
        timeb=false;
    }

    //consume 4 commas
    j=0;
    while(j<4)
    {
        if(line[i]=='\n') return;
        else if(line[i]==',') j++;
        i++;
    }
    //8 commas

    //verify and read dropoff longitude
    //,-73.962440,40.715008,
    j=0;
    while(line[i]!=',')
    {
        if(line[i]=='\n') return;
        j++;
        i++;
    }
    i++; //9 commas
    if(j==10)
    {
        cell_long = get_cell_longitude(&line[i-11]);
    }
    else cell_long=-1;


    //verify and read dropoff latitude
    //,-73.962440,40.715008,
    if(cell_long>=0)
    {
        j=0;
        while(line[i]!=',')
        {
            if(line[i]=='\n') return;
            j++;
            i++;
        }
        if(j==9)
        {
            cell_lat = get_cell_latitude(&line[i-9]);
        }
        else cell_long=-1;
    }
    else cell_lat=-1;

    //update time
    if(timeb)
    {
        current_time = mktime(&temp_time);
    }
    //on first line time is correct so i won't handle time=false on first line


    //check if it was in empty map and/or list
    map<char*,struct empty_taxi*>::iterator iter;
    struct empty_taxi* current_taxi=NULL;
    if(medal)
    {
        line[32]='\0';
        iter = medalion_map.find(line);

        //if taxi is in map
        if(iter!=medalion_map.end())
        {
            current_taxi = iter->second;
            //remove from FIFO list if it's in there
            if(current_taxi->position_in_list != empty_taxi_fifo.end())
            {
                empty_taxi_fifo.erase(current_taxi->position_in_list);
            }

            //remove from map
            map_grid[current_taxi->long_cell_index][current_taxi->lat_cell_index]--;
	    if(map_grid[current_taxi->long_cell_index][current_taxi->lat_cell_index]<0) {sleep(10); cout<<"negative cell! problem"<<endl;}
            fprintf(stdout,"D %d %d %d\n", current_taxi->long_cell_index, current_taxi->lat_cell_index, map_grid[current_taxi->long_cell_index][current_taxi->lat_cell_index]);

            //update medalion_map   using current_time regardless if it was updated or not
            if(cell_lat>=0 && cell_long>=0)
            {
                current_taxi->expire_time = current_time+1800;
                current_taxi->long_cell_index = cell_long;
                current_taxi->lat_cell_index = cell_lat;
            }
            //remove struct from map
            else
            {
                delete current_taxi;
                delete[] iter->first;
                medalion_map.erase(iter);
                current_taxi = NULL;
            }
        }
        //if taxi is not in map
        else if(cell_lat>=0 && cell_long>=0)
        {
            //make struct
            current_taxi = new struct empty_taxi;
            char* medalion_key = new char[33];
            strcpy(medalion_key, line);

            current_taxi->expire_time = current_time+1800;
            current_taxi->lat_cell_index = cell_lat;
            current_taxi->long_cell_index = cell_long;
            current_taxi->medalion = medalion_key;

            medalion_map.insert(pair<char*,struct empty_taxi*>(medalion_key,current_taxi));
        }

        //if sturct ain't null then update fifo and map_grid
        if(current_taxi!=NULL)
        {
            map_grid[current_taxi->long_cell_index][current_taxi->lat_cell_index]++;
            fprintf(stdout,"P %d %d %d\n", current_taxi->long_cell_index, current_taxi->lat_cell_index, map_grid[current_taxi->long_cell_index][current_taxi->lat_cell_index]);
            empty_taxi_fifo.push_front(current_taxi);
            current_taxi->position_in_list = empty_taxi_fifo.begin();
            //current_taxi->position_in_list = empty_taxi_fifo.insert(empty_taxi_fifo.begin(), current_taxi);
        }
    }

    //30min check
    if(timeb)
    {
        //list<empty_taxi*>::reverse_iterator fifo_iter = empty_taxi_fifo.rbegin();
        //list<empty_taxi*>::iterator fifo_iter;
        struct empty_taxi* temp_taxi;
        while(!empty_taxi_fifo.empty())
        {
            temp_taxi = empty_taxi_fifo.back();
            if(current_time >= temp_taxi->expire_time)
            {
                //remove from fifo
                empty_taxi_fifo.pop_back();

                //remove from map_grid
                map_grid[temp_taxi->long_cell_index][temp_taxi->lat_cell_index]--;
		if(map_grid[temp_taxi->long_cell_index][temp_taxi->lat_cell_index]<0) {sleep(10); cout<<"negative cell! problem"<<endl;}

                //print line
                fprintf(stdout,"T %d %d %d\n", temp_taxi->long_cell_index, temp_taxi->lat_cell_index, map_grid[temp_taxi->long_cell_index][temp_taxi->lat_cell_index]);

                //remove from medalion_map
                medalion_map.erase(temp_taxi->medalion);

                //delete medalion and empty taxi struct
                delete[] temp_taxi->medalion;
                delete temp_taxi;
            }
            else
            {
                break;
            }
        }
    }

}















inline void read_time(char* t_str, struct tm *out_time)
{
    //init to 0
    memset(out_time, 0, sizeof(*out_time));
    //daytime saving time is not enable

    //load year
    out_time->tm_year = 1000*((int)t_str[0] - 48) + 10*((int)t_str[2] - 48) + ((int)t_str[3] - 48);
    if(out_time->tm_year>2014 || out_time->tm_year<2012)
    {
        cout<<"BAD YEAR AT LINE: " << file_line << endl;
        out_time->tm_year=0;
        return;
    }
    out_time->tm_year -= 1900;

    //load month
    out_time->tm_mon = 10*((int)t_str[5] - 48) + ((int)t_str[6] - 48) -1;
    if(out_time->tm_mon>11 || out_time->tm_mon<0)
    {
        cout<<"BAD MONTH AT LINE: " << file_line << endl;
        out_time->tm_year=0;
        return;
    }

    //load day
    out_time->tm_mday = 10*((int)t_str[8] - 48) + ((int)t_str[9] - 48);
    if(out_time->tm_mday>31 || out_time->tm_mday<1)
    {
        cout<<"BAD DAY AT LINE: " << file_line << endl;
        out_time->tm_year=0;
        return;
    }

    //load hour
    out_time->tm_hour = 10*((int)t_str[11] - 48) + ((int)t_str[12] - 48);
    if(out_time->tm_hour>23 || out_time->tm_hour<0)
    {
        cout<<"BAD HOURS AT LINE: " << file_line << endl;
        out_time->tm_year=0;
        return;
    }

    //load minutes
    out_time->tm_min = 10*((int)t_str[14] - 48) + ((int)t_str[15] - 48);
    if(out_time->tm_min>59 || out_time->tm_min<0)
    {
        cout<<"BAD MINUTES AT LINE: " << file_line << endl;
        out_time->tm_year=0;
        return;
    }

    //load seconds
    out_time->tm_sec = 10*((int)t_str[17] - 48) + ((int)t_str[18] - 48);
    if(out_time->tm_sec>59 || out_time->tm_sec<0)
    {
        cout<<"BAD SECONDS AT LINE: " << file_line << endl;
        out_time->tm_year=0;
        return;
    }
}








inline int get_cell_longitude(char* c_str)
{
    c_str[LONGITUDE_LENGTH]='\0';

    float longitude = (float)atof(c_str);
    if(longitude < LONGITUDE_BASE_LIMIT) return -1; //more WEST than borders

    int long_cell = floor( (fabs(longitude-LONGITUDE_BASE_LIMIT)) / LONGITUDE_STEP );
    if(long_cell>GRID_SIZE) return -1;
    return long_cell;
}

inline int get_cell_latitude(char* c_str)
{
    c_str[LATITUDE_LENGTH]='\0';

    float latitude = (float)atof(c_str);
    if(latitude > LATITUDE_BASE_LIMIT) return -1; //more WEST than borders

    int lat_cell = floor( (fabs(latitude-LATITUDE_BASE_LIMIT)) / LATITUDE_STEP );
    if(lat_cell>GRID_SIZE) return -1;
    return lat_cell;
}








/* testing map
    struct empty_taxi t1,t2,t3;
    char m1[33]= "taxi1",m2[33]= "taxi2",m3[33]= "taxi3";
    medalion_map.insert(pair<char*,empty_taxi*>(m1,&t1));
    medalion_map.insert(pair<char*,empty_taxi*>(m2,&t2));
    medalion_map.insert(pair<char*,empty_taxi*>(m3,&t3));


    cout<< "taxi1 => " << medalion_map.find(m1)->second << endl;
    cout<< "taxi2 => " << medalion_map.find(m2)->second << endl;
    cout<< "taxi3 => " << medalion_map.find(m3)->second << endl;
    */
/*
struct classcomp {
  bool operator() (const char* lhs, const char* rhs) const
  {
      int i=0;
      while( ((i<32) && lhs[i]==rhs[i]) )i++;
      if(i==32) return false;
      else return lhs[i] < rhs[i];
  }
};*/


/*
    char buf[80];
    strftime (buf,80,"%c",out_time);
    cout<< buf <<endl;
    */

/*
    cout<<"tm_sec "<<out_time->tm_sec<<endl;
    cout<<"tm_min "<<out_time->tm_min<<endl;
    cout<<"tm_hour "<<out_time->tm_hour<<endl;
    cout<<"tm_mday "<<out_time->tm_mday<<endl;
    cout<<"tm_mon "<<out_time->tm_mon<<endl;
    cout<<"tm_year "<<out_time->tm_year<<endl;
    cout<<"tm_wday "<<out_time->tm_wday<<endl;
    cout<<"tm_yday "<<out_time->tm_yday<<endl;
    cout<<"tm_isdst "<<out_time->tm_isdst<<endl;


    cout<<"tm_sec "<<temp_time.tm_sec<<endl;
    cout<<"tm_min "<<temp_time.tm_min<<endl;
    cout<<"tm_hour "<<temp_time.tm_hour<<endl;
    cout<<"tm_mday "<<temp_time.tm_mday<<endl;
    cout<<"tm_mon "<<temp_time.tm_mon<<endl;
    cout<<"tm_year "<<temp_time.tm_year<<endl;
    cout<<"tm_wday "<<temp_time.tm_wday<<endl;
    cout<<"tm_yday "<<temp_time.tm_yday<<endl;
    cout<<"tm_isdst "<<temp_time.tm_isdst<<endl;


*/












/*
07290D3599E7A0D62097A346EFCC1FB5,E7750A37CAB07D0DFF0AF7E3573AC141,2013-01-01 00:00:00,2013-01-01 00:02:00,120,0.44,-73.956528,40.716976,-73.962440,40.715008,CSH,3.50,0.510,0.50,0.00,0.00,4.50
22D70BF00EEB0ADC83BA8177BB861991,3FF2709163DE7036FCAA4E5A3324E4BF,2013-01-01 00:02:00,2013-01-01 00:02:00,0,0.00,0.000000,0.000000,0.000000,0.000000,CSH,27.00,0.00,0.50,0.00,0.00,27.50
0EC22AAF491A8BD91F279350C2B010FD,778C92B26AE78A9EBDF96B49C67E4007,2013-01-01 00:01:00,2013-01-01 00:03:00,120,0.71,-73.973145,40.752827,-73.965897,40.760445,CSH,4.00,0.50,0.50,0.00,0.00,5.00
1390FB380189DF6BBFDA4DC847CAD14F,BE317B986700F63C43438482792C8654,2013-01-01 00:01:00,2013-01-01 00:03:00,120,0.48,-74.004173,40.720947,-74.003838,40.726189,CSH,4.00,0.50,0.50,0.00,0.00,5.00
3B4129883A1D05BE89F2C929DE136281,7077F9FD5AD649AEACA4746B2537E3FA,2013-01-01 00:01:00,2013-01-01 00:03:00,120,0.61,-73.987373,40.724861,-73.983772,40.730995,CRD,4.00,0.50,0.50,0.00,0.00,5.00
5FAA7F69213D26A42FA435CA9511A4FF,00B7691D86D96AEBD21DD9E138F90840,2013-01-01 00:02:00,2013-01-01 00:03:00,60,0.00,0.000000,0.000000,0.000000,0.000000,CRD,2.50,0.50,0.50,0.25,0.00,3.75
DFBFA82ECA8F7059B89C3E8B93DAA377,CF8604E72D83840FBA1978C2D2FC9CDB,2013-01-01 00:02:00,2013-01-01 00:03:00,60,0.39,-73.981544,40.781475,-73.979439,40.784386,CRD,3.00,0.50,0.50,0.70,0.00,4.70
1E5F4C1CAE7AB3D06ABBDDD4D9DE7FA6,E0B2F618053518F24790C7FD0264E302,2013-01-01 00:03:00,2013-01-01 00:04:00,60,0.00,-73.993973,40.751266,0.000000,0.000000,CSH,2.50,0.50,0.50,0.00,0.00,3.50
468244D1361B8A3EB8D206CC394BC9E9,BB899DFEA9CC964B50C540A1D685CCFB,2013-01-01 00:00:00,2013-01-01 00:04:00,240,1.71,-73.955383,40.779728,-73.967758,40.760326,CSH,6.50,0.50,0.50,0.00,0.00,7.50
5F78CC6D4ECD0541B765FECE17075B6F,B7567F5BFD558C665D23B18451FE1FD1,2013-01-01 00:00:00,2013-01-01 00:04:00,240,1.21,-73.973000,40.793140,-73.981453,40.778465,CRD,6.00,0.50,0.50,1.30,0.00,8.30
6BA29E9A69B10F218C1509BEDD7410C2,ED368552102F12EA252C63782F12CD4C,2013-01-01 00:01:00,2013-01-01 00:04:00,180,0.74,-73.971138,40.758980,-73.972206,40.752502,CRD,4.50,0.50,0.50,0.00,0.00,5.50
75C90377AB710B04772D21ACBE3C52FB,00B7691D86D96AEBD21DD9E138F90840,2013-01-01 00:03:00,2013-01-01 00:04:00,60,0.00,0.000000,0.000000,0.000000,0.000000,CRD,3.00,0.50,0.50,0.07,0.00,4.07
C306CAC565429C12852164EB38175736,E255D5DFB9A967B9D55472A4D7ABA428,2013-01-01 00:01:00,2013-01-01 00:04:00,180,0.84,-73.942841,40.797031,-73.934540,40.797314,CSH,4.50,0.50,0.50,0.00,0.00,5.50
C4D6E189EF44EB83E2D058D67F490E3A,95B5B3CDA9EDC88AA395239589B57196,2013-01-01 00:03:00,2013-01-01 00:04:00,60,0.00,-73.989189,40.721924,0.000000,0.000000,CSH,2.50,0.50,0.50,0.00,0.00,3.50
DD467ED2E7DDB5C8B9B918469604E54F,1D7876BD692018A5BE422C420A5148EA,2013-01-01 00:01:00,2013-01-01 00:04:00,180,0.95,-73.976753,40.750706,-73.990089,40.750729,CSH,5.00,0.50,0.50,0.00,0.00,6.00
F8A0B52B22BB58B3C45E66CEE135C29D,00B7691D86D96AEBD21DD9E138F90840,2013-01-01 00:03:00,2013-01-01 00:04:00,60,0.00,-73.937637,40.758369,-73.937607,40.758350,CRD,2.50,0.50,0.50,0.25,0.00,3.75
120E700FE35B2DDBEA4D64CCCF02C808,3EDDD1433E2276DF9F9BDC3997C4BD47,2013-01-01 00:03:00,2013-01-01 00:05:00,120,0.52,-73.981972,40.752525,-73.985313,40.747738,CSH,4.00,0.50,0.50,0.00,0.00,5.00
256C6A3CA47F81497C1F5038438B54B2,8FA961982C3FEF4386FB05B4667A2580,2013-01-01 00:00:00,2013-01-01 00:05:00,300,0.13,-73.969841,40.797359,-73.961899,40.812466,CSH,6.50,0.50,0.50,0.00,0.00,7.50
655E773C92FA446353D5C8B7416BE818,B6EAE07E2AD023B387EE5F09BB2D89E3,2013-01-01 00:02:00,2013-01-01 00:05:00,180,1.55,-74.003197,40.733032,-74.012985,40.717377,CRD,6.00,0.50,0.50,1.62,0.00,8.62
76942C3205E17D7E7FE5A9F709D16434,25BA06A87905667AA1FE5990E33F0E2E,2013-01-01 00:00:00,2013-01-01 00:05:00,300,0.61,-73.955925,40.781887,-73.963181,40.777832,CSH,5.00,0.50,0.50,0.00,0.00,6.00
8B5F45807D8EC24DDFD9A7EDFFAEC138,577D1B6729EF9014CDBC949554EB4483,2013-01-01 00:00:00,2013-01-01 00:05:00,300,0.06,-73.984871,40.753723,-73.983849,40.754467,CSH,4.50,0.50,0.50,0.00,0.00,5.50
*/
