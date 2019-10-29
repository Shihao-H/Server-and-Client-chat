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
        void encode(char*, char*);
        int decode(char*, char*);
    
    
    private:
        int port;
        char* IP;
    
};


void Chat::client()
{
    
    //create buffers
    
    char recv_buffer[1000];
    char decode_buffer[141];
    char input_buffer[141];
    char encode_buffer[144];
    
    
    
    //setup a socket and connection tools
    struct hostent* host = gethostbyname(this->IP);
    sockaddr_in sendSockAddr;
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr =
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(this->port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;
        exit(0);
    }
    cout << "Connected to the server!" << endl;
    while(1)
    {
        cout << "You: ";
        string data;
        data = this->readIn();
        memset(&input_buffer, 0, sizeof(input_buffer));//clear the buffer
        memset(&encode_buffer, 0, sizeof(encode_buffer));
        
        strcpy(input_buffer, data.c_str());
        
        this->encode(input_buffer,encode_buffer);
        send(clientSd, encode_buffer, sizeof(encode_buffer), 0);
        
        cout << "Awaiting server response..." << endl;
        
        memset(&recv_buffer, 0, sizeof(recv_buffer));//clear the buffer
        memset(&decode_buffer, 0, sizeof(decode_buffer));
        
        recv(clientSd, recv_buffer, sizeof(recv_buffer), 0);

        int result=this->decode(recv_buffer,decode_buffer);
        if(result!=1)
        {
            exit(0);
        }
        cout << "Friend: " << decode_buffer << endl;
    }
    
    
}



void Chat::server()
{

    char recv_buffer[1000];
    char decode_buffer[141];
    char input_buffer[141];
    char encode_buffer[144];
    
    
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
   cout << "Waiting for a connection on port " << ServerPort << endl;
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

   while(1)
   {
       //receive a message from the client (listen)
       cout << "Awaiting client response..." << endl;
       memset(&recv_buffer, 0, sizeof(recv_buffer));
       memset(&decode_buffer, 0, sizeof(decode_buffer));
       
       recv(newSd, recv_buffer, sizeof(recv_buffer), 0);
       this->decode(recv_buffer,decode_buffer);
       
       cout << "Friend: " << decode_buffer << endl;
       cout << "You: ";
       string data;
       data = this->readIn();
       memset(&input_buffer, 0, sizeof(input_buffer)); //clear the buffer
       memset(&encode_buffer, 0, sizeof(encode_buffer));
       strcpy(input_buffer, data.c_str());
       
       this->encode(input_buffer,encode_buffer);
       //send the message to client
       send(newSd, encode_buffer, sizeof(encode_buffer), 0);
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
    cout<<"Port "<<this->port<<endl;
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

void Chat::encode(char* input_buffer, char* encode_buffer)
{
    uint16_t version_num = 457;
    uint16_t length = strlen(input_buffer);

    // First put version into encode_buffer
    memcpy(encode_buffer, &version_num, 2);

    // Next copy length into
    memcpy(encode_buffer + 2, &length, 2);

    // Last copy actual message, excluding the terminating 0
    memcpy(encode_buffer + 4, input_buffer, length);
}

int Chat::decode(char* recv_buffer, char* decode_buffer)
{
    uint16_t version_num = 0;
    memcpy(&version_num, recv_buffer, 2);
    if (version_num != 457)
    {
        cout << "Error: Recevied invalid version number. Program will now terminate." << endl;
        return 0;
    }

    // Next decode the length
    uint16_t length = 0;
    memcpy(&length, recv_buffer+2, 2);
    if (length < 0 || length > 140)
    {
        cout << "Error: Invalid length received. Program will now terminate." << endl;
        return 0;
    }

    // Last decode the message
    memcpy(decode_buffer, recv_buffer+4, length);
//    decode_buffer[length] = '\0'; // Terminate the string with 0 for security
    
    return 1;

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
                        cerr<<"Invalid port format! It should be allnumbers."<< endl;
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
