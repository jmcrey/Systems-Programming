# include <stdio.h>
# include <math.h>
# include <stdlib.h>
# include <errno.h>
# include <limits.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/stat.h>
# include <fcntl.h>
# define MSG_BUFFER 25

// Global board
char board[] ={ "123456789" };
int flag = 1;
int ingame = 0;
int first = -1;
int second = -1;
int turn = -1;
// Messages to send
char who[] = { "wWho are you?: "};
char next[] = { "nNext move: " };
char opp[] = { "oYour oppenent is: " };
char letter[] = { "lYour letter is: "};
char goodbye[] = {"You are disconnected."};
struct player {
    char handle[25];
    char XorO;
    int sd;
};

void printboard() {
    int i, count=0;
    for( i = 0; i < 9; i++ ) {
	if(count < 2) {
	    printf("  %c  |", board[i]);
	    count++;
	}
	else {
	    count=0;
	    printf("  %c  \n", board[i]);
	    printf("=====|=====|=====\n");
	}
    }
    printf("\n");
}


char gameover() {
    int i, count=0;
    for(i = 0; i < 9; i++) {
	if( board[i] == 'X' || board[i] == 'O' )
	    count++;
    }

    	if (
	    (board[0] == 'X' && board[3] == 'X' && board[6] == 'X') ||
	    (board[0] == 'X' && board[1] == 'X' && board[2] == 'X') ||
	    (board[0] == 'X' && board[4] == 'X' && board[8] == 'X') ||
	    (board[1] == 'X' && board[4] == 'X' && board[7] == 'X') ||
	    (board[3] == 'X' && board[4] == 'X' && board[5] == 'X') ||
	    (board[2] == 'X' && board[4] == 'X' && board[6] == 'X') ||
	    (board[2] == 'X' && board[5] == 'X' && board[8] == 'X') ||
	    (board[6] == 'X' && board[7] == 'X' && board[8] == 'X') 
           ) 
	    {	ingame = 0; return 'x';    }
	else if (
	    (board[0] == 'O' && board[3] == 'O' && board[6] == 'O') ||
            (board[0] == 'O' && board[1] == 'O' && board[2] == 'O') ||
            (board[0] == 'O' && board[4] == 'O' && board[8] == 'O') ||
            (board[1] == 'O' && board[4] == 'O' && board[7] == 'O') ||
            (board[3] == 'O' && board[4] == 'O' && board[5] == 'O') ||
            (board[2] == 'O' && board[4] == 'O' && board[6] == 'O') ||
            (board[2] == 'O' && board[5] == 'O' && board[8] == 'O') ||
            (board[6] == 'O' && board[7] == 'O' && board[8] == 'O')
  	        )
	{    ingame = 0; return 'o';    }

	else if( count >= 9 ) {
            ingame = 0; return 'd';
	}
	else
	    return 'c';

}

int game(struct player *players[]) {
    int i, flagger;
    if(!ingame) {
        for(i=0; i<6; i++) {
            if(players[i]->sd != 0 && players[i]->handle[0] && first == -1)
                first = i;
            else if(players[i]->sd != 0 && players[i]->handle[0] && second== -1)
                second=i;
        }
        if(first != -1 && second != -1) {
	    flagger= 1;
   //         startgame(players[first], players[second]);
        }
        else { first = -1; second = -1; flagger= 0;}
    }
    return flagger;
}

// Code from Professor Kearns
void printsin(struct sockaddr_in *sin, char *m1, char *m2 )
{
  char fromip[INET_ADDRSTRLEN];

  printf ("%s %s:\n", m1, m2);
  printf ("  family %d, addr %s, port %d\n", sin -> sin_family,
	    inet_ntop(AF_INET, &(sin->sin_addr.s_addr), fromip, sizeof(fromip)),
            ntohs((unsigned short)(sin -> sin_port)));
}

void sendmsgtype(char code, int *new_con, struct player * play) {
    int left, put, written;
    switch(code) {

	case 'w':
	    left = sizeof(who); put=0;
            while (left > 0) {
                if((written = write(*new_con, who+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
	    break;

	case 'o':
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
	   
	    left = sizeof(play->handle); put=0;
            while (left > 0) {
                if((written = write(*new_con, (play->handle)+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
            break;

	case 'l':
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
	    if( (written = write(*new_con, &(play->XorO), 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            } 
            break;

	case 'n':
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
	    left = sizeof(board); put = 0;
	    while (left > 0) {
                if((written = write(*new_con, board+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
	    break;

	case 'b':
	    left = sizeof(board); put = 0;
            while (left > 0) {
                if((written = write(*new_con, board+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
	    break;

	case 'd':
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
            left = sizeof(board); put = 0;
            while (left > 0) {
                if((written = write(*new_con, board+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
	    break;

	case 'x': // Win
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
	    left = sizeof(board); put = 0;
            while (left > 0) {
                if((written = write(*new_con, board+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
            break;

	case 't': // Lose
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
            left = sizeof(board); put = 0;
            while (left > 0) {
                if((written = write(*new_con, board+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
            break;

	case 'g': // Goodbye
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
            left = sizeof(goodbye); put = 0;
            while (left > 0) {
                if((written = write(*new_con, goodbye+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }
            break;

	case 'e':
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
	    break;
    }

}

void startgame(struct player *one, struct player *two) {
    ingame =1; turn=first;
    one->XorO = 'X'; two->XorO = 'O';
    sendmsgtype('o', &(one->sd), two); sendmsgtype('o', &(two->sd), one);
    sendmsgtype('l', &(one->sd), one); sendmsgtype('l', &(two->sd), two);
    sendmsgtype('n', &(one->sd), one); sendmsgtype('b', &(two->sd), two);
}

void makemove( int move, struct player * cur, struct player *players[] ) {
    int index = move-1, count=0;
    if( board[index] == 'X' || board[index] == 'O' ) {
        sendmsgtype('e', &(players[first]->sd), cur);
	sendmsgtype('e', &(players[second]->sd), cur);
	players[first]->sd = 0; players[second]->sd = 0;
        for( count = 0; count< 21; count++) {
            players[first]->handle[count] = '\0';
            players[second]->handle[count] = '\0';
        }
        first = -1; second = -1; turn=-1; ingame=0; strncpy(board, "123456789", 9);
        if(game(players) == 1) {
            startgame(players[first], players[second]);
        }
    }
    else {
        if( cur->XorO == 'X' )
            board[index] = 'X';
        else
            board[index] = 'O';
    }
}


void recvmsgtype(char code, int *sd, struct player * play, struct player * players[]) {
    size_t buffer;
    int val, count;
    char handle[21];
    switch (code) {

        case 'h':
            count=0; buffer = 21;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    //printf("read error\n");
                }
                else { handle[count] = code; if(code != '\0') count++; buffer -= 1; }
            }
            handle[count] ='\0';
            strcpy(play->handle, handle);
//            printf("Handle is: %s", play->handle);
            break;

        case 'n':
            buffer = 1;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    //printf("read error\n");
                }
                else { putchar(code); buffer -= 1; }
            }
            makemove( atoi(&code), play, players);
            if( gameover() == 'c' ) {
                if(turn == first)
                    turn = second;
                else
                    turn = first;
                sendmsgtype('n', &(players[turn]->sd), players[turn]);
            }
            else {
                if(gameover() == 'x') {
                    if(players[first]->XorO == 'X'){
                        sendmsgtype('x', &(players[first]->sd), players[first]);
                        sendmsgtype('t', &(players[second]->sd), players[second]);
                    }
                    else {
                        sendmsgtype('x', &(players[first]->sd), players[first]); 
                        sendmsgtype('t', &(players[second]->sd), players[second]);
			
                    }
                }
                else if(gameover() == 'o') {
                    if(players[first]->XorO == 'O'){
                        sendmsgtype('x', &(players[first]->sd), players[first]);
                        sendmsgtype('t', &(players[second]->sd), players[second]);
                    }
                    else {
                        sendmsgtype('t', &(players[first]->sd), players[first]);
                        sendmsgtype('x', &(players[second]->sd), players[second]);
                    }
                }
                else {
 		    sendmsgtype('d', &(players[first]->sd), players[first]); sendmsgtype('d', &(players[second]->sd), players[second]);
                }
		sendmsgtype('g', &(players[first]->sd), players[first]);
                sendmsgtype('g', &(players[second]->sd), players[second]);
                players[first]->sd = 0; players[second]->sd = 0;
		for( count = 0; count< 21; count++) {
		    players[first]->handle[count] = '\0';
		    players[second]->handle[count] = '\0';
		}
                first = -1; second = -1; turn=-1; strncpy(board, "123456789", 9);
	        if(game(players) == 1)
		   startgame(players[first], players[second]);
                }

            break;

	default:
	    for(count=0; count<6; count++) {
		if(players[count]->sd == *sd) {
		    if(count == first) {
			sendmsgtype('g', &(players[second]->sd), players[second]);
			players[first]->sd = 0; players[second]->sd = 0;
                    	for( count = 0; count< 21; count++) {
                            players[first]->handle[count] = '\0';
                            players[second]->handle[count] = '\0';
                        }
			first = -1; second = -1; turn = -1; ingame = 0; strncpy(board, "123456789", 9);
			if(game(players) == 1)
                      	    startgame(players[first], players[second]);
		    }
		    else if( count == second ) {
			sendmsgtype('g', &(players[first]->sd), players[first]);
			players[first]->sd = 0; players[second]->sd = 0;
                        for( count = 0; count< 21; count++) {
                            players[first]->handle[count] = '\0';
                            players[second]->handle[count] = '\0';
                        }
                        first = -1; second = -1; turn = -1; ingame = 0; strncpy(board, "123456789", 9);
                        if(game(players) == 1)
                            startgame(players[first], players[second]);
		    }
		    else {
		        players[count]->sd = 0;
		        for( val = 0; val< 21; val++) {
                            players[count]->handle[val] = '\0';
                        }
		    }
		    break;
		}
	    }
	    break;
    }
}


void service(int *listener, int *datagram) {
    int max_sd, sd, new_con, i, val, activity, count;
    fd_set readfds;
    struct sockaddr_in peer;
    socklen_t length, gramsize;
    char code;
    // Keeping track of players
    struct player *players[6], *one, *two, *three, *four, *five, *six;
    // For grams
    struct {
        char    head;
        char    response[3];
        char    handle1[21];
        char    handle2[21];
        char    tail;
    } gram;

    struct {
        char    head;
        char    response[3];
        char    handle1[21];
        char    handle2[21];
        char    tail;
    } got;

    // All connections set to 0 
    if( listen(*listener,6) < 0) {
        perror("listen");
        exit(1);
    }

    if((one = (struct player *) malloc(sizeof(struct player))) == NULL) {
        fprintf(stderr, "TTT: malloc failed\n");
        exit(1);
    }

    if((two = (struct player *) malloc(sizeof(struct player))) == NULL) {
        fprintf(stderr, "TTT: malloc failed\n");
        exit(1);
    }
    
    if((three = (struct player *) malloc(sizeof(struct player))) == NULL) {
        fprintf(stderr, "TTT: malloc failed\n");
        exit(1);
    }

    if((four = (struct player *) malloc(sizeof(struct player))) == NULL) {
        fprintf(stderr, "TTT: malloc failed\n");
        exit(1);
    }

    if((five = (struct player *) malloc(sizeof(struct player))) == NULL) {
        fprintf(stderr, "TTT: malloc failed\n");
        exit(1);
    }

    if((six = (struct player *) malloc(sizeof(struct player))) == NULL) {
        fprintf(stderr, "TTT: malloc failed\n");
        exit(1);
    }

    one->sd = 0; one->XorO = 'Z'; 
    two->sd = 0; two->XorO = 'Z'; 
    three->sd = 0; three->XorO = 'Z';
    four->sd = 0; four->XorO = 'Z';
    five->sd = 0; five->XorO = 'Z';
    six->sd = 0; six->XorO = 'Z';
    players[0] = one; players[1] = two; players[2] = three;
    players[3] = four; players[4] = five; players[5] = six;
    
// Code idea from http://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
   
    while(1) {
	gramsize = sizeof(peer);
	// Clear the socket set
	FD_ZERO(&readfds);

	// Add listener to set
	FD_SET(*listener, &readfds);
	FD_SET(*datagram, &readfds);
	if(*datagram > *listener)
	    max_sd = *datagram;
	else
	    max_sd = *listener;

	for ( i = 0; i < 6; i++) {
	    sd = players[i]->sd;
	    if( sd > 0 )
		FD_SET( sd, &readfds);
	    // Need max socket desc for select
	    if( sd > max_sd)
		max_sd = sd;
	}

	activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

	if ( activity < 0 && errno!=EINTR ) {
	    printf("select error\n");
	}

	if(FD_ISSET(*listener, &readfds)) {
	    if( (new_con = accept(*listener, (struct sockaddr *)&peer, &length)) < 0) {
	        perror("accept");
	        exit(1);
	    }
	// Send the handle message here
	// Make player object with first two connections
	    for( i = 0; i<6; i++ ) {
		if(players[i]->sd == 0) {
		    players[i]->sd = new_con;
		    sendmsgtype('w', &new_con, players[i]);
		    break;
		}
		else count++;
	    }
	    if( count == 6 ) {
		sendmsgtype('g', &new_con, players[i]);
		count = 0;
	    }
	    else count=0;
		    
	}
	if(FD_ISSET(*datagram, &readfds)) {
	    val = recvfrom(*datagram,&got,sizeof(got),0,(struct sockaddr *)&peer,&gramsize);
	    if (val < 0) perror("recv_udp:recvfrom");
	    else {
		getpeername(sd, (struct sockaddr*)&peer, &length);
		gram.head='<';
		gram.tail='>';
		if(ingame) {
		    strcpy(gram.response, "Yes");
		    strcpy(gram.handle1, players[first]->handle);
		    strcpy(gram.handle2, players[second]->handle);
		}
		else {
		    val =0;
		    strcpy(gram.response, "No");
		    strcpy(gram.handle2, "None");
		    for( i = 0; i<6; i++ ) {
                	if(players[i]->sd != 0 && players[i]->handle[0]!='\0') {
			    val ++;
                    	    strcpy(gram.handle1, players[i]->handle);
                            break;
                	}
            	    }
		    if(!val) {
			strcpy(gram.handle1, "None");
		    }
		    
	    	}
	        if( (val = sendto(*datagram, &gram, sizeof(gram), 0, (struct sockaddr *) &peer, sizeof(struct sockaddr_in))) < 0) {
		    perror("send_udp:sendto");
	        }
		for(count = 0; count < 3; count++)
		{  gram.response[count] = '\0'; }
		for( count = 0; count< 21; count++) {
                   gram.handle1[count] = '\0';
                   gram.handle2[count] = '\0';
                }
	    }
	}
	// Some kind of I/O activity
	for( i = 0; i < 6; i++) {
	    sd = players[i]->sd;
	    code = '\0';
	    if(FD_ISSET( sd, &readfds)) {
//    printf("\n\nRSTREAM:: data from stream:\n");
//    while ( read(conn, &ch, 1) == 1)
//        putchar(ch);
//    putchar('\n');
		if( (val = read(sd, &code, 1)) == 0) {
		    getpeername(sd, (struct sockaddr*)&peer, &length);
		    printf("Host disconnected, ip %s, port %d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
		    close(sd);
		    players[i]->sd =0;
		    for( count = 0; count< 21; count++) {
                   	players[i]->handle[count] = '\0';
                    }
		    if(i == first) {
			sendmsgtype('e', &(players[second]->sd), players[second]);
			players[second]->sd = 0;
			for( count = 0; count< 21; count++) {
                            players[second]->handle[count] = '\0';
                        }
			ingame = 0; first = -1;	second = -1; turn=-1; strcpy(board, "123456789");
			if(game(players) == 1)
			    startgame(players[first], players[second]);
		    }
		    if(i == second) {
			sendmsgtype('e', &(players[first]->sd), players[first]); strcpy(board, "123456789");
			players[first]->sd =0;
			for( count = 0; count< 21; count++) {
                            players[first]->handle[count] = '\0';
                        }
		        ingame = 0; first = -1; second = -1; turn= -1;
			if(game(players) == 1)
                            startgame(players[first], players[second]);
		    }
		}
		else {
		    recvmsgtype(code, &sd, players[i], players);
		    if(game(players) == 1)
			startgame(players[first], players[second]);
				
//		    while (read(sd, &rec, 1) == 1) {
//			putchar(rec);
//		    }
//		    putchar('\n');
		}
	    }
	}
    }
}
	
int main(int argc, char *argv[]) {
    
    // Code from Professor Kearns
    int listener, opt = 1, datagram;  /* fd for socket on which we get connection requests */
    int conn;      /* fd for socket thru which we pass data */
    struct sockaddr_in *localaddr, peer, *s_in;
    int ecode;
    socklen_t length;
    char hostname[256], port[256];
    struct addrinfo hints, *addrlist, gram_info, *gramlist;
    FILE *addr_info;

    if ( argc > 1 ) {
	fprintf(stderr, "usage: accepts no arguments\n");
	exit(1);
    }



   // Creating listener socket using Kearns' code

/* 
   Want to specify local server address of:
      addressing family: AF_INET
      ip address:        any interface on this system 
      port:              0 => system will pick free port
*/

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE; hints.ai_protocol = 0;
    hints.ai_canonname = NULL; hints.ai_addr = NULL;
    hints.ai_next = NULL;

    ecode = getaddrinfo(NULL, "0", &hints, &addrlist);
    if (ecode != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
       exit(1);
    }

    localaddr = (struct sockaddr_in *) addrlist->ai_addr;

  /* 
    For datagram
  */

    memset(&gram_info, 0, sizeof(gram_info));
    gram_info.ai_family = AF_INET; gram_info.ai_socktype = SOCK_DGRAM;
    gram_info.ai_flags = AI_NUMERICSERV | AI_PASSIVE; gram_info.ai_protocol = 0;
    gram_info.ai_canonname = NULL; gram_info.ai_addr = NULL;
    gram_info.ai_next = NULL;

    ecode = getaddrinfo(NULL, "13107", &gram_info, &gramlist);
    if (ecode != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
      exit(1);
    }

    s_in = (struct sockaddr_in *) gramlist->ai_addr;

    printsin(s_in, "FANCY_RECV_UDP", "Local socket is:"); fflush(stdout);

    datagram = socket (gramlist->ai_family, gramlist->ai_socktype, 0);
    if (datagram < 0) {
      perror ("recv_udp:socket");
      exit (1);
    }
    
    if (bind(datagram, (struct sockaddr *)s_in, sizeof(struct sockaddr_in)) < 0) {
      perror("recv_udp:bind");
      exit(1);
    }
 
  /*
     Create socket on which we will accept connections. This is NOT the
     same as the socket on which we pass data.
  */
    if ( (listener = socket( addrlist->ai_family, addrlist->ai_socktype, 0 )) < 0 ) {
        perror("inet_rstream:socket");
        exit(1);
    }
    
    if ( setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
	perror("setsockopt");
	exit(1);
    }

    if (bind(listener, (struct sockaddr *)localaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("inet_rstream:bind");
        exit(1);
    }

  /*
     Print out the port number assigned to this process by bind().
  */
    length = sizeof(struct sockaddr_in);
    if (getsockname(listener, (struct sockaddr *)localaddr, &length) < 0) {
        perror("inet_rstream:getsockname");
        exit(1);
    }

    if(gethostname(hostname, 256) != 0) {
	perror("inet_rstream:gehostname");
	exit(1);
    }
    sprintf(port, "%d", ntohs(localaddr->sin_port));
    printf("RSTREAM:: string of port number %s\n", port);
    printf("hostname: %s\n", hostname);

   /*
     Now accept a single connection. Upon connection, data will be
     passed through the socket on descriptor conn.
  */
    if( listen(listener,6) < 0) {
	perror("listen");
	exit(1);
    }
    if(( addr_info = fopen("addr_info", "w")) == NULL) {
	fprintf(stderr, "TTT: opening addr_info failed\n");
	exit(1);
    }
    if(fputs(hostname, addr_info) == EOF) {
	fprintf(stderr, "TTT: writing hostname addr_info failed\n");
        exit(1);
    }
    if(fputs("\n", addr_info) == EOF) {
        fprintf(stderr, "TTT: writing hostname addr_info failed\n");
        exit(1);
    }
    if(fputs(port, addr_info) == EOF) {
        fprintf(stderr, "TTT: writing port num addr_info failed\n");
        exit(1);
    }
    fclose(addr_info);
    length = sizeof(peer);
    service(&listener, &datagram);
    if ((conn=accept(listener, (struct sockaddr *)&peer, &length)) < 0) {
        perror("inet_rstream:accept");
        exit(1);
    }
    
    printsin(&peer,"RSTREAM::", "accepted connection from"); 

//    printf("\n\nRSTREAM:: data from stream:\n");
//    while ( read(conn, &ch, 1) == 1)
//        putchar(ch);
//    putchar('\n');
    
    exit(0);  
} 
