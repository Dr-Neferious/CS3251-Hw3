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
enum Cmd {none, connecting, window, post, get, disconnect};
Cmd command = Cmd::none;
int window_size = 1;
string file;

int main(int argc, const char* argv[])
{
    parseArgs(argc, argv);

    inputThread = boost::thread(handleInput);
    int counter = 0;
    while(true)
    {
        counter++;
        if(counter%1000000000==0)
            cout << "Running" << endl;
        m.lock();
        switch(command)
        {
            case Cmd::disconnect:
                cout << "Disconnecting" << endl;
                break;
            case Cmd::window:
                cout << "Setting window size to " << window_size << endl;
                break;
            case Cmd::connecting:
                cout << "Connecting to server" << endl;
                break;
            case Cmd::get:
                cout << "Downloading \"" << file << "\"" << endl;
                break;
            case Cmd::post:
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
        if(cmd.compare("connect")==0)
        {
            m.lock();
            command = Cmd::connecting;
            m.unlock();
        }
        else if(cmd.compare("get")==0)
        {
            cin >> file;
            m.lock();
            command = Cmd::get;
            m.unlock();
        }
        else if(cmd.compare("post")==0)
        {
            //Extra credit
            m.lock();
            command = Cmd::post;
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
        else if(cmd.compare("disconnect")==0)
        {
            m.lock();
            command = Cmd::disconnect;
            m.unlock();
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
            ip = args[2];
            port_NetEmu= stoi(args[3]);
        }
        catch(...)
        {
            cout << "Invalid parameters" << endl;
            exit(EXIT_FAILURE);
        }
    }
}
