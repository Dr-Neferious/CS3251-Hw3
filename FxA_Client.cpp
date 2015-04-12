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
#include <fstream>

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

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sock==-1)
    {
        cout << "Error creating socket" << endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port_to_bind;
    int res = inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if(res<0)
    {
        perror("Not a valid address family");
        close(sock);
        exit(EXIT_FAILURE);
    }
    else if (res==0)
    {
        perror("Invalid ip address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    inputThread = boost::thread(handleInput);
    while(true)
    {
        m.lock();
        switch(command)
        {
            case Cmd::disconnect:
                {
                    cout << "Disconnecting" << endl;
                    (void) shutdown(sock, SHUT_RDWR);
                    close(sock);
                    isConnected = false;
                    m.unlock();
                    exit(EXIT_SUCCESS);
                }
                break;
            case Cmd::window:
                cout << "Setting window size to " << window_size << endl;
                break;
            case Cmd::connecting:
                {
                    cout << "Connecting to server" << endl;
                    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
                        cout << "Error connection failed" << endl;
                        close(sock);
                        m.unlock();
                        exit(EXIT_FAILURE);
                    }
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
                    res = send(sock, "get", 3, 0);
                    if (res == -1)
                    {
                        cout << "Error sending get" << endl;
                        close(sock);
                        break;
                    }

                    cout << "Sending file name " << file.c_str() << " " << sizeof(file.c_str()) << endl;
                    res = send(sock, file.c_str(), file.size()+1, 0);
                    if (res == -1)
                    {
                        cout << "Error sending filename" << endl;
                        close(sock);
                        break;
                    }
                    ofstream fileS("Client_"+file, ios::binary);

                    char b[8] = {0};
                    res = recv(sock, b, 8, 0);
                    if (res == -1)
                    {
                        cout << "Error receiving file status" << endl;
                        close(sock);
                        break;
                    }
                    cout << b << endl;
                    if (string((char *) b).compare("GodFile") == 0)
                    {
                        char r[10] = {0};
                        int res = recv(sock, r, 10, 0);
                        if (res == -1) {
                            cout << "Error receiving file length" << endl;
                            close(sock);
                            break;
                        }
                        cout << "Length is " << string((char*)r) << " " << res << endl;
                        int length = stoi(string((char*)r));
                        cout << "File is " << length << " bytes long" << endl;

                        char *buffer = new char[length];
                        int bytesrecvd = 0;
                        while(bytesrecvd<length)
                        {
                            res = recv(sock, buffer, length, 0);
                            if (res == -1) {
                                cout << "Error receiving file data" << endl;
                                close(sock);
                                break;
                            }
                            bytesrecvd+=res;
                        }
                        fileS.write(buffer, length);
                        fileS.close();
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

                    res = send(sock, "pst", 3, 0);
                    if (res == -1)
                    {
                        cout << "Error sending pst" << endl;
                        close(sock);
                        break;
                    }

                    ifstream fileS(file, ios::binary | ios::in);

                    if (fileS.is_open()) {
                        int res = send(sock, "GodFile", 8, 0);
                        if (res == -1) {
                            cout << "Error sending GodFile" << endl;
                            close(sock);
                            break;
                        }
                        fileS.seekg(0, ios::end);
                        int length = fileS.tellg();
                        fileS.seekg(0, fileS.beg);
                        cout << "Length is " << length << endl;

                        char b[10];
                        strcpy(b, to_string(length).c_str());
                        res = send(sock, b, 10, 0);
                        if (res == -1) {
                            cout << "Error sending file length" << endl;
                            close(sock);
                            break;
                        }

                        res = send(sock, file.c_str(), file.size()+1, 0);
                        if (res == -1)
                        {
                            cout << "Error sending filename" << endl;
                            close(sock);
                            break;
                        }

                        char *buffer = new char[length];
                        fileS.read(buffer, length);
                        res = send(sock, buffer, length, 0);
                        if (res == -1) {
                            cout << "Error sending file" << endl;
                            close(sock);
                            break;
                        }
                        fileS.close();
                    }
                    else {
                        cout << "Error opening file probably doesn't exist" << endl;
                        int res = send(sock, "BadFile", 7, 0);
                        if (res == -1) {
                            cout << "Error sending BadFile" << endl;
                            close(sock);
                            break;
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
