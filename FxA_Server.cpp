//
// Created by alchaussee on 4/8/15.
//

#include <stdlib.h>
#include <iostream>

using namespace std;

int port_to_bind;
string ip;
int port_NetEmu;

void parseArgs(int numArgs, const char* args[]);

int main(int argc, const char* argv[])
{
    parseArgs(argc, argv);

    string cmd;
    cin >> cmd;
    //do stuff based on the command
}

void parseArgs(int numArgs, const char* args[])
{
    if(numArgs>4)
    {
        cout << "Too many arguments" << endl;
        exit(EXIT_FAILURE);
    }
    else if(numArgs<4)
    {
        cout << "Not enough arguments" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        try
        {
            port_to_bind = stoi(args[1]);
            ip = stoi(args[2]);
            port_NetEmu= stoi(args[3]);
        }
        catch(...)
        {
            cout << "Invalid parameters" << endl;
            exit(EXIT_FAILURE);
        }
    }
}