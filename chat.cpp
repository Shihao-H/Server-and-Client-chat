#include <iostream>
#include <unistd.h>
#include <string>
#include <cctype>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <random>

using namespace std;
  
class Chat
{
    public:
        void client();
        void server();
        void help();
        void setPort(int);
        void setIP(char*);
        vector<string> split(const string&, char);
        bool isNumber(const string&);
        bool isIP(string);
        string readIn();
        void encode(char*, char*, int);
        int decode(char*, char*);
    
    
    private:
        int port;
        char* IP;
        short version=457;
    
};


void Chat::client()
{
    
    //create a message buffer
   char msg[150];
    
    //setup a socket and connection tools
    struct hostent* host = gethostbyname(this->IP);
    sockaddr_in sendSockAddr;
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr =
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(this->port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "Connecting to server... ";
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;
        exit(0);
    }
    cout << "Connected!" << endl;
    cout << "Connected to a friend! You send first." << endl;
    while(true)
    {
        cout << "You: ";
        string data;
        data = this->readIn();
        
        memset(&msg, 0, sizeof(msg));
        
        uint16_t version_unit2 = ntohs(this->version);
        uint16_t message_unit2 = ntohs(data.length());
        memcpy((char*)&msg[0], &version_unit2, sizeof(version_unit2));
        memcpy((char*)&msg[2], &message_unit2, sizeof(message_unit2));
        memcpy((char*)&msg[4], data.c_str(), strlen(data.c_str()));

        send(clientSd, (char*)&msg, data.length()+4, 0);
        
        memset(&msg, 0, sizeof(msg));//clear the buffer
        recv(clientSd, (char*)&msg, sizeof(msg), 0);

        cout << "Friend: " << (char*)&msg[4] << endl;
    }   
}



void Chat::server()
{
   //buffer to send and receive messages with
   char msg[150];
    random_device seeder;
    mt19937 engine(seeder());
    uniform_int_distribution<int> dist(0, 65535);
    int ServerPort = dist(engine);
    
   //setup a socket and connection tools
   sockaddr_in servAddr;
   bzero((char*)&servAddr, sizeof(servAddr));
   servAddr.sin_family = AF_INET;
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(ServerPort);

   //open stream oriented socket with internet address
   //also keep track of the socket descriptor
   int serverSd = socket(AF_INET, SOCK_STREAM, 0);
   if(serverSd < 0)
   {
       cerr << "Error establishing the server socket" << endl;
       exit(0);
   }
   //bind the socket to its local address
   int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr,
       sizeof(servAddr));
   if(bindStatus < 0)
   {
       cerr << "Error binding socket to local address" << endl;
       exit(0);
   }
   cout << "Welcome to Chat!" << endl;
   cout << "Waiting for a connection on 127.0.0.1 port " << ServerPort << endl;
   //listen for up to 5 requests at a time
   listen(serverSd, 5);
   //receive a request from client using accept
   //we need a new address to connect with the client
   sockaddr_in newSockAddr;
   socklen_t newSockAddrSize = sizeof(newSockAddr);
   //accept, create a new socket descriptor to
   //handle the new connection with client
   int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
   if(newSd < 0)
   {
       cerr << "Error accepting request from client!" << endl;
       exit(1);
   }
   cout << "Found a friend! You receive first." << endl;
   while(true)
   {
       //receive a message from the client (listen)
       memset(&msg, 0, sizeof(msg));//clear the buffer
       recv(newSd, (char*)&msg, sizeof(msg), 0);       
       cout << "Friend: " << (char*)&msg[4]<< endl;
       cout << "You: ";
       string data;
       data = this->readIn();
       memset(&msg, 0, sizeof(msg));
       
        uint16_t version_unit = htons(this->version);
        uint16_t message_unit = htons(data.length());
        memcpy((char*)&msg[0], &version_unit, sizeof(version_unit));
        memcpy((char*)&msg[2], &message_unit, sizeof(message_unit));
        memcpy((char*)&msg[4], data.c_str(), strlen(data.c_str()));
        send(newSd, (char*)&msg,  data.length()+4, 0);
       
   }
}

void Chat::help()
{
    cout << "Usage: ./chat [OPTION...]" << endl;
    cout << "  -s=SERVER_IP\tSpecify the IP of the server to connect" <<endl;
    cout << "  -p=Port_Num\tSpecify the port number of the server" << endl;
    cout << "  -h\t\tDisplay this help message" << endl;
    cout << endl;
    cout << "To start a server, simply type ./chat" << endl;
    cout << endl;
    cout << "To start a client, you must use both -s and -p flags" << endl;
    cout << "  For example, ./chat -s 127.0.0.1 -p 60000" << endl;
    exit(0);
}

void Chat::setPort(int port)
{
    this->port=port;
}

void Chat::setIP(char* IP)
{
    this->IP=IP;
}

bool Chat::isNumber(const string& str)
{
    // std::find_first_not_of searches the string for the first character
    // that does not match any of the characters specified in its arguments
    return !str.empty() &&
        (str.find_first_not_of("[0123456789]") == std::string::npos);
}

// Function to split string str using given delimiter
vector<string> Chat::split(const string& str, char delim)
{
    auto i = 0;
    vector<string> list;

    auto pos = str.find(delim);
    
    while (pos != string::npos)
    {
        list.push_back(str.substr(i, pos - i));
        i = ++pos;
        pos = str.find(delim, pos);
    }
    
    list.push_back(str.substr(i, str.length()));
    
    return list;
}


bool Chat::isIP(string IP)
{
    // split the string into tokens
    vector<string> list = split(IP, '.');

    // if token size is not equal to four
    if (list.size() != 4)
        return false;

    // validate each token
    for (string str : list)
    {
        // verify that string is number or not and the numbers
        // are in the valid range
        if (!isNumber(str) || stoi(str) > 255 || stoi(str) < 0)
            return false;
    }

    return true;
}

string Chat::readIn()
{
    string temp;
    // Populate the input_buffer from user input
    while (true)
    {
        getline(cin,temp);
        if (temp.length() > 140)
        {
            cerr << "Error: Input should be no more than 140 letters."<<endl;
            cout << "You: ";
        }
        else if (temp.length() == 0)
        {
            cerr << "Error: There is no input."<<endl;
            cout << "You: ";
        }
        else
        {
            break;
        }
    }
    return temp;
}

//========================================================================

int main(int argc, char** argv)
{
    Chat chat;
    if(argc==1)
        chat.server();
    else if(argc==2)
    {
        if(strncmp(argv[1], "-h", 2)!=0)
        {
            cerr<<"Invalid parameter!"<< endl;
            exit(0);
        }
        else
        {
            chat.help();
        }
    }
    else if(argc==5)
    {
        int opt; bool pp=false; bool ss=false;
        while ((opt = getopt(argc, argv, "p:s:")) != -1)
        {
            switch (opt)
            {
                case 'p':
                    pp=true;
                    int port;
                    try
                    {
                        port = stoi(argv[optind-1]);
                    }
                    catch (exception &e)
                    {
                        cerr<<"Invalid port format! It should be all numbers."<< endl;
                        exit(0);
                    }
                    chat.setPort(port);
                    break;
                case 's':
                    ss=true;
                    if(chat.isIP(argv[optind-1]))
                    {
                        chat.setIP(argv[optind-1]);
                    }
                    else
                    {
                        cerr<<"Invalid IP format!"<< endl;
                        exit(0);
                    }
                    break;
                case '?':
                    cerr<<"Invalid argument!"<< endl;
                    exit(0);
                    break;
            }
        }
        if(pp&&ss)
        {
            chat.client();
        }
        else
        {
            cerr<<"Invalid arguments!"<< endl;
            exit(0);
        }
    }
    else
    {
        cerr<<"Argument length can only be 1,2 or 5!"<< endl;
        cerr<<"Type ./chat -h for further help information."<< endl;
        exit(0);
    }
    return 0;
}
