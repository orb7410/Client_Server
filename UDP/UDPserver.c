#include <stdio.h> /*printf*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#define SERVER_PORT 1025



int main(void)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in sin;
    char buffer[24];
    int readBytes, sentBytes, dataSize, i, massagesSize;
    socklen_t sinSize;
    char data[24] = "str to send from server";
    if (sock < 0)
    {
	    perror("socket failed");
    }

    /*bind*/
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);
    if(bind(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) 
    {
        perror("bind failed");
    }


    for (i = 0; i < 5; i++)
    {
        /*resevfrom*/
        sinSize = sizeof(sin);
        readBytes = recvfrom(sock, buffer, sizeof(buffer),0,(struct sockaddr *) &sin, &sinSize);
        if (readBytes < 0) 
        {
            perror("recvfrom failed");
        }

        /*sendto*/
        sin.sin_addr.s_addr = inet_addr("10.0.0.17");
        dataSize = sizeof(data);
        sentBytes = sendto(sock, data, dataSize, 0, (struct sockaddr *) &sin, sizeof(sin));
        if (sentBytes < 0)
        {
            perror("sendto failed");
        } 
        printf("%s\n",buffer);
    }
    
    close();	
    return 0;
}
