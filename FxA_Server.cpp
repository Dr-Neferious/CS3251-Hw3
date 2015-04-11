//
// Created by alchaussee on 4/8/15.
//

#include <stdlib.h>
#include <iostream>
#include <boost/thread.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int port_to_bind;
string ip;
int port_NetEmu;


void parseArgs(int numArgs, const char* args[]);
void handleInput();

boost::thread inputThread;
boost::mutex m;
enum Cmd {none, terminate, window};
Cmd command = Cmd::none;
int window_size = 1;

int main(int argc, const char* argv[])
{
    parseArgs(argc, argv);

    inputThread = boost::thread(handleInput);
    int counter = 0;
    while(true)
    {
        counter++;
        if(counter%100000000==0)
            cout << "Running" << endl;
        m.lock();
        switch(command)
        {
            case Cmd::terminate:
                cout << "Stopping Server" << endl;
                break;
            case Cmd::window:
                cout << "Setting window size to " << window_size << endl;
                break;
            default:
                break;
        }
        if(command!=Cmd::none)
            command = Cmd::none;
        m.unlock();
    }
}

void handleInput()
{
    string cmd;
    while(true)
    {
        cin >> cmd;
        if (cmd.compare("terminate") == 0)
        {
            m.lock();
            command = Cmd::terminate;
            m.unlock();
        }
        else if(cmd.compare("window")==0)
        {
            string w;
            cin >> w;
            try
            {
                window_size = stoi(w);
                m.lock();
                command = Cmd::window;
                m.unlock();
            }
            catch(invalid_argument e)
            {
                cout << "Invalid window size" << endl;
            }
        }
        else
            cout << "Unrecognized Command" << endl;
    }
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
