#include <stdio.h>
#include <iostream>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <random>
#include <chrono>
#include <thread>
#include <poll.h>
#include <sstream>
#include <netinet/in.h>

using namespace std;

// Space for buffers
#define MAXBUF 1024

char buf[MAXBUF];
int number;
vector<int> *break_size;
string location;
string hostname;
char ip[200];
bool flag = true;
int *total_download;

// http://www.pdf995.com/samples/pdf.pdf

// The given function takes in a hostname and returns its IP Address using getHostByName
void hostname_to_ip(const char * hname)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        exit(1);
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        strcpy(ip , inet_ntoa(*addr_list[i]) );
    }
}

// This function is used to parse the response of the server
// We have to parse this response inorder to get the File Size to be downloaded
int getFileSize(char inpu[])
{
    string input(inpu);
    size_t pos = input.find("Length");  // Because Content-Length occurs in Response
    string str = input.substr (pos);
    string out = "";
    for (int i = 7; i < str.length(); ++i)
    {
        if (str[i] == ' ')      // Ignoring White-Spaces
            continue;
        else
            out += str[i];
    }

    return stoi(out);       // Return Lenght
}

// This is the main worker function performed by each connection
// The environment is multi-threaded
void work(int id)
{

    char reply[10000];              // Buffer for reply
    struct sockaddr_in server;      // Server Struct
    int client_socket;             

    client_socket = socket(PF_INET , SOCK_STREAM , 0);      // Creating a Socket

    if(client_socket == -1)
    {
        printf("Problem creating socket\n");
        exit(1);
    }

    // Setting up the server struct
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    // Connecting with the server
    if (connect(client_socket , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("Error while making Connections\n EXITING\n");
        exit(1);
    }

    // This is the HTTP Request GET message 
    string message = "GET " + location + " HTTP/1.1 \r\nHost: " + hostname + "\r\nRange: bytes=";
    // INcluse the Range request
    message = message + to_string(break_size[id][0]) + "-" + to_string(break_size[id][1]) + "\r\n\r\n";

    // Send message
    if(send(client_socket , message.c_str() , message.length() , 0) < 0)
    {
        puts("Send failed");
        exit(1);
    }

    // Each connection downloads into a file
    string filename = "File" + to_string(id) + ".bin";
    ofstream temp;
    temp.open(filename, ios::app | ios::binary);

    int total_len;

    // Responses
    string response;
    while(recv(client_socket, reply , 1, 0) > 0)
    {
        response.push_back(reply[0]);
        if(response.size() > 3 && response.substr(response.size() - 4) == "\r\n\r\n")
            break;
    }

    // Downoad the file
    while(recv(client_socket, reply , 1, 0) > 0)
    {   
        temp.write(reply, 1);       // Write it into temp file
        total_download[id]++;       // Used for throughput
    }

    temp.close();       // Close file


    pthread_exit(NULL);
}


// Calculator Thread : Used for Throughtput
void calculate(int num)
{
    ofstream *thr;
    thr = new ofstream[num];
    for (int i = 0; i < num; ++i)
    {
        string fname = "Throughtput_" + to_string(i) + ".txt";
        thr[i].open(fname, ios::app);
    }

    while(flag)
    {
        for (int i = 0; i < num; ++i)
        {
            if(total_download[i] > 0)
                thr[i] << total_download[i] << "\n";
        }
        sleep(1);
    }


    for (int i = 0; i < num; ++i)
    {
        thr[i].close();
    }
}

// The function strips the URL of http:// and https://
string strip(string input)
{
    size_t pos = input.find("http");
    string str = input.substr (pos);
    string out = "";
    for (int i = 7; i < str.length(); ++i)
    {
        out += str[i];
    }
    if (out[0] == '/')
        return out.substr(1);
    return out;
}

// Main Function of the program
int main(int argc, char **argv)
{
    // Correct number of argument
    if(argc!=3)
    {
        printf("Usage: %s <url> <connections>\n",argv[0]);
        exit(0);
    }

   

    string url = argv[1];
    number = stoi(argv[2]);

    // Array to hold amount of information downloaded
    total_download = new int[number];
    fill(total_download, total_download + number, 0);   // Initialization

    url = strip(url);   // Strip

    hostname = url.substr(0,string(url).find('/'));     // Extract Hostname
    location = url.substr(string(url).find('/'));       // Extract file-path

	int socket_desc;

    char server_reply[10000];   

    struct sockaddr_in server;      // Server 

     //Create socket
    socket_desc = socket(PF_INET , SOCK_STREAM , 0);
	if(socket_desc == -1)
	{
		printf("Problem creating socket\n");
		exit(1);
	}

    hostname_to_ip(hostname.c_str());       // Get IP Of Hostname
    printf("%s resolved to %s\n" ,hostname.c_str() , ip);

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("Error while making Connections\n EXITING\n");
        exit(1);
    }
    cout << "Connected to " << ip << endl;

    // HTTP Head message
    string message = "HEAD " + location + " HTTP/1.1 \r\nHost: " + hostname + "\r\n\r\n";
    
    // Send the message to Server
    if(send(socket_desc , message.c_str() , message.length() , 0) < 0)
    {
        puts("Send failed");
        exit(1);
    }

    // Response
    int res = recvfrom(socket_desc,buf,MAXBUF,0,NULL,NULL);

    cout << "Head Response recieved from server\n";

    int filesize = getFileSize(buf);    // Get the size of the file

    cout << "Total File size is : " << filesize << " bytes" << endl;
    int partition_size = filesize/number;

    // Is used to get which part of the file is downloaded by each connection
    break_size = new vector<int> [number];

    for (int i = 0; i < number; ++i)
    {
        break_size[i].push_back(i*partition_size);
        break_size[i].push_back(((i+1)*partition_size) - 1);
    }
    break_size[number-1][1] = filesize;

    long long startTime = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);

    // Create Threads for each socket
    thread *workers = new thread[number+1];

    // One calculator
    workers[number] = thread(calculate,number);

    sleep(2);   // Make sure calculator thread is ready

    cout << "Opening Connections\n";

    for (int q = 0; q < number; ++q)
    {
        workers[q] = thread(work,q);
    }

    for(int q=0; q < number; q++)
        workers[q].join(); 
    flag = false;

    cout << "Connections Closed\n";
    workers[number].join();
    
    // Stiching the file

    string filename = "Output.bin";
    ofstream mainFile;
    mainFile.open(filename, ios::app | ios::binary);

    for (int i = 0; i < number; ++i)
    {
        std::ifstream tempInFile;
        tempInFile.open("File" + std::to_string(i) + ".bin", std::ios::binary | std::ios::in);//append temp files to main one
        std::copy(std::istreambuf_iterator<char>(tempInFile), std::istreambuf_iterator<char>( ), std::ostreambuf_iterator<char>(mainFile));

    }

    long long endTime = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);

    cout << "The total time taken is : " << endTime - startTime << endl;
    system("rm File*.bin");

}
