#include <stdio.h> /*printf, NULL*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "GenericDoubleLinkedList.h"
#include "ListItr.h"
#include "listFanctions.h"
#include "ListItr2.h"
#define SERVER_PORT 1025
#define SERVER_SUCCESS 1
#define SERVER_FAIL -1
#define BACK_LOG 1000 /*Q size*/
#define SERVER_NO_CLIENT -2
#define BUFFER_SIZE 1000
/*initilaze*/
int ServerInitialization(void);
/****/
static int ServerSocket(void);
static int ServerReusingTheSamePort(int _sock);
static int ServerBind(int _sock);
static int ServerListen(int _sock);
/****/
static int NoBlocking(int _sock);
/****/
static int ServerAccept(int _listeningSock);
/***/
static int ServerRecv(int _clientSock);
static int ServerSend(int _clientSock, char* _data, int _dataLen);
/********/
int CheckNewClient(List *_clientList, int sock);
int CheckCurrentClient(List* _clientList);
void CloseClients (void* _clientSock);
int ServerActionRunConnection(void* _clientSock, void* _context);
/********************************/
int main()
{
    int sock;
    List* clientList;
    if(SERVER_FAIL ==(sock = ServerInitialization()))
    {
        return SERVER_FAIL;
    }
    if (NULL == ( clientList = ListCreate()))
    {
        close (sock);
        return SERVER_FAIL;
    }
    while(1)
    {
        if(SERVER_FAIL == CheckNewClient(clientList, sock))
        {
            printf("out of server\n");
            break;
        }
        CheckCurrentClient(clientList);
    }
    ListDestroy(&clientList, CloseClients);
    close(sock);
    return 0;
}
/**********************************************************/
int CheckNewClient(List *_clientList, int _sock) /*malloc to new client, accept and push to list*/
{
    int *clientSock;
    if((clientSock =(int*)malloc(sizeof(int)))==NULL)
    {
        return SERVER_FAIL;
    }
    *clientSock = ServerAccept(_sock);
    if (*clientSock == SERVER_FAIL) /*error*/
    {
        free(clientSock);
        return SERVER_FAIL;
    }
    if (*clientSock == SERVER_NO_CLIENT ) /*no clients*/
    {
        free (clientSock);
        return SERVER_SUCCESS;
    }
    if(LIST_SUCCESS != (ListPushTail(_clientList, clientSock)))
    {
        close(*clientSock);
        free (clientSock);
        return SERVER_FAIL;
    }
    return SERVER_SUCCESS;
}
/************************/
int CheckCurrentClient(List* _clientList) /*send recv*/
{
    ListItr runner, temp, end;
    void* sock;
    runner = ListItrBegin(_clientList);
    end = ListItrEnd(_clientList);
    while(runner != end)
    {
        runner = ListItrForEach(runner, end, ServerActionRunConnection, NULL);
        if(runner != end)
        {
            temp = runner;
            runner = ListItrNext(runner); 
            sock = ListItrRemove(temp);
            close(*(int*)sock);
            free(sock);
        }
        
    }
    return SERVER_SUCCESS;
}
/***********************/
void CloseClients (void* _clientSock)
{
    close(*(int*)_clientSock);
    free(_clientSock);
}
/**********************************************************/
int ServerActionRunConnection(void* _clientSock, void* _context) /*action func for the foreach*/
{
    char data[7] ="server";
    int dataLen = sizeof(data);
    int result;
    if(SERVER_FAIL ==(result= ServerRecv(*(int*)_clientSock)))
    {
        return 0;        
    }
    if(SERVER_SUCCESS == result)
    {
        if(SERVER_FAIL == ServerSend(*(int*)_clientSock, data,dataLen))
        {
            return 0; 
        }   
    }
    return 1;
}
/*******************************************************/
int ServerInitialization(void)
{
    int sock;
    struct sockaddr_in sin;
    if (0 > (sock = ServerSocket()))
    {
      return SERVER_FAIL;  
    } 
    if(NoBlocking(sock) == SERVER_FAIL)
    {
        close (sock);
        return SERVER_FAIL;
    }
    if (SERVER_FAIL == ServerReusingTheSamePort(sock))
    {
        close(sock);
        return SERVER_FAIL;
    }
    if (SERVER_FAIL == ServerBind(sock))
    {
        close(sock);
        return SERVER_FAIL;
    }
    if (SERVER_FAIL == ServerListen(sock))
    {
        close(sock);
        return SERVER_FAIL;
    }
    return sock;
}
/*Initialize*/
static int ServerSocket(void)
{
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock < 0)
    {
	    perror("socket failed");
    }
    return sock;
} 

static int ServerReusingTheSamePort(int _sock) /*if the port is occupide, reales it for immidiat use*/
{
    int optval = 1;
    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
	    perror("reuse failed");
        return SERVER_FAIL;
    }
    return SERVER_SUCCESS;
}
 /*bind*/
static int ServerBind(int _sock)
{   
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);
    if(bind(_sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) 
    {
        perror("bind failed");
        return SERVER_FAIL;
    }
    return SERVER_SUCCESS;
}
 /*listen*/
static int ServerListen(int _sock)/*make the cock as a passive lisiner for clients*/
{
    if (listen(_sock, BACK_LOG) < 0) 
    {
        perror("listen failed");
        return SERVER_FAIL;
    }
    return SERVER_SUCCESS;
}
/*****************************************************/
/*accept*/
static int ServerAccept(int _listeningSock)
{
    struct sockaddr_in sinClient;
    socklen_t sinClientSize = sizeof(sinClient);
    int clientSock = accept(_listeningSock,(struct sockaddr*) &sinClient, &sinClientSize);
    if (clientSock < 0)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("accept failed\n");
            return SERVER_FAIL;
        }
        return SERVER_NO_CLIENT;
    }
    if( SERVER_SUCCESS != NoBlocking(clientSock))
    {
        printf("no blocking failed\n");
        close(clientSock);
        return SERVER_FAIL;
    }
    return clientSock;
}
/**receive**/

static int ServerRecv(int _clientSock)
{
    char buffer[BUFFER_SIZE];
    int expectedDataLen = sizeof(buffer);
    int read_bytes = recv(_clientSock, buffer, expectedDataLen, 0);
    if (read_bytes < 0)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("recv failed");
            return SERVER_FAIL;
        }
        if( SERVER_SUCCESS != NoBlocking(_clientSock))
        {
            return SERVER_FAIL;
        }
        return SERVER_NO_CLIENT;
    } 
    if (read_bytes == 0) /*connection is closed*/
    {
        return SERVER_FAIL;
    } 
    printf("%s\n", buffer);
    return SERVER_SUCCESS;
} 
/*send*/
static int ServerSend(int _clientSock, char* _data, int _dataLen)
{
    int sentBytes = send(_clientSock, _data, _dataLen, 0);
    if (sentBytes < 0)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("send failed");
            return SERVER_FAIL;
        }
        if( SERVER_SUCCESS != NoBlocking(_clientSock))
        {
            return SERVER_FAIL;
        }
    }
    return SERVER_SUCCESS;
}
/****************************************************/
static int NoBlocking(int _sock)
{
    int flags;
    if (-1 == (flags = fcntl(_sock, F_GETFL)))
    {
        perror("error at fcntl. F_GETFL.");
        return SERVER_FAIL;
    }
    if(-1  == fcntl(_sock, F_SETFL, flags | O_NONBLOCK))
    {
        perror("error at fcntl. F_SETFL.");
        return SERVER_FAIL;
    }  
    return SERVER_SUCCESS;
}
