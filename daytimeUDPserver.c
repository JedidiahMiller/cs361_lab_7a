/* daytimeUDPserver.c - A UDP TIME server */
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
#include <time.h>

#include <signal.h>

#define MAXBUFLEN   256
#define IPSTRLEN    50
typedef struct sockaddr SA ;

/*------------------------------------------------------------------------
 * Error Handling Functions
 *----------------------------------------------------------------------*/
void err_sys(const char* msg) 
{ 
    fflush( stderr ) ;
    perror( msg ) ; 
    exit( 1 ); 
}

/*------------------------------------------------------------------------
 * Signal Handling Functions
 *----------------------------------------------------------------------*/

// This was copied in from the provided wrapper
typedef void Sigfunc( int ) ;
Sigfunc * sigactionWrapper( int signo, Sigfunc *func )
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset( &act.sa_mask );
	act.sa_flags   = 0;

	if( sigaction( signo, &act, &oact ) < 0 )
		return( SIG_ERR );

	return( oact.sa_handler );
}

int terminating = 0;
void sig_handler() {
    terminating = 1;
}

/*------------------------------------------------------------------------
 * main - Iterative UDP server for DAYTIME service
 *------------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{
   struct sockaddr_in  
                 srvrSkt ,         /* the address of this server   */
                 clntSkt ;         /* remote client's socket       */
    unsigned short port = 13 ;     /* service name or port number  */
    int	    sd ;                   /* server socket                */
    time_t	now ;                  /* current time                 */
    char    buf[ MAXBUFLEN ] ;     /* "input" buffer; any size > 0 */
    char    ipStr[ IPSTRLEN ] ;    /* dotted-dec IP addr. */
    char    timeStr[ MAXBUFLEN ] ; /* time string   */

    unsigned int    alen;          /* from-address length          */

    sigactionWrapper( SIGTERM, sig_handler );
    sigactionWrapper( SIGINT, sig_handler );

	switch (argc) 
	{
      case 1:
        break;
      case 2:
        port = atoi( argv[1] );
        break;
      default:
        snprintf(buf, MAXBUFLEN , "usage: %s [port]\n" , argv[0] );
        err_sys( buf ) ;
    }

    sd = socket( AF_INET, SOCK_DGRAM  , 0 ) ;
    if ( sd < 0 )
        err_sys( "Could NOT create socket" ) ;

    // Prepare the server's socket address structure
    memset( (void *) & srvrSkt , 0 , sizeof( srvrSkt ) );
    srvrSkt.sin_family      = AF_INET;
    srvrSkt.sin_addr.s_addr = htonl( INADDR_ANY );
    srvrSkt.sin_port        = htons( port ) ;

    // Now, bind the server to above socket
    if ( bind( sd, (SA *) & srvrSkt , sizeof(srvrSkt) ) < 0 )
    {
        snprintf( buf, MAXBUFLEN, "Could NOT bind to port %d", port );
        err_sys( buf ) ;
    }

    inet_ntop( AF_INET, (void *) & srvrSkt.sin_addr.s_addr , ipStr , IPSTRLEN ) ;
    printf( "Bound socket %d to IP %s Port %d\n" , sd , ipStr , ntohs( srvrSkt.sin_port ) );    

    while (!terminating) 
    {
        alen = sizeof( clntSkt ) ;
        fprintf( stderr , "DAYTIME server waiting\n" ) ;        

        if ( recvfrom( sd , buf , MAXBUFLEN , 0 , (SA *) & clntSkt , & alen ) < 0 ) {
            if ( errno == EINTR ) break;
            err_sys( "recvfrom" ) ;
        }

        fprintf( stderr , "DAYTIME server received '%s'\n" , buf ) ;

        inet_ntop( AF_INET, (void *) & clntSkt.sin_addr.s_addr , ipStr , IPSTRLEN ) ;
        printf("from IP %s Port %d\n" , ipStr , ntohs( clntSkt.sin_port ) ) ;

        time( &now ); /* get the current system's time */
	    ctime_r( &now , timeStr ) ;    /* WARNING! ctime() is NOT thread-safe */
        timeStr[ strlen(timeStr) -1 ] = 0 ;  // remove the unwanted '\n' character. I hate it!!!
        fprintf( stderr , "DAYTIME server sending '%s'\n" , timeStr ) ;

		// Must also send the '\0' as part of the timeStr. That's why 1+strlen()
        sendto( sd , (void *) timeStr , 1 + strlen( timeStr ) , 0 , (SA *) & clntSkt , alen ) ;
	}

    printf("Goodbye!\n");
    close( sd );
}
