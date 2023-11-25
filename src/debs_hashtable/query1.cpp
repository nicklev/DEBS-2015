#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include "query1.h"
using namespace std;


void read_time(char* t_str, struct tm *out_time);
int get_cell_longitude(char* c_str);
int get_cell_latitude(char* c_str);

long long int file_line;


bool proc_line(char *line, time_t *dropoff_time, short int *cells)
{
    //get time
    struct tm temp_time;
    read_time(&line[DROPOFF_TIME_OFFSET],&temp_time);
    *dropoff_time = mktime(&temp_time);

    //get coordinates
    short int i = TRIPTIME_OFFSET;
    short int commas=0;
    //consume 2 commas
    while(commas<2) if(line[i++] == ',') commas++;

    //verify coordinates are there
    commas=0;
    short int coord_start = i;
    short int len=0;
    while(commas<4)
    {
        len++;
        if(line[i++]==',')
        {
            commas++;
            if(commas%2==1 && len == LONGITUDE_LENGTH) len=0;
            else if(commas%2==0 && len == LATITUDE_LENGTH) len=0;
            else {commas=-1; break;}
        }
    }

    //get coordinates
    cells[0] = get_cell_longitude(&(line[coord_start]));
    cells[1] = get_cell_latitude(&(line[coord_start+LONGITUDE_LENGTH+1]));
    cells[2] = get_cell_longitude(&(line[coord_start+LONGITUDE_LENGTH+LATITUDE_LENGTH+2]));
    cells[3] = get_cell_latitude(&(line[coord_start+LONGITUDE_LENGTH+LATITUDE_LENGTH+LONGITUDE_LENGTH+3]));

    if(cells[0]<0 || cells[1]<0 || cells[2]<0 || cells[3]<0)
    {
        return false;
        //cout << cells[0] << "  " << cells[1]<<"  " <<  cells[2]<< "  " << cells[3] <<endl;
    }
    else return true;
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










/*


    char buf[80];
    strftime (buf,80,"%c",&temp_time);
    cout<< buf <<endl;



    */
