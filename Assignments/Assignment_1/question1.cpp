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
char buf[MAXBUF];
int buf_len;
int failure = 0;
int sl;

void sender(int number, int capa)
{
	int n_sent;
	for (int i = 0; i < number; ++i)
	{
		long long sentTimeStamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);
		sprintf(buf, "%lld", sentTimeStamp);
		buf_len = strlen(buf);

		printf("Sending %d bytes to server - Sending...\n",buf_len);

		n_sent = sendto(client_socket,buf,buf_len,0,(struct sockaddr*) &server,sizeof(server));

		if(n_sent<0)
		{
			perror("Problem sending data");
			exit(1);
		}
		sleep(sl); 
	}
	pthread_exit(NULL);
}

void reciever(int number, int capa)
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
		        n_read = recvfrom(client_socket,buf,MAXBUF,0,NULL,NULL); // get your data
		        long long recTimeStamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);
		        if (n_read<0) 
		        {
				    perror("Problem in recvfrom");
				    exit(1);
				}
		        printf("Got back %d bytes\n",n_read);
				fflush(stdout);
				flag = false;
				std::stringstream temp;
				temp << buf;
				long long sentTimeStamp;
				temp >> sentTimeStamp;
				printf("RTT is : %lld microseconds\n", recTimeStamp - sentTimeStamp);
		        break;
		}

		if (flag)
			failure++;
	}
	pthread_exit(NULL);
}

int main( int argc, char **argv ) 
{
	if(argc!=6)
	{
		printf("Usage: %s <server name> <port number> <number of packets> <packet size> <sleep>\n",argv[0]);
		exit(0);
	}

	int capa = atoi(argv[4]);
	int tot = atoi(argv[3]);
	sl = atoi(argv[5]);
  	if (capa<16)
  	{
  		printf("The minimum message size is 16 bytes\n");
  		exit(1);
  	}

	client_socket = socket(PF_INET, SOCK_DGRAM, 0);

	if(client_socket == -1)
	{
		printf("Problem creating socket\n");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
  	memset(server.sin_zero, '\0', sizeof server.sin_zero);

  	thread *children = new thread[2];

  	children[0] = thread(sender, tot, capa);
  	children[1] = thread(reciever, tot, capa);

	for(int i=0; i < 2; i++)
		children[i].join();					// Joining the exited threads

    printf("Packet loss percentage is : %d%%\n", (failure*100/tot));
	return(0);
}