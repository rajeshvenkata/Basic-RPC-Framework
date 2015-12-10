//Server side RPC

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#define RECIPIENT_PORT 1234
 #define SIZE 1000
#define TRUE 1
#define FALSE 0
typedef struct {
 unsigned int length;
 unsigned char data[SIZE];
 } Message;

typedef enum {
 OK,  /* operation successful */ 
 BAD,  /* unrecoverable error */ 
 WRONGLENGTH,/* bad message length supplied */
 DIVZERO,
 TIMEOUT

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
//void printSA(SocketAddress sa) ;
void makeDestSA(SocketAddress * sa, char *hostname, int port) ;
//void makeLocalSA(SocketAddress *sa) ;
void makeReceiverSA(SocketAddress *sa, int port);
void marshal(RPCMessage *rm1,RPCMessage *rm2);
void unMarshal(RPCMessage *rm1, Message *rm2);

Status validateMessage(RPCMessage *r,char *exp);
void configureRPC(RPCMessage *r,char *str,char* op);

 Status DoOperation (Message *message, Message *reply, int s, SocketAddress serverSA);
 //Status GetRequest (Message *callMessage, int s, SocketAddress *clientSA);
// Status SendReply (Message *replyMessage, int s, SocketAddress clientSA);
 Status  UDPsend(int s, Message *m, SocketAddress destination);
 Status  UDPreceive(int s, Message *m, SocketAddress *origin);
int anyThingThere(int s);
/* Function to read the string entered by the user -
*/

void read_line(char Str[]) {
    int i = 0;   char next;
    while ((next=getchar())!='\n') {
     Str[i] = next;
     i++;
    }
    Str[i] = 0;    /* Set the null char at the end */
}


/* main for the client - 
*/
void main(int argc,char **argv)
{
    int s,port = RECIPIENT_PORT+6218;
    Message m,m_reply;
    Status status,status_i;
    SocketAddress serverSA,clientSA,aclientSA;
    
            if(argc <= 1)
            {
        printf("Usage: Provide the server name ??\n");
        exit(1);
            }
    if(( s = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("socket failed");
        return;
    }
    
    char *machine=argv[1];/*fetching the host name  */
    /*setting the receiver internet domain socket address*/
   // makeReceiverSA(&clientSA, port);
    //printSA(clientSA);
    makeDestSA(&serverSA,machine,port);/*setting the destination internet domain socket address*/
            while(1) 
          {
        printf("\n Enter the message:");
              read_line(m.data);/*taking input from the user*/
                m.length=strlen(m.data);
                
        status=DoOperation(&m,&m_reply,s,serverSA);/*starting the operation*/
        if(status==TIMEOUT)
        {
            printf("\n Timeout occured");
        }
              }
}

/*validating the message for the desired format*/
Status validateMessage(RPCMessage *r,char *exp)
{
    int first_digit=FALSE;
    int second_digit=FALSE;
    
    char *p=exp;
    char op[2];
    if(p==NULL)
    {
        return BAD;
    }
    else {
      while(p!=NULL && isdigit(*p))
        {
           // printf("inside while");
        first_digit=TRUE;
        p++;
        }
    
        if(!(*p=='+' || *p=='-'|| *p=='*' || *p=='/'))
        {
           // printf("inside if");
            return BAD;
        }
        else {
            strncpy(op,p,1);
            p++;
            while(p!=NULL && isdigit(*p))
            {
               // printf("inside while2");
                second_digit=TRUE;
                p++;
            }
        }
        if((first_digit==FALSE) || (second_digit==FALSE))
        {
          //  printf("inside if 2");
            return BAD;
        }
        else
        {
            /*configuring the essage into rpc message structure*/
            configureRPC(r,exp,op);
           // printf("after config");
            return OK;
        }
    }
    
    
  
}
/*configuring the essage into rpc message structure*/
void configureRPC(RPCMessage *r,char *str,char* op)
{
    static int RPC_id=1;
    if(*op == '+')
    {
        r->procedureId = 1;
    }
    else if(*op == '-')
    {
        r->procedureId = 2;
        
    }
    else if(*op == '*')
        
    {
        r->procedureId = 3;
        
    }
    else if(*op == '/')
    {
        r->procedureId = 4;
        
    }
     else
         return;
    
    char buffer [256];
    strcpy(buffer,str);
    r->arg1=atoi(strtok(buffer,op));
    r->arg2=atoi(strtok(NULL,op));
    r->RPCId=RPC_id;
    RPC_id++;
    r->messageType=Request;
   // printf("\n at the end of config");
   // printf("\n Rpc structure %d %d %d",r->procedureId,r->arg1,r->arg2);
    
    
}

/*marshalling the data*/
void marshal(RPCMessage *rm1,RPCMessage *rm2)
{
    rm2->RPCId=htonl(rm1->RPCId);
    rm2->procedureId=htonl(rm1->procedureId);
    rm2->arg1=htonl(rm1->arg1);
    rm2->arg2=htonl(rm1->arg2);
    //rm2->messageType=htonl(rm1->messageType);
}

/*unmarshalling the data*/
void unMarshal(RPCMessage *rm1,Message *m)
{
    RPCMessage *rm2=(RPCMessage *)m->data;
    printf("\n before unmarshalling%d",rm2->arg1);
    rm1->RPCId=ntohl(rm2->RPCId);
    rm1->procedureId=ntohl(rm2->procedureId);
    rm1->arg1=ntohl(rm2->arg1);
    rm1->arg2=ntohl(rm2->arg2);
    printf("\n after unmarshalling %d",rm1->arg1);
    printf("\n unmarshall \n ");
    //rm1->messageType=ntohl(rm2->messageType);
}

/*sends a given request message to a given socket address and blocks until it returns with a reply message
*/
Status DoOperation (Message *message, Message *reply, int s, SocketAddress serverSA)
{
Status status=OK;
int n=0;
/*creating the socket*/
    if(( s = socket(AF_INET, SOCK_DGRAM, 0))<0) 
        {
        perror("socket failed");
        printf("errno = %d.\n", errno);
        return BAD;
        }
  /*checking for time out*/


status=UDPsend(s,message,serverSA);/*sending the message through UDPsend*/
int j=anyThingThere(s);
    while(j==0)
    {
   
    
        status=UDPsend(s,message,serverSA);
    printf("\n Sending failed:");
        printf("\n trying to send again");
        
        if(n>2)
        {
            printf("\n Exit-ing");
            exit(1);
        }
        n++;
         j=anyThingThere(s);
    
    }
   
status=UDPreceive(s,reply,&serverSA);/*receiving the reply through UDPreceive*/
     if(status==BAD)
    {
    printf("\n Sending failed:");
    exit(1);
    }
return status;
}

/*sends a given message through a socket to a given socket address.
*/
Status  UDPsend(int s, Message *m, SocketAddress destination)
{
int n;
RPCMessage r1,r2;
char *p;
char op[2];
Status status=OK;
Status status_i=OK;
    r1.RPCId=0;
    if(strstr(m->data,"Stop")!=NULL)
    {
       // printf("\n inside Stop");
        if( (n = sendto(s, m->data,SIZE, 0, (struct sockaddr *)&destination,
                        sizeof(SocketAddress))) < 0)
        {
            status=BAD;
            perror("Send failed\n");
        }
    }
    else if(strstr(m->data,"Ping")!=NULL)
    {
      //  printf("\n inside Ping");
        if( (n = sendto(s, m->data,SIZE, 0, (struct sockaddr *)&destination,
                        sizeof(SocketAddress))) < 0)
        {
            status=BAD;
            perror("Send failed\n");
        }
    }
    else
    {
      //  printf("\n inside validate");
        /*validating the message to be in the desired format*/
    status_i=validateMessage(&r1,m->data);
    if(status_i==BAD)
    {
        printf("\n Enter the message in the correct format(eg:3+4):");
       // continue;
        status=BAD;
    }
   /*marshalling the Rpc message structure before sending to the server */
   marshal(&r1,&r2);

if( (n = sendto(s, &r2,sizeof(RPCMessage), 0, (struct sockaddr *)&destination,
        sizeof(SocketAddress))) < 0)
    {
        status=BAD;
        perror("Send failed\n");
    }
    }
return status;
}

/*receives a message and the socket address of the sender into two arguments
*/
Status  UDPreceive(int s, Message *m, SocketAddress *origin)
{
int n,length=sizeof(origin);
RPCMessage m_reply;
Status status=OK;
if((n = recvfrom(s, m->data, 1000, 0, (struct sockaddr *)origin, &length))<0)
    {
            perror("Receive failed");
            status=BAD;
        return BAD;
    }
    
    printf("\n after receive from");
    
    /*un marshalling the reply message*/
    unMarshal(&m_reply,m);
    printf("\n after unmarshall\n ");
    if((m_reply.arg2==2)|| (m_reply.arg2==3))/*If the sent message is Ping or Stop*/
    {
        printf("\n server replied: OK");
    }
    if(m_reply.arg2==-1)/*when trying to divide by zero*/
    {
        printf("\n Cannot divide 0:DivideZero error");
    }
    else/*printing the calculated value with the rpc id*/
    {
        printf("\n The calculated value: %d%d",m_reply.arg1,m_reply.arg2);
        printf("\n and the process Id for the request is:%u",m_reply.RPCId);
    }
        
        //printf("\n server replied: %s\n",m->data);
return status;
}


/* make a socket address for a destination whose machine and port
    are given as arguments */
void makeDestSA(SocketAddress * sa, char *hostname, int port)
{
    struct hostent *host;

    sa->sin_family = AF_INET;
/*resolve host by name*/
    if((host = gethostbyname(hostname))== NULL){
        printf("Unknown host name\n");
        exit(-1);
    }
    sa-> sin_addr = *(struct in_addr *) (host->h_addr);
    sa->sin_port = htons(port);
}

/* use select to test whether there is any input on descriptor s*/
int anyThingThere(int s)
{
    unsigned long read_mask;
    struct timeval timeout;
    int n;
    
    timeout.tv_sec = 10; /*seconds wait*/
    timeout.tv_usec = 0;        /* micro seconds*/
    read_mask = (1<<s);
    if((n = select(32, (fd_set *)&read_mask, 0, 0, &timeout))<0)
        perror("Select fail:\n");
    else printf("n = %d\n");
    return n;
}

/* make a socket address using any of the addressses of this computer
 for a local socket on given port */
void makeReceiverSA(SocketAddress *sa, int port)
{
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    sa-> sin_addr.s_addr = htonl(INADDR_ANY);
}
