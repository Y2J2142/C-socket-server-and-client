#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

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

void read_request(request * rq)
{
	rq->opcode0 = 4;
	if(rq->opcode1 == 1)
	{
		rq->d.n = sqrt(ntohd(rq->d.n));
		rq->d.n = ntohd(rq->d.n);
	}
	else{
		time_t now = time(0);
		rq->d.td.size_of_time = strlen(ctime(&now));
		strcpy(rq->d.td.time_str, ctime(&now));
		
	}
}


int
main ()
{
	int server_sockfd, client_sockfd;
	socklen_t server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	server_sockfd = socket (AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl (INADDR_ANY);
	server_address.sin_port = htons (9734);
	server_len = sizeof (server_address);
	bind (server_sockfd, (struct sockaddr *) &server_address, server_len);

	/*  Create a connection queue, ignore child exit details and wait for clients.  */

	listen (server_sockfd, 5);

	signal (SIGCHLD, SIG_IGN);

	while (1)
	{
		request rq;

		printf ("server waiting\n");

		/*  Accept connection.  */

		client_len = sizeof (client_address);
		client_sockfd = accept (server_sockfd,
				(struct sockaddr *) &client_address,
				&client_len);

		/*  Fork to create a process for this client and perform a test to see
			whether we're the parent or the child.  */

		if (fork () == 0)
		{

			/*  If we're the child, we can now read/write to the client on client_sockfd.
				The five second delay is just for this demonstration.  */
			while(1){
			read (client_sockfd, &rq, sizeof(request));
			read_request(&rq);
			send(client_sockfd, &rq, sizeof(request), 0);
			}
			close (client_sockfd);
			exit (0);
		}

		/*  Otherwise, we must be the parent and our work for this client is finished.  */

		else
		{
			close (client_sockfd);
		}
		
	}
}
