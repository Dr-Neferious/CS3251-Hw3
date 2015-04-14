/*
 * CS 3251 - Programming Homework 2
 * FxA Client Application using RxP
 * Matthew Barulic
 * Richard Chaussee
 */

#include <stdlib.h>
#include <iostream>
#include <boost/thread.hpp>
#include <fstream>
#include "RxPSocket.h"

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
bool isConnected = false;

int main(int argc, const char* argv[])
{
    parseArgs(argc, argv);

    cout << "calling connect" << endl;
    RxPSocket sock = RxPSocket::connect(ip, port_NetEmu, port_to_bind);
    cout << "connected" << endl;

    inputThread = boost::thread(handleInput);
    while(true)
    {
        m.lock();
        switch(command)
        {
            case Cmd::disconnect:
            {
                cout << "Disconnecting" << endl;
                sock.close();
                isConnected = false;
                m.unlock();
                exit(EXIT_SUCCESS);
            }
            case Cmd::window:
                cout << "Setting window size to " << window_size << endl;
                break;
            case Cmd::connecting:
            {
                cout << "Connecting to server" << endl;
                cout << "Connection Successful" << endl;
                isConnected = true;
            }
                break;
            case Cmd::get:
            {
                if(!isConnected)
                {
                    cout << "Not connected to server" << endl;
                    break;
                }
                cout << "Downloading \"" << file << "\"" << endl;
                cout << "Sending get" << endl;
                char s[] = "get";
                int bytesrecvd = 0;
                int res = 0;
                while(bytesrecvd<3)
                {
                    res = sock.send(s+bytesrecvd, 3-bytesrecvd);
                    bytesrecvd+=res;
                }

                cout << "Sending file name " << file.c_str() << endl;
                char *cstr = new char[file.length() + 1];
                strcpy(cstr, file.c_str());

                bytesrecvd = 0;
                while(bytesrecvd<file.size()+1)
                {
                    res = sock.send(cstr+bytesrecvd, file.size()+1-bytesrecvd);
                    bytesrecvd+=res;
                }
                ofstream fileS("Client_"+file, ios::binary);

                char b[8] = {0};
                bytesrecvd = 0;
                while(bytesrecvd<8)
                {
                    res = sock.recv(b+bytesrecvd, 8-bytesrecvd);
                    bytesrecvd+=res;
                }

                if (string((char *) b).compare("GodFile") == 0)
                {
                    char r[10] = {0};
                    bytesrecvd = 0;
                    while(bytesrecvd<10)
                    {
                        res = sock.recv(r+bytesrecvd, 10-bytesrecvd);
                        bytesrecvd+=res;
                    }

                    int length = stoi(string((char*)r));
                    cout << "File is " << length << " bytes long" << endl;

                    char *buffer = new char[length];
                    int bytesrecvd = 0;
                    while(bytesrecvd<length)
                    {
                        res = sock.recv(buffer, length);
                        if (res == -1) {
                            cout << "Error receiving file data" << endl;
                            sock.close();
                            break;
                        }
                        bytesrecvd+=res;
                        fileS.write(buffer, res);
                        buffer = buffer+res;
                        length-=res;
                    }
                    fileS.close();
                    cout << "File successfully downloaded" << endl;
                }
                else if (string((char *) b).compare("BadFile") == 0)
                {
                    cout << "Error file " << file.c_str() << " doesn't exist on server" << endl;
                    remove(("Client_"+file).c_str());
                }
                else
                {
                    cout << "Something happened with file status" << endl;
                }
            }
                break;
            case Cmd::post:
            {
                if(!isConnected)
                {
                    cout << "Not connected to server" << endl;
                    break;
                }
                cout << "Uploading \"" << file << "\"" << endl;

                char s[] = "pst";
                int res = 0;
                int bytesrecvd = 0;
                while(bytesrecvd<3)
                {
                    res = sock.send(s+bytesrecvd, 3-bytesrecvd);
                    bytesrecvd+=res;
                }

                ifstream fileS(file, ios::binary | ios::in);

                if (fileS.is_open()) {
                    char s[] = "GodFile";
                    bytesrecvd = 0;
                    while(bytesrecvd<7)
                    {
                        res = sock.send(s+bytesrecvd,7-bytesrecvd);
                        bytesrecvd+=res;
                    }

                    fileS.seekg(0, ios::end);
                    int length = fileS.tellg();
                    fileS.seekg(0, fileS.beg);
                    cout << "Length is " << length << endl;

                    char b[10];
                    strcpy(b, to_string(length).c_str());
                    bytesrecvd = 0;
                    while(bytesrecvd<10)
                    {
                        res = sock.send(b+bytesrecvd, 10-bytesrecvd);
                        bytesrecvd+=res;
                    }

                    char *cstr = new char[file.length() + 1];
                    strcpy(cstr, file.c_str());

                    bytesrecvd = 0;
                    while(bytesrecvd<file.size()+1)
                    {
                        res = sock.send(cstr+bytesrecvd, file.size()+1-bytesrecvd);
                        bytesrecvd+=res;
                    }

                    char *buffer = new char[length];
                    fileS.read(buffer, length);
                    int bytessent = 0;
                    int l = length;
                    while(bytessent<length)
                    {
                        res = sock.send(buffer, l);

                        bytessent+=res;
                        buffer+=res;
                        l-=res;
                    }
                    fileS.close();
                    cout << "File successfully uploaded" << endl;
                }
                else {
                    cout << "Error opening file probably doesn't exist" << endl;
                    char s[] = "BadFile";
                    bytesrecvd = 0;
                    while(bytesrecvd<7)
                    {
                        res = sock.send(s+bytesrecvd, 7-bytesrecvd);
                        bytesrecvd+=res;
                    }
                }
            }
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
            m.lock();
            cin >> file;
            command = Cmd::get;
            m.unlock();
        }
        else if(cmd.compare("post")==0)
        {
            m.lock();
            cin >> file;
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
