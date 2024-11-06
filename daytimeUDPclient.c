/* daytimeUDPclient.c - main */
/* Modified by ABOUTABL from Comer's code */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define MAXBUFLEN   256
#define IPSTRLEN    50
typedef struct sockaddr SA ;

/*------------------------------------------------------------------------
 * Error Handling Functions
 *----------------------------------------------------------------------*/
void err_sys(const char* msg) 
{ 
    fflush( stderr ) ;
    perror(msg); 
    exit(1); 
}

/*------------------------------------------------------------------------
 * main - UDP client for DAYTIME service that prints the resulting time
 *----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char	    *serverIP;
    unsigned short port;

    char    *msg     = "Please, tell me the time and date." ;
    char    ipStr[ IPSTRLEN ] ;        /* dotted-dec IP addr. */
    char    timeStr[ MAXBUFLEN ];      /* time string   */ 
    int	    sd, n;                     /* socket descriptor, read count */

    if ( argc < 3 ) 
    {
      fprintf(stderr, "usage: %s  serverIP  port \n" , argv[0] );
      exit(1);
    }
     
    serverIP          = argv[1] ;
    port              = (unsigned short) atoi( argv[2] ) ;

    /* Allocate a socket */
	sd = socket( AF_INET, SOCK_DGRAM , 0 ) ;
	if (sd < 0)
		err_sys( "Could NOT create socket" ) ;
    
    // Prepare the server's socket address structure
    struct sockaddr_in srvSkt ;	   /* Server's socket structrue  */
    memset( (void *) & srvSkt , 0 , sizeof( srvSkt ) );

    srvSkt.sin_family   = AF_INET;
    srvSkt.sin_port     = htons( port ) ;

    if( inet_pton( AF_INET, serverIP , (void *) & srvSkt.sin_addr.s_addr ) != 1 )
      err_sys( "Invalid server IP address" ) ;

    fprintf(stderr , "DAYTIME client sending '%s'\n" , msg ) ;

    /* must send the \0 at end of msg, too */
    sendto ( sd, (void *) msg , 1 + strlen( msg ) , 0 , (SA *) & srvSkt , sizeof( srvSkt )  ) ;

    unsigned alen = sizeof( srvSkt ) ;
    n = recvfrom ( sd, (void *) timeStr , MAXBUFLEN , 0 , (SA *) & srvSkt , & alen  );
    if ( n <= 0 )
        err_sys( "Failed to get the daytime from the server" );

    inet_ntop( AF_INET, (void *) & srvSkt.sin_addr.s_addr , ipStr , IPSTRLEN ) ;

    printf( "DAYTIME client received '%s'\n"  , timeStr ) ;
    printf( "From IP: %s , Port: %hu\n" , ipStr , ntohs( srvSkt.sin_port ) ) ;
    close( sd ) ;
    exit(0);
}

