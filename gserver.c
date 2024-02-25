#include <stdio.h> /*printf, NULL*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include "gserver.h"
#include "GenericDoubleLinkedList.h"
#include "ListItr.h"
#include "listFanctions.h"
#include "ListItr2.h"
#define BUFFER_SIZE 1000
#define SERVER_SUCCESS 1
#define SERVER_FAIL -1
#define SERVER_PORT 1025
#define BACK_LOG 10 /*Queue size*/
#define MAX_FD 1024
static int ServerInitialization(void);
static int NoBlocking(int _sock);
static int ServerListen(int _sock);
static int ServerBind(int _sock);
static int ServerReusingTheSamePort(int _sock);
static int ServerSocket(void);
void CloseClient(void* _client);
int StopRunClient(void* _element);
int CheakNewClient(List* _list, int _listenSock);
int CheakCurrentClient(Server* _server,  fd_set* _setMaster, fd_set* _setTemp);
static int ServerSend(int _clientSock, char* _data, int _dataSize);
static int ServerRecv(int _clientSock);
void ServerDestroy (Server** _server);
int ServerAction(void* _clientSock, void* _context);

typedef struct FdSet
{
    fd_set*  m_setMaster;
    fd_set*  m_setTemp;
    int      m_activity; 
}FdSet;

struct Server
{
    NewClient         m_newClientFunc;
    GotMessage        m_gotMessageFunc;
    CloseClientFunc   m_closeClientFunc;
    Failed            m_failFunc;
    StopRun           m_stopRunFunc;
    FdSet*             m_fdSruct;
    List*             m_clients;
    int               m_flag;
    int               m_sock;
};
/******/
Server* ServerCreate(NewClient _newClientFunc, GotMessage _gotMessageFunc, CloseClientFunc _closeClientFunc, Failed _failFunc, StopRun _stopRunFunc)
{
    List* clientList;
    Server* newServer; 
    int serverSocket;
    FdSet* newFdStruct;
    if (NULL == _gotMessageFunc)
    {
        return NULL;
    }
    if (NULL == (newServer =(Server*)malloc(sizeof(Server))));
    {
        return NULL;
    }
    if (NULL == (clientList = ListCreate()))
    {
        return NULL;
    }
    if (SERVER_FAIL == (serverSocket = ServerInitialization()))
    {
        ListDestroy(&clientList, CloseClient);
        free(newServer);
        return NULL;
    }
    if (NULL == (newFdStruct =(FdSet*)malloc(sizeof(FdSet))))
    {
        ListDestroy(&clientList, CloseClient);
        close (serverSocket);
        free(newServer);
        return NULL;
    }
    newServer -> m_newClientFunc = _newClientFunc;
    newServer ->  m_gotMessageFunc = _gotMessageFunc;
    newServer -> m_closeClientFunc = _closeClientFunc;
    newServer -> m_failFunc = _failFunc;
    newServer -> m_stopRunFunc = _stopRunFunc;
    newServer -> m_fdSruct = newFdStruct;
    newServer -> m_clients = clientList;
    newServer -> m_flag = 1;
    newServer -> m_sock = serverSocket;

    return newServer;
}
/******/
int ServerRun(Server* _server)
{
    if (NULL == Server)
    {
        return SERVER_FAIL;
    }
    fd_set setMaster, setTemp;
    int activity;
    int newSock;
    int counter;
    setMaster = _server-> m_fdSruct. m_setMaster;
    setTemp = _server-> m_fdSruct. m_setTemp;
    activity = _server-> m_fdSruct.m_activity;

    FD_ZERO(&setMaster);
    FD_SET(_server->m_sock ,&setMaster);

    while (_server -> flage)  /*do i need the counter? do i need return fail instead of StopRunClient();*/
    {
        setTemp = setMaster;
        activity = select(MAX_FD, &setTemp, NULL, NULL, NULL);
        if(activity < 0 && errno != EINTR)
        {
            if(_server->m_failFunc != NULL)
            {
                _server->m_failFunc;/*restart or somthing*/
            } 
            printf("select error");
            return SERVER_FAIL;
        }
        else if(activity == 0)
        {
            continue;
        }
        else
        {
            counter = 0;
            if(FD_ISSET(_server->m_sock, &setTemp))
            {
                if((newSock = CheakNewClient(_server->m_clients, _server->m_sock)) == SERVER_FAIL)
                {
                    if(_server->m_failFunc != NULL)
                    {
                        _server->m_failFunc;/*restart or somthing*/
                    }
                    printf("accept error"); 
                    StopRunClient(_server);
                }
                FD_SET(newSock, &setMaster);
                 ++counter;
            }
            if(counter < activity)
            {
                if(SERVER_SUCCESS != CheakCurrentClient(_server, &setMaster, &setTemp))
                {
                    if(_server->m_failFunc != NULL)
                    {
                        _server->m_failFunc;/*restart or somthing*/
                    } 
                    printf("send error");
                    StopRunClient(_server);
                } 
            }  
        }        
    }
    return SERVER_SUCCESS;
}
/*****/
void ServerDestroy (Server** _server)
{
    if (NULL == _server || NULL == *_server)
    {
        return; 
    }
    ListDestroy(&(*_server)->m_clients, CloseClient);
    close((*_server) -> m_sock);
    free((*_server)-> m_fdSruct);
    free((*_server));
    *_server = NULL;
}
/*************************************************************/
/*****************run help functions*************************/
int CheakNewClient(List* _list, int _listenSock)/*malloc to new client, accept and push to list*/
{
    int* clientSock;
    if(NULL == (clientSock = (int*)malloc(sizeof(int))))
    {
        return SERVER_FAIL;
    }
    *clientSock = ServerAccept(_listenSock);
    if(*clientSock == SERVER_FAIL)
    {
        free(clientSock);
        return SERVER_FAIL;
    }
    if(ListPushTail(_list, clientSock) != LIST_SUCCESS)
    {
        close(*clientSock);
        free(clientSock);
        return SERVER_FAIL;
    }
    return *clientSock;
}
/*************/
int CheakCurrentClient(Server* _server,  fd_set* _setMaster, fd_set* _setTemp)
{
    void* clientToRemove;
    ListItr runner, temp, end;
    runner = ListItrBegin(_server ->m_clients);
    end = ListItrEnd(_server ->m_clients);
    while (runner != end)
    {
        runner = ListItrForEach(runner, end, ServerAction, &_server->m_fdSruct);
        if(runner != end)
        {
            temp = runner;
            runner = ListItrNext(runner);
            clientToRemove = ListItrRemove(temp);
            FD_CLR(*(int*)clientToRemove, _setMaster);
            close((*(int*)clientToRemove));
            free(clientToRemove);
        }
    }
    return SERVER_SUCCESS;
}
/*************/
int ServerAction(void* _clientSock, void* _context)
{
    char data[20] = "message from server";
    int dataSize = sizeof(data);
    int result;
    if(FD_ISSET(*(int*) _clientSock, ((FdSet*)_context)->m_setTemp))
    {
        result = ServerRecv(*(int*)_clientSock);
        if(result == SERVER_FAIL)
        {
            return 0;
        }
        if(result == SERVER_SUCCESS)
        {
            if(ServerSend((*(int*)_clientSock), data, dataSize) != SERVER_SUCCESS)
            {
                return 0;
            }
        }
    }
    return 1;
}
/******************/
static int ServerRecv(int _clientSock)
{
    char buffer[BUFFER_SIZE];
    int bufferSize = sizeof(buffer);
    int readBytes = recv(_clientSock, buffer, bufferSize, 0);
    if(readBytes < 0)
    {
        return SERVER_FAIL;
    }
    else if(readBytes == 0)
    {
        return SERVER_FAIL;
    }
    printf("%s \n", buffer);
    return SERVER_SUCCESS;
}
/******************/
static int ServerSend(int _clientSock, char* _data, int _dataSize)
{
    int sendBytes;
    sendBytes = send(_clientSock, _data, _dataSize, 0);
    if(sendBytes < 0)
    {
        return SERVER_FAIL;
    }
    return SERVER_SUCCESS;
}
/*****************************************************/
static int ServerInitialization(void)
{
    int sock;
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
/********************************************************/
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
/**************************************************/
void CloseClient(void* _client)
{
    close(*(int*)_client);
    free(_client);
}
/************/
int StopRunClient(void* _element)
{
    ((Server*)_element) -> m_flag = 0;
    return 1;
}
/************/
int ServerFailed(void* _element, void* _context)
{
    /* _element is a pointer to the failed element-client socket */
    int failedSock = *(int*)_element;
    char* failedContext = (char*)_context;
    printf("Server failed on socket %d with context: %s\n", failedSock, failedContext);
    close(failedSock);
   /* ServerDestroy???*/
    return 0;
}