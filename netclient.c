/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


void reverseValue(const long long int size, void* value){
    int i;
    char result[32]; // never seen built-in types >8 bytes (have room for 32)
    for( i=0; i<size; i+=1 ){
        result[i] = ((char*)value)[size-i-1];
    }
    for( i=0; i<size; i+=1 ){
        ((char*)value)[i] = result[i];
    }
}



double ntohd(double src)
{
#   if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
        reverseValue(sizeof(double), &src);
#   endif
        return src;

}

typedef struct{
	int size_of_time;
	char time_str[20];
}time_data;

typedef union{
	double n;
	time_data td;
}data;


typedef struct{
	int opcode0 :3;
	int opcode1 :2;
	int RQID :3;
	data d;
}request;


void init_request_root(request* rq, int rqid, double d){
	rq->opcode0 = 0;
	rq->opcode1 = 1;
	rq->RQID = rqid;
	rq->d.n = ntohd(d);
}

void init_request_time(request* rq, int rqid){
	rq->opcode0 = 0;
	rq->opcode1 = 2;
	rq->RQID = rqid;
}

void read_request(request rq){
	if(rq.opcode1 == 1)
		printf("Root : %lf\n", ntohd(rq.d.n));
	else
		printf("Current time %.*s\n", rq.d.td.size_of_time - 6, rq.d.td.time_str);
}

int main ()
{
	int sockfd;
	socklen_t len;
	struct sockaddr_in address;
	int result;
	unsigned int request_id = 0;
	request rq;
	
	
	
	/*  Create a socket for the client.  */

	sockfd = socket (AF_INET, SOCK_STREAM, 0);

	/*  Name the socket, as agreed with the server.  */

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr ("127.0.0.1");
	address.sin_port = htons (9734);
	len = sizeof (address);

	/*  Now connect our socket to the server's socket.  */

	result = connect (sockfd, (struct sockaddr *) &address, len);

	if (result == -1)
	{
		perror ("oops: netclient");
		exit (1);
	}

	/*  We can now read/write via sockfd.  */
	while(1){
		char choice;
		puts("1. Square root\n2.Current time\n");
		scanf(" %c", &choice);

		if(choice == '1'){
			puts("Input your number");
			double d;
			scanf("%lf", &d);
			init_request_root(&rq, request_id++, d);
		}
		else
			init_request_time(&rq, request_id++);

		write (sockfd, &rq, sizeof(request));
		read (sockfd, &rq, sizeof(request));
		read_request(rq);
	}
	close (sockfd);
	exit (0);
}
