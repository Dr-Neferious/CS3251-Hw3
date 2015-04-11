/*
 * CS 3251 - Programming Homework 2
 * FxA Server Application using RxP
 * Matthew Barulic
 * Richard Chaussee
 */

#include <stdlib.h>
#include <iostream>
#include <boost/thread.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

using namespace std;

int port_to_bind;
string ip;
int port_NetEmu;


void parseArgs(int numArgs, const char* args[]);
void handleInput();
void acceptConnection(int* connectSock, int* sock, bool* isConnecting);

boost::thread inputThread;
boost::mutex m;
enum Cmd {none, terminate, window};
Cmd command = Cmd::none;
int window_size = 1;

int main(int argc, const char* argv[])
{
    parseArgs(argc, argv);

    cout << "Starting server" << endl;
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sock==-1)
    {
        cout << "Error creating socket" << endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port_to_bind;
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))==-1)
    {
        cout << "Error binding socket" << endl;
        close(sock);
        exit(EXIT_FAILURE);
    }

    if(listen(sock, 5)==-1)
    {
        cout << "Error listening" << endl;
        close(sock);
        exit(EXIT_FAILURE);
    }

    cout << "Server started" << endl;


    inputThread = boost::thread(handleInput);
    boost::thread acceptThread;
    int connectSock;
    bool isConnecting = false;
    while(true)
    {
        if(!isConnecting)
            acceptThread = boost::thread(acceptConnection, &connectSock, &sock, &isConnecting);

        m.lock();
        switch(command)
        {
            case Cmd::terminate:
                cout << "Stopping Server" << endl;
                m.unlock();
                pthread_cancel(acceptThread.native_handle());
                if(shutdown(connectSock, SHUT_RDWR)==-1)
                {
                    close(connectSock);
                    close(sock);
                    exit(EXIT_SUCCESS);
                }
                close(connectSock);
                (void) shutdown(sock, SHUT_RDWR);
                close(sock);
                exit(EXIT_SUCCESS);
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

void acceptConnection(int* connectSock, int* sock, bool* isConnecting)
{
    *isConnecting = true;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port_NetEmu;
    int res = inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if(res<0)
    {
        perror("Not a valid address family");
        close(*connectSock);
        exit(EXIT_FAILURE);
    }
    else if (res==0)
    {
        perror("Invalid ip address");
        close(*connectSock);
        exit(EXIT_FAILURE);
    }


    *connectSock = accept(*sock, NULL, NULL);
    if(*connectSock==-1)
    {
        cout << "Error accepting connection" << endl;
        close(*sock);
        exit(EXIT_FAILURE);
    }

    while(true) {
        char buf[3] = {0};
        res = recv(*connectSock, buf, 3, 0);
        if (res == 0)
            cout << "Connection shutdown" << endl;
        if (res == -1)
            cout << "Error receiving data" << endl;
        cout << "Got command " << string((char *) buf) << endl;

        if (string((char *) buf).compare("get") == 0) {
            char b[128] = {0};
            fill(begin(b), end(b), 1);
            int bytesrecvd = 0;
            while (bytesrecvd < 128 && find(begin(b), end(b), 0) == end(b)) {
                res = recv(*connectSock, b + bytesrecvd, 128 - bytesrecvd, 0);
                if (res == -1) {
                    cerr << "Error receiving file name " << string((char *) b) << endl;
                    cerr << strerror(errno) << endl;
                    close(*connectSock);
                    *isConnecting = false;
                    return;
                }
                bytesrecvd += res;
            }
            cout << "File is \"" << string((char *) b) << "\"" << endl;

            ifstream file(string((char *) b), ios::binary | ios::in);
            if (file.is_open()) {
                int res = send(*connectSock, "GodFile", 8, 0);
                if (res == -1) {
                    cout << "Error sending GoodFile" << endl;
                    close(*connectSock);
                    *isConnecting = false;
                    return;
                }
                file.seekg(0, ios::end);
                int length = file.tellg();
                file.seekg(0, file.beg);
                cout << "Length is " << length << endl;

                char b[10];
                strcpy(b, to_string(length).c_str());
                cout << b << endl;
                res = send(*connectSock, b, 10, 0);
                if (res == -1) {
                    cout << "Error sending file length" << endl;
                    close(*connectSock);
                    *isConnecting = false;
                    return;
                }

                char *buffer = new char[length];
                file.read(buffer, length);
				int bytessent = 0;
				int l = length;
				while(bytessent<length)
				{
		            res = send(*connectSock, buffer, l, 0);
		            if (res == -1) {
		                cout << "Error sending file" << endl;
		                close(*connectSock);
		                *isConnecting = false;
		                return;
		            }
					bytessent+=res;
					l-=res;
					buffer+=res;
				}
                file.close();
				cout << "File successfully sent" << endl;
            }
            else {
                cout << "Error opening file probably doesn't exist" << endl;
                int res = send(*connectSock, "BadFile", 7, 0);
                if (res == -1) {
                    cout << "Error sending BadFile" << endl;
                    close(*connectSock);
                    *isConnecting = false;
                    return;
                }
            }
        }
        else if (string((char *) buf).compare("pst") == 0) {

            char t[7] = {0};
            res = recv(*connectSock, t, 7, 0);
            if (res == -1) {
                cout << "Error receiving file status" << endl;
                close(*connectSock);
                *isConnecting = false;
                return;
            }

            if(string((char*)t).compare("GodFile")==0)
            {
                char l[10] = {0};
                int bytesrecvd = 0;
                while(bytesrecvd<10)
                {
                    res = recv(*connectSock, l, 10, 0);
                    if (res == -1) {
                        cout << "Error receiving file length" << endl;
                        close(*connectSock);
                        *isConnecting = false;
                        return;
                    }
                    bytesrecvd+=res;
                }
                int length = stoi(string((char *)l));
                cout << "File is " << length << " bytes long" << endl;

                char b[128] = {0};
                fill(begin(b), end(b), 1);
                bytesrecvd = 0;
                while (bytesrecvd < 128 && find(begin(b), end(b), 0) == end(b)) {
                    res = recv(*connectSock, b + bytesrecvd, 128 - bytesrecvd, 0);
                    if (res == -1) {
                        cerr << "Error receiving file name " << string((char *) b) << endl;
                        cerr << strerror(errno) << endl;
                        close(*connectSock);
                        *isConnecting = false;
                        return;
                    }
                    bytesrecvd += res;
                }
                cout << "File is " << string((char *) b) << " " << length << " bytes long" << endl;

                ofstream file("Server_"+string((char *) b), ios::binary);

                char *buffer = new char[length];
                bytesrecvd = 0;
                while(bytesrecvd<length)
                {
                    res = recv(*connectSock, buffer, length, 0);
                    if (res == -1) {
                        cout << "Error receiving file data" << endl;
                        close(*connectSock);
                        break;
                    }
                    bytesrecvd+=res;
					file.write(buffer, res);
					buffer = buffer+res;
					length-=res;
                }
                file.close();
				cout << "File successfully received" << endl;
            }
            else if(string((char*)t).compare("BadFile")==0)
            {
                cout << "File doesn't exist on client" << endl;
                continue;
            }
        }
        else {
            cout << "Connection lost" << endl;
            close(*connectSock);
            *isConnecting = false;
            return;
        }
        *isConnecting = false;
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
