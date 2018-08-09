#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#define MAXBUF 1024

int main() 
{
    int server_socket;
    struct sockaddr_in skaddr;
    unsigned int length;

    server_socket = socket(PF_INET, SOCK_DGRAM,0);

    skaddr.sin_family = AF_INET;
    skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    skaddr.sin_port = htons(0);

    if (bind(server_socket, (struct sockaddr *) &skaddr, sizeof(skaddr))<0) 
    {
        printf("Problem binding\n");
        exit(0);
    }

    length = sizeof( skaddr );
    if (getsockname(server_socket, (struct sockaddr *) &skaddr, &length)<0) 
    {
        printf("Error getsockname\n");
        exit(1);
    }

    printf("The server UDP port number is %d\n",ntohs(skaddr.sin_port));

    unsigned int len;
    int n;
    char bufin[MAXBUF];
    struct sockaddr_in remote;

    len = sizeof(remote);

    while (1) 
    {
        n=recvfrom(server_socket,bufin,MAXBUF,0,(struct sockaddr *)&remote,&len);

        printf("Got a datagram from %s port %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));

        if (n<0) 
        {
            perror("Error receiving data");
        } 
        else 
        {
            printf("GOT %d BYTES\n",n);
            sendto(server_socket,bufin,n,0,(struct sockaddr *)&remote,len);
        }
    }
    return(0);
}
