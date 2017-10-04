#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

char handle[21];
char board[] = { "123456789" };
char move[2];
char flag;

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

void sendmsgtype(char code, int *new_con) {
    int left, put, written;
    switch(code) {

        case 'h':
            if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
            left = sizeof(handle); put=0;
            while (left > 0) {
                if((written = write(*new_con, handle+put, left)) < 0) {
                    perror("inet_wstream:write");
                    exit(1);
                }
                else left-=written;
                put += written;
            }

            break;

        case 'n':
	    if( atoi(move) <= 0 || atoi(move) > 9 ) {
                fprintf(stderr, "usage: must choose valid position\n");
                exit(1);
            }
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
            if( (written = write(*new_con, move, 1)) < 0) {
                perror("inet_wstream:write");
                exit(1);
            }

            break;
	case 'e':
	    if( (written = write(*new_con, &code, 1)) < 0) {
                perror("inet_wstream:write");
                    exit(1);
            }
	    fprintf(stderr, "usage: must choose valid position\n");
	    exit(1);
	    break;
    }

}

void recvmsgtype(char code, int *sd) {
    size_t buffer;
    int val, count=0;
    switch (code) {
	case 'g':
	    buffer = 21;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { putchar(code); buffer -= 1; }
            }
	    putchar('\n');
	    exit(1);
	    break;

	case 'w':
	    buffer = 15;
	    while(buffer > 0) {
		if( (val = read(*sd, &code, 1)) == 0) {
		    printf("read error\n");
		}
		else { putchar(code); buffer -= 1; }
	    }
	    putchar('\n');
//	    scanf("%s", handle);
//	    if(strlen(handle) > 20){
//        	fprintf(stderr, "usage: handle must be less than 20 chars\n");
//        	exit(1);
//    	    }
//	    sendmsgtype('h', sd);
	    break;
	
	case 'o':
	    printf("Your opponent is: ");
	    buffer = 21;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { if(code != '\0') { putchar(code); buffer -= 1;} 
			else break;}
            }
	    putchar ('\n');
	    break;

	case 'l':
	    printf("Your letter is: ");
	    buffer = 1;
	    while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { putchar(code); buffer -= 1; }
            }
	    putchar('\n');
	    break;

	case 'b':
	    board[count]=code; count++;
	    buffer = 9;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { board[count] =code; count++; buffer -= 1; }
            }
            printboard();
            break;
	
	case 'n':
            buffer = 9;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { board[count] =code; count++; buffer -= 1; }
            }
	    putchar('\n');
            printboard();
	    printf("Your move: ");
	    scanf("%s", move);
	    if(atoi(move) <= 0) {
		sendmsgtype('e', sd);
		exit(1);
	    }
	    else
	        sendmsgtype('n', sd);
            break;

	case 'd':
	    buffer = 9;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { board[count] =code; count++; buffer -= 1; }
            }
            printboard();
	    printf("Draw\n");
	    break;

	case 'x':
	    buffer = 9;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { board[count] =code; count++; buffer -= 1; }
            }
            printboard();
            printf("You win\n");
            break;

	case 't':
	    buffer = 9;
            while(buffer > 0) {
                if( (val = read(*sd, &code, 1)) == 0) {
                    printf("read error\n");
                }
                else { board[count] =code; count++; buffer -= 1; }
            }
            printboard();
            printf("You lose\n");
            break;

	case 'e':
	    printf("There was an error\n");
	    exit(1);
    }
}


int main(argc, argv)
int argc;
char **argv;
{
  int sock, dgram=0, max_sd, ecode, activity, val;
  struct sockaddr_in *server, *gramdest, peer;
  socklen_t gramsize;
  struct addrinfo hints, *addrlist, gram_info, *gramlist;
  char hostname[256], port[256];
  FILE *fp;
  fd_set readfds; 
  // -q
  struct {
    char   head;
    char   response[3];
    char   handle1[21];
    char   handle2[21];
    char   tail;
  } gram;

  struct  {
    char   head;
    char   response[3];
    char   handle1[21];
    char   handle2[21];
    char   tail;
  } got;

  // -t
   struct timeval timeout;
   int time =0;
/*
   Want a sockaddr_in containing the ip address for the system
   specified in argv[1] and the port specified in argv[2].
*/

  memset( &hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV; hints.ai_protocol = 0;
  hints.ai_canonname = NULL; hints.ai_addr = NULL;
  hints.ai_next = NULL;

/*
  Reading from the file to get hostname and port number
*/

  if( ( fp = fopen("addr_info", "r")) == NULL) {
     sleep(60);
     if( ( fp = fopen("addr_info", "r")) == NULL) {
	fprintf(stderr, "ttt: server info does not exist\n");
	exit(1);
     }
  }
  fscanf(fp, "%s %s", hostname, port);
  printf("hostname: %s, portnum: %s\n", hostname, port);

  // If in query mode:
  if(argc > 1 ) {

    if(strcmp(argv[1], "-q") == 0) {
      memset( &gram_info, 0, sizeof(gram_info));
      gram_info.ai_family = AF_INET; gram_info.ai_socktype = SOCK_DGRAM;
      gram_info.ai_flags = AI_NUMERICSERV; gram_info.ai_protocol = 0;
      gram_info.ai_canonname = NULL; gram_info.ai_addr = NULL;
      gram_info.ai_next = NULL;

      ecode = getaddrinfo(hostname, "13107", &gram_info, &gramlist);
      if (ecode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
        exit(1);
      }
    
      gramdest = (struct sockaddr_in *) gramlist->ai_addr;

      if ( (dgram = socket( gramlist->ai_family, gramlist->ai_socktype, 0 )) < 0 ) {
        perror("inet_wstream:socket");
        exit(1);
      }

      gram.head = '<';
      gram.tail = '>';

      if( (val = sendto(dgram,&gram,sizeof(gram),0,(struct sockaddr *) gramdest,
                  sizeof(struct sockaddr_in))) < 0) {
    	  printf("val: %d", val);
          perror("send_udp:sendto");
          exit(1);
      }
    }

    else if( strcmp(argv[1], "-t") == 0 ) {
      if(argc < 3 || argc > 4) {
	fprintf(stderr, "usage: ttt -t val\n");
	exit(1);
      }
      else {
	if( (time = atoi(argv[2])) <= 0 ) {
	    fprintf(stderr, "usage: ttt -t val must be an int\n");
	    exit(1);
	}
      }
    }	
  }


  ecode = getaddrinfo(hostname, port, &hints, &addrlist);
  if (ecode != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
    exit(1);
  }

  server = (struct sockaddr_in *) addrlist->ai_addr;

/*
   Create the socket.
*/
  if ( (sock = socket( addrlist->ai_family, addrlist->ai_socktype, 0 )) < 0 ) {
    perror("inet_wstream:socket");
    exit(1);
  }

/*
   Connect to data socket on the peer at the specified Internet
   address.
*/
  if(argc <= 3) {
    if ( connect(sock, (struct sockaddr *)server, sizeof(struct sockaddr_in)) < 0) {
      perror("inet_wstream:connect");
      exit(1);
    }
 }

  for (;;) {
    // Listens for activity on the socket
    // Clear the socket set
    FD_ZERO(&readfds);

    // Add listener to set
    FD_SET(0, &readfds);
    FD_SET(sock, &readfds);
    if(dgram!=0)
	FD_SET(dgram, &readfds);
    if(dgram != 0 && dgram > sock)
    	max_sd = dgram;
    else
	max_sd = sock;

    timeout.tv_sec = (time_t)time;
    timeout.tv_usec = 0;

    if(timeout.tv_sec > 0)
      activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

    else
      activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if ( activity < 0 && errno!=EINTR ) {
      printf("select error\n");
    }
    if(activity == 0) {
	printf("Shutting down\n");
	exit(0);
    }
    if(activity > 0 && FD_ISSET(0, &readfds)) {
	if(flag == 'w') {
	    scanf("%s", handle);
            sendmsgtype('h', &sock);
	}
    } 
    if(FD_ISSET(sock, &readfds)) {
	if( (val = read(sock, &flag, 1)) == 0) {
	    fprintf(stderr, "read error\n");
        }
        else {
	  recvmsgtype(flag, &sock);
        }
    }
    if(dgram != 0 && FD_ISSET(dgram, &readfds)) {
	val = recvfrom(dgram,&got,sizeof(got),0,(struct sockaddr *)&peer,&gramsize);
	if( val < 0 ) perror("recv_udp:recvfrom");
	else {
	    printf("%c%s %s %s%c\n", got.head, got.response, got.handle1, got.handle2, got.tail);
	}
	break;
    }	

}
 
  exit(0);
}

