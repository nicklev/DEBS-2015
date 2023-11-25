#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "debs2015_trials.h"

using namespace std;

#define FILENAME "sorted_data.csv"

int main(int argc, char** argv)
{
    ifstream inFile;
    if(argc==2)
    {
        inFile.open(argv[1], ios_base::in);
    }
    else
    {
        inFile.open(FILENAME, ios_base::in);
    }

    if(!(inFile.is_open()))
    {
        cout<<"bad filename"<<endl;
        return 1;
    }

    char line[2048];
    while(inFile.getline(line,2048))
    {
        proc_line_B(line);
        //char c; cout<<"enter q to quit"; cin >> c; if(c=='q') return 0;
        //system("PAUSE");
    }

    cout<<file_line+1<<endl;

    return 0;
}


