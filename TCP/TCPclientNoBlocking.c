#include <stdio.h> /*printf, NULL*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#define CLIENT_PORT 1025
#define CLIENT_SUCCESS 1
#define CLIENT_FAIL -1
#define BUFFER_SIZE 1000
#define NUM_OF_CLIENT 1000
#define ARRAY_SIZE 1000
static int ClientSocket(void);
static int Clientconnect(int _sock);
static int ClientSend(int _sock, char* _data, int _dataLen);
static int ClientRecv(int _sock);
int main()
{
    char data[12] = "I am client";
    int dataSize = sizeof(data);
    int sockArray[ARRAY_SIZE]= {0};
    size_t i = 0;
    while(1)
    {
        if(sockArray[i] == 0)
        {
            if((rand() % 100) <= 30)
            {
                sockArray[i] = ClientSocket();
                if(sockArray[i] < 0)
                {
                    close(sockArray[i]);
                    sockArray[i] = 0;
                }
                if(ClientConnect(sockArray[i]) != CLIENT_SUCCESS)
                {
                    close(sockArray[i]);
                    sockArray[i] = 0;
                }
            }
            i = (i + 1) % ARRAY_SIZE;
            continue;
        }
        if((rand() % 100) <= 5)
        {
            close(sockArray[i]);
            sockArray[i] = 0;
            i = (i + 1) % ARRAY_SIZE;
            continue;
        }
        if((rand() % 100) <= 30)
        {
           if(ClientSend(sockArray[i], data, dataSize) != CLIENT_SUCCESS)
           {
                close(sockArray[i]);
                sockArray[i] = 0;
                i = (i + 1) % ARRAY_SIZE;
                continue;
           }
           if(ClientRecv(sockArray[i]) != CLIENT_SUCCESS)
           {
                close(sockArray[i]);
                sockArray[i] = 0;
                i = (i + 1) % ARRAY_SIZE;
           }
           
        }
    }

    return 0;
}  
/*int ClientRun(char* _data, int _dataSize)
{
    int sock = ClientSocket();
    if(sock < 0)
    {
        return CLIENT_FAIL;
    }
    if(ClientConnect(sock) != CLIENT_SUCCESS)
    {
        return CLIENT_FAIL;
    }
    if(ClientSend(sock, _data, _dataSize) != CLIENT_SUCCESS)
    {
        return CLIENT_FAIL;
    }
    if(ClientRecv(sock) != CLIENT_SUCCESS)
    {
        return CLIENT_FAIL;
    }
    return CLIENT_SUCCESS;
}
*/

/******************************************help Run function**************************************/
int ClientSocket()
{
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket failed");
    }
    return sock;
}

int ClientConnect(int _sock)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr("192.168.1.107");
    sin.sin_port = htons(CLIENT_PORT);
    if(connect(_sock, (struct sockaddr*) &sin, sizeof(sin)) < 0)
    {
        perror("connection failed");
        return CLIENT_FAIL;
    }
    return CLIENT_SUCCESS;
}

int ClientSend(int _sock, char* _data, int _dataSize)
{
    int sendBytes;
    sendBytes = send(_sock, _data, _dataSize, 0);
    if(sendBytes < 0)
    {
        perror("send failed");
        return CLIENT_FAIL;
    }
    return CLIENT_SUCCESS;
}

int ClientRecv(int _sock)
{
    char buffer[BUFFER_SIZE];
    int bufferSize = sizeof(buffer);
    int readBytes = recv(_sock, buffer, bufferSize, 0);
    if(readBytes == 0)
    {
        close(_sock);
    }
    else if(readBytes < 0)
    {
        perror("recv failed");
        return CLIENT_FAIL;
    }
    printf("%s \n", buffer);
    return CLIENT_SUCCESS;
}
