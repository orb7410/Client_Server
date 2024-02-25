#include <stdio.h> /*printf, NULL*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#define SERVER_PORT 1025



int main(void)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int i;
    struct sockaddr_in sin;
    int readBytes, sentBytes, dataSize, massagesSize;
    socklen_t sinSize;
    char buffer[24];
    char data[24] = "str to send from client";
    if (sock < 0)
    {
	    perror("socket failed");
    }
    memset(&sin, 0, sizeof(sin));

    for (i = 0; i < 5; i++)
    {
            /*sendto*/
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr("10.0.0.17");
        sin.sin_port = htons(SERVER_PORT);
        dataSize = sizeof(data);
        sentBytes = sendto(sock, data, dataSize, 0, (struct sockaddr *) &sin, sizeof(sin));
        if (sentBytes < 0)
        {
            perror("sendto failed");
        }

        /*resevfrom*/
        sinSize = sizeof(sin);
        readBytes = recvfrom(sock, buffer, sizeof(buffer),0 ,(struct sockaddr *) &sin, &sinSize);
        if (readBytes < 0) 
        {
            perror("recvfrom failed");
        }
        printf("%s\n",buffer);
    }
    
	
    close();
    return 0;
}
