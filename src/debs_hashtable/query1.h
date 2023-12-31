#include <time.h>

#ifndef QUERY1_H
#define QUERY1_H




#define TIME_LENGTH 19
#define PICKUP_TIME_OFFSET 66
#define DROPOFF_TIME_OFFSET 86
#define TRIPTIME_OFFSET 106
#define TIME_SPECIFIER "%.19s, %.19s, "

//based on bitbucket WIKI 2015-2-23 18:14
#define GRID_SIZE 300
#define LONGITUDE_LENGTH 10
#define LONGITUDE_BASE_LIMIT -74.913585
#define LONGITUDE_STEP 0.0059861167
#define LATITUDE_LENGTH 9
#define LATITUDE_BASE_LIMIT 41.474937
#define LATITUDE_STEP 0.0045024634

//process the line and returns dropoff time and the cells indexes cells[4] = { x1,y1,x2,y2 }
//if coordinates are corrupted return false
//else it is confident enough that dropoff time is always correct so returns true of coordinates are correct
extern bool proc_line(char *line, time_t *dropoff_time, short int *cells);

extern long long int file_line;





#endif // QUERY1_H







/*
    char pickup_time[20]; // 2013-01-01 00:00:00\0
    char dropoff_time[20];


07290D3599E7A0D62097A346EFCC1FB5,E7750A37CAB07D0DFF0AF7E3573AC141,2013-01-01 00:00:00,2013-01-01 00:02:00,120,0.44,-73.956528,40.716976,-73.962440,40.715008,CSH,3.50,0.50,0.50,0.00,0.00,4.50
22D70BF00EEB0ADC83BA8177BB861991,3FF2709163DE7036FCAA4E5A3324E4BF,2013-01-01 00:02:00,2013-01-01 00:02:00,0,0.00,0.000000,0.000000,0.000000,0.000000,CSH,27.00,0.00,0.50,0.00,0.00,27.50
0EC22AAF491A8BD91F279350C2B010FD,778C92B26AE78A9EBDF96B49C67E4007,2013-01-01 00:01:00,2013-01-01 00:03:00,120,0.71,-73.973145,40.752827,-73.965897,40.760445,CSH,4.00,0.50,0.50,0.00,0.00,5.00
1390FB380189DF6BBFDA4DC847CAD14F,BE317B986700F63C43438482792C8654,2013-01-01 00:01:00,2013-01-01 00:03:00,120,0.48,-74.004173,40.720947,-74.003838,40.726189,CSH,4.00,0.50,0.50,0.00,0.00,5.00
3B4129883A1D05BE89F2C929DE136281,7077F9FD5AD649AEACA4746B2537E3FA,2013-01-01 00:01:00,2013-01-01 00:03:00,120,0.61,-73.987373,40.724861,-73.983772,40.730995,CRD,4.00,0.50,0.50,0.00,0.00,5.00
5FAA7F69213D26A42FA435CA9511A4FF,00B7691D86D96AEBD21DD9E138F90840,2013-01-01 00:02:00,2013-01-01 00:03:00,60,0.00,0.000000,0.000000,0.000000,0.000000,CRD,2.50,0.50,0.50,0.25,0.00,3.75
*/
