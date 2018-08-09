#include <stdio.h>     
#include <stdlib.h>     
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h>
#include <iostream>
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

using namespace std;

#define MAXBUF 1024

int client_socket;
struct sockaddr_in server;
ofstream throughputFile, avgTimeFile;
const int pktSize = 512;
char buf[MAXBUF];
int buf_len;
int failure = 0, success = 0;
int sl;
long long sum_wait;
bool calculate;

void sender(int number)
{
	int n_sent;
	for (int i = 0; i < number; ++i)
	{
		char *filler = new char[pktSize - 16];
		std::fill(filler, filler + pktSize - 16, 'p');
		long long Time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);
		sprintf(buf, "%lld%s", Time, filler);
		buf_len = strlen(buf);

		printf("Sending %d bytes to server - Sending...\n",buf_len);

		n_sent = sendto(client_socket,buf,buf_len,0,(struct sockaddr*) &server,sizeof(server));

		if(n_sent<0)
		{
			perror("Error Error");
			exit(1);
		}
		usleep(sl); 
	}
	pthread_exit(NULL);
}

void reciever(int number)
{
	int n_read;
	struct pollfd fd;
	int ret;
	bool flag;
	for (int i = 0; i < number; i++)
	{
		flag = true;
		fd.fd = client_socket;
		fd.events = POLLIN;
		ret = poll(&fd, 1, 3000); 
		switch (ret) 
		{
		    case -1:
		    	printf("Some error has occured : No clue what to do\n"); 
		    	exit(1);
		        break;
		    case 0:
				printf("A time-out has occured while waiting for a packet : Moving on\n"); 
		        break;
		    default:
		        n_read = recvfrom(client_socket,buf,MAXBUF,0,NULL,NULL); // get data
		        long long rcv = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);
		        if (n_read<0) 
		        {
				    perror("Problem in recvfrom");
				    exit(1);
				}
		        printf("Got back %d bytes\n",n_read);
				fflush(stdout);
				flag = false;
				stringstream hold;
				hold << buf;
				long long sent;
				hold >> sent;
				sum_wait += rcv - sent;
				printf("RTT is : %lld microseconds\n", rcv - sent);
		        break;
		}

		if (flag)
			failure++;
		else
			success++;
	}
	pthread_exit(NULL);
}

void calc()
{
	while(calculate)
	{
		if(success != 0)
		{
			double avg_time = (sum_wait * 1.0) / (success);
			double throughput = (success * pktSize) / avg_time;
			throughputFile << throughput << "\n";
			avgTimeFile << avg_time << "\n";
			usleep(1000);
		}
	}
	pthread_exit(NULL);
}

int main( int argc, char **argv ) 
{
	if(argc!=3)
	{
		printf("Usage: %s <server name> <port number>\n",argv[0]);
		exit(0);
	}

	client_socket = socket(PF_INET, SOCK_DGRAM, 0);

	if(client_socket == -1)
	{
		printf("Problem creating socket\n");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons(atoi(argv[2]));
  	//memset(server.sin_zero, '\0', sizeof server.sin_zero);

	throughputFile.open("tp_output.txt", ios::out);
	avgTimeFile.open("avg_time_output.txt", ios::out);

	sl = 100000;
	int number = 10;
  	for(int i = 0; i < 5; i++)
	{
		sum_wait = 0;
		failure = 0;
		success = 0;

		thread *children = new thread[3];

		children[0] = thread(sender, number);
  		children[1] = thread(reciever, number);
  		children[2] = thread(calc);

		calculate = true;

		for(int i=0; i < 2; i++)
	    {
	        children[i].join();					// Joining the exited threads
	    }

		calculate = false;
		children[2].join();	
		sl /= 10;
		number *= 2;
		printf("Success rate is %f%%\n\n\n", 100* (1 - (1.0 * failure) / number));
	}

	return(0);
}

