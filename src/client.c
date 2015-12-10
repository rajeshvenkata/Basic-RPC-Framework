

//Client side RPC


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#define RECIPIENT_PORT 1234
 #define SIZE 1000
#define TRUE 1
#define FALSE 0
//#define Request 0
//#define Reply 1
 typedef struct {
 unsigned int length;
 unsigned char data[SIZE];
 } Message;

 typedef enum {
 OK,  /* operation successful */ 
 BAD,  /* unrecoverable error */ 
 WRONGLENGTH,/* bad message length supplied */
 DIVZERO,
 STOP,
 PING,
 MARSHAL
 } Status;
 typedef struct sockaddr_in SocketAddress ;


typedef struct {
    enum {Request, Reply} messageType;  /* same size as an unsigned int */
    unsigned int RPCId; /* unique identifier */
    unsigned int procedureId; /* e.g.(1,2,3,4) for (+, -, *, /) */
    int arg1; /* argument/ return parameter */
    int arg2; /* argument/ return parameter */
} RPCMessage; /* each int (and unsigned int) is 32 bits = 4 bytes */


struct hostent *gethostbyname() ;
void printSA(SocketAddress sa) ;
//void makeDestSA(SocketAddress * sa, char *hostname, int port) ;
//void makeLocalSA(SocketAddress *sa) ;
void makeReceiverSA(SocketAddress *sa, int port);
// Status DoOperation (Message *message, Message *reply, int s, SocketAddress serverSA);
void marshal(RPCMessage *rm1,RPCMessage *rm2);
void unMarshal(RPCMessage *rm1, Message *m);
 Status GetRequest (Message *callMessage, int s, SocketAddress *clientSA);
 Status SendReply (RPCMessage *replyMessage, int s, SocketAddress clientSA);
 Status  UDPsend(int s, RPCMessage *m, SocketAddress destination);
 Status  UDPreceive(int s, Message *m, SocketAddress *origin);

Status add( int arg1, int arg2, int *result);
Status subtract( int arg1, int arg2, int *result);
Status multiply( int arg1, int arg2, int *result);
Status divide( int arg1, int arg2, int *result);
int Calculate(RPCMessage *m);
/*
main for server
*/
void main(int argc,char **argv)
{
int s,port = RECIPIENT_PORT+6218;
    Message m_input,m_reply,m;
    Status status;
    SocketAddress serverSA,clientSA;
    RPCMessage rm1,rm_reply;
/*creating socket*/
    if(( s = socket(AF_INET, SOCK_DGRAM, 0))<0) 
    {
        perror("socket failed");
        printf("errno = %d.\n", errno);
        return;
    }
/*setting the receiver internet domain socket address*/
makeReceiverSA(&serverSA, port);
/*binding the socket to the server socket address*/
    if( bind(s, (struct sockaddr *)&serverSA, sizeof(SocketAddress))!= 0){
        perror("Bind failed\n");
        close(s);
        return;
    }
printSA(serverSA);
while(1)
    {
status=GetRequest(&m_input,s,&clientSA);
if(status==BAD)
    {
printf("Receive request failed");
    }
if(status==PING)/*if client enters Ping*/
{
    rm1.arg2=2;
   // rm1.messageType=Reply;
    rm1.arg1=0;
    rm1.procedureId=0;
    rm1.RPCId=0;
    marshal(&rm_reply,&rm1);
    status=SendReply(&rm_reply,s,clientSA);

}
if(status==STOP)/*if client enters stop*/
{
    rm1.arg2=3;
  //  rm1.messageType=Reply;
    rm1.arg1=0;
    rm1.procedureId=0;
    rm1.RPCId=0;
    marshal(&rm_reply,&rm1);
    status=SendReply(&rm_reply,s,clientSA);
    close(s);
    exit(1);
}
if(status==MARSHAL)/*if client sent an expression to evaluate*/
{
    unMarshal(&rm1,&m_input);
    rm1.arg2=Calculate(&rm1);
    printf("\n calculatiom %d",rm1.arg1);
 //   rm1.messageType=Reply;
    marshal(&rm1,&rm_reply);
    status=SendReply(&rm_reply,s,clientSA);
}
//status=SendReply(&rm_reply,s,clientSA);
if(status==BAD)
    {
printf("Reply failed");
    }
     }

}

/*receives a request message and the client's socket address.
*/
 Status GetRequest (Message *callMessage, int s, SocketAddress *clientSA)
{
printf("\n get request checking\n");
Status status=OK;
status=UDPreceive(s,callMessage,clientSA);
if(status==DIVZERO)
{
    status=DIVZERO;
}

return status;
}
/*sends a reply message to the given client's socket address
*/
Status SendReply (RPCMessage *replyMessage, int s, SocketAddress clientSA)
{
Status status=OK;
status=UDPsend(s,replyMessage,clientSA);
return status;
}

/*receives a message and the socket address of the sender into two arguments.
*/
Status  UDPreceive(int s, Message *m, SocketAddress *origin)
{
    RPCMessage rm1,rm_reply;
int n,length=sizeof(origin);
Status status=OK;

if((n = recvfrom(s, m->data, sizeof(RPCMessage), 0, (struct sockaddr *)origin, &length))<0)
    {
            status=BAD;
            perror("Receive failed.");
                     
    }


if(strstr(m->data,"Stop")!=NULL)
{
printf("Quit connection");
status=STOP;
//close(s);
//exit(1);
}
if(strstr(m->data,"Ping")!=NULL)
{
status=PING;
printf("\n Ok");
}
else
{
status=MARSHAL;
//unMarshal(&rm1,m);
//rm1.arg2=Calculate(&rm1);
//printf("evaluated expression: %d",rm1.arg1);
//if(rm1.arg2==-1)
//{
 //   status=DIVZERO;
//}
//    marshal(&rm1,&rm_reply);
    
}
return status;
}

/*sends a given message through a socket to a given socket address.
*/
Status  UDPsend(int s, RPCMessage *m, SocketAddress destination)
{
int n;
Status status=OK;
Message m1;
    int n2,n3;
RPCMessage m_reply;
    m_reply.RPCId=m->RPCId;
    m_reply.procedureId=m->procedureId;
    m_reply.arg1=m->arg1;
    m_reply.arg2=m->arg2;
    printf("\n arg1:%d",m_reply.arg1);
    n2=ntohs(m_reply.arg1);
    n3=ntohs(m_reply.arg2);
    printf("\n arg2:%d",n3);
//strcpy(m1.data,"     message received at server");
if( (n = sendto(s, &m_reply,sizeof(m_reply), 0, (struct sockaddr *)&destination,
        sizeof(SocketAddress))) < 0)
{
                        status=BAD;
                        perror("Receive failed.");
}

return status;
}

/*print a socket address */
void printSA(SocketAddress sa)
{
    char mybuf[80];
    char *ptr=inet_ntop(AF_INET, &sa.sin_addr, mybuf, 80);
    printf("sa = %d, %s, %d\n", sa.sin_family, mybuf, ntohs(sa.sin_port));
}

/* make a socket address using any of the addressses of this computer
for a local socket on given port */
void makeReceiverSA(SocketAddress *sa, int port)
{
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    sa-> sin_addr.s_addr = htonl(INADDR_ANY);
}
/*function to evaluate addition*/
Status add( int arg1, int arg2, int *result)
{
    *result=arg1+arg2;
    return OK;
}
/*function to evaluate subtraction*/
Status subtract( int arg1, int arg2, int *result)
{
    *result=arg1-arg2;
    return OK;
}
/*function to evaluate multiplication*/
Status multiply( int arg1, int arg2, int *result)
{
    *result=arg1*arg2;
    return OK;
}
/*function to evaluate division*/
Status divide( int arg1, int arg2, int *result)
{
    if(arg2 == 0)
    {
        return DIVZERO;
    }
    
    *result=arg1/arg2;
    return OK;
}

/*marshalling the rpc message structure to send the reply*/
void marshal(RPCMessage *rm1,RPCMessage *rm2)
{
    rm2->RPCId=htonl(rm1->RPCId);
    rm2->procedureId=htonl(rm1->procedureId);
    rm2->arg1=htonl(rm1->arg1);
    rm2->arg2=htonl(rm1->arg2);
   // rm2->messageType=htonl(rm1->messageType);
}

/*unmarshalling the message sent by the client*/
void unMarshal(RPCMessage *rm1,Message *m)
{
    RPCMessage *rm2=(RPCMessage *)m->data;
    rm1->RPCId=ntohl(rm2->RPCId);
    rm1->procedureId=ntohl(rm2->procedureId);
    rm1->arg1=ntohl(rm2->arg1);
    rm1->arg2=ntohl(rm2->arg2);
  //  rm1->messageType=ntohl(rm2->messageType);
}

/*calculating the value of the expression*/
int Calculate(RPCMessage *m)
{
    int result;
    m->messageType=Reply;
    if(m->procedureId == 1)
    {
        if(add(m->arg1,m->arg2,&result) == OK)
        {
            m->arg1=result;
            return 0;
        }
    }
    else if(m->procedureId == 2)
    {
        if(subtract(m->arg1,m->arg2,&result) == OK)
        {
            m->arg1=result;
            return 0;
        }
    }
    else if(m->procedureId == 3)
    {
        if(multiply(m->arg1,m->arg2,&result) == OK)
        {
            m->arg1=result;
            return 0;
        }
    }
    else if(m->procedureId == 4)
    {
        Status status=OK;
        status=divide(m->arg1,m->arg2,&result);
        if(status == OK)
        {
            m->arg1=result;
            return 0;
        }
        else if(status == DIVZERO)
        {
            m->arg1=0;
            return -1;
        }
    }
    
    
    return -2;
}
