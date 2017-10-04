/************************************************************************
 			         ttt.c
 Simple ttt client. No queries, no timeouts.
 Uses deprecated address translation functions.

 Phil Kearns
 April 12, 1998
 Modified March 2014
************************************************************************/

#include "common.h"

//void dump_board();
int start_child();

int main(int argc, char **argv)

{
  char hostid[128], handle[32], opphandle[32], junk;
  char my_symbol, other_symbol; /* X or O ... specified by server in MATCH message */
  char board[9], result[80];
  unsigned short xrport;
  int sock, sfile;
  struct sockaddr_in remote;
  struct hostent *h;
  int num, i, move, valid, finished, my_move, my_moves[9], bell, resign;
  struct tttmsg inmsg, outmsg;

  // For GUI
  FILE *read_from, *write_to;
  int childpid;

  if (argc != 1) {
    fprintf(stderr,"ttt:usage is ttt\n");
    exit(1);
  }

  /* Get host,port of server from file. */

  if ( (sfile = open(SFILE, O_RDONLY)) < 0) {
    perror("TTT:sfile");
    exit(1);
  }
  i=0;
  while (1) {
    num = read(sfile, &hostid[i], 1);
    if (num == 1) {
      if (hostid[i] == '\0') break;
      else i++;
    }
    else {
      fprintf(stderr, "ttt:error reading hostname\n");
      exit(1);
    }
  }
  if (read(sfile, &xrport, sizeof(int)) != sizeof(unsigned short)) {
    fprintf(stderr, "ttt:error reading port\n");
      exit(1);
  }
  close(sfile);


  /* Got the info. Connect. */

  if ( (sock = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
    perror("ttt:socket");
    exit(1);
  }

  bzero((char *) &remote, sizeof(remote));
  remote.sin_family = AF_INET;
  if ((h = gethostbyname(hostid)) == NULL) {
    perror("ttt:gethostbyname");
    exit(1);
  }
  bcopy((char *)h->h_addr, (char *)&remote.sin_addr, h->h_length);
  remote.sin_port = xrport;
  if ( connect(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0) {
    perror("ttt:connect");
    exit(1);
  }

  /* We're connected to the server. Engage in the prescribed dialog */

  /* Await WHO */

  bzero((char *)&inmsg, sizeof(inmsg));  
  getmsg(sock, &inmsg);
  if (inmsg.type != WHO) protocol_error(WHO, &inmsg);
  
  /* Send HANDLE */

  printf("Enter handle (31 char max):");
  fgets(handle, 31, stdin);
  bzero((char *)&outmsg, sizeof(outmsg));
  outmsg.type = HANDLE;
  strncpy(outmsg.data, handle, 31); outmsg.data[31] = '\0';
  putmsg(sock, &outmsg);

  /* Create GUI */
  
  childpid = start_child("wish",&read_from,&write_to);
  fprintf (write_to, "source splot.tcl\n");
//  if (fgets (result, 80, read_from) <= 0) exit(0); /* Exit if wish dies */
  fprintf (write_to, " .sf.handle configure -text \"Your name is: %s\"\n", handle);
  fprintf (write_to, "set y 0\n");
  bell = 1;
  /* Await MATCH */

  bzero((char *)&inmsg, sizeof(inmsg));  
  getmsg(sock, &inmsg);
  if (inmsg.type != MATCH) protocol_error(MATCH, &inmsg);
  my_symbol = inmsg.board[0];
  strncpy(opphandle, inmsg.data, 31); opphandle[31] = '\0';
//  printf("You are playing %c\t your opponent is %s\n\n", my_symbol, opphandle);
  if(my_symbol == 'X') { other_symbol = 'O'; my_move = 1; }
  else { other_symbol = 'X'; my_move = 0; }
  fprintf (write_to, ".sf.opponent configure -text \"Your opponent is %s He is %c\"\n", opphandle, other_symbol);
  fprintf (write_to, ".sf.handle configure -text \"Your name is: %s You are %c\"\n", handle, my_symbol);
  /* In the match */
  if( my_move <1 ) 
    { fprintf (write_to, " .sf.status configure -text \"Awaiting Opponent move\"\n"); 
      fprintf (write_to, " set y 1\n");
    }
  else {
    fprintf (write_to, " set y 0\n");
  }
  for(i=0; i<9; i++) board[i]=' ';
  for(i=0; i<9; i++) my_moves[i]=0;
  finished = 0;
  while(!finished){

    /* Await WHATMOVE/RESULT from server */
    
    bzero((char *)&inmsg, sizeof(inmsg));  
    getmsg(sock, &inmsg);
    switch (inmsg.type) {

    case WHATMOVE:
      for(i=0; i<9; i++) board[i]=inmsg.board[i];
      dump_board(write_to,board,my_symbol,my_moves);
      do {
        valid = 0;
//        printf("Enter your move: ");
	fprintf (write_to, " set y 1\n");
	fprintf (write_to, ".sf.status configure -text \"Your move\"\n");
	if(bell == 1)
	   fprintf (write_to, "bell\n");

	if (fgets (result, 80, read_from) <= 0) exit(0); /* Exit if wish dies */
	if ((num = sscanf(result, "%d %d %d", &move, &bell, &resign)) == 3) { }
	else { fprintf(stderr, "Bad command: %s", result); }
//        num = scanf("%d", &move);
//	printf("I am getting here\n");
//	fprintf (write_to, "set y 0\n");
//	fprintf (write_to, ".sf.status configure -text \"Awaiting Opponent move\"\n");
	if (num == EOF) {
	  fprintf(stderr,"ttt:unexpected EOF on standard input\n");
	  exit(1);
	}
	if (num == 0) {
	  if (fread(&junk, 1, 1, stdin)==EOF) {
	    fprintf(stderr,"ttt:unexpected EOF on standard input\n");
	    exit(1);
	  }
	continue;
	}
	if ((num ==3) && (resign == 1)) 
	{
	  bzero((char *)&outmsg, sizeof(outmsg));
      	  outmsg.type = RESULT;
          putmsg(sock, &outmsg);
	  fprintf (write_to, ".sf.status configure -text \"You Lose.\"\n");
//      printf("Draw\n");
          fprintf (write_to, ".bf.exit configure -command {exit}\n");
	  return(0);
	}
	if ((num == 3) && (move >= 1) && (move <= 9) )valid=1;
        if ((valid) && (board[move-1] != ' ')){ valid=0; if(bell==0)fprintf (write_to, "bell\n");}
      } while (!valid);
    
      /* Send MOVE to server */
      fprintf (write_to, "set y 0\n");
      fprintf (write_to, ".sf.status configure -text \"Awaiting Opponent move\"\n");
      my_moves[move-1]=1;
      bzero((char *)&outmsg, sizeof(outmsg));
      outmsg.type = MOVE;
      sprintf(&outmsg.res, "%c", move-1);
      putmsg(sock, &outmsg);
      break;

    case RESULT:
      for(i=0; i<9; i++) board[i]=inmsg.board[i];
      dump_board(write_to,board,my_symbol,my_moves);
      switch (inmsg.res) {
      case 'W':
	fprintf (write_to, ".sf.status configure -text \"You win\"\n");
	draw_win(write_to,board);
	fprintf (write_to, ".bf.exit configure -command {exit}\n");
//	printf("You win\n");
	break;
      case 'L':
	fprintf (write_to, ".sf.status configure -text \"You lose\"\n");
	draw_win(write_to,board);
	fprintf (write_to, ".bf.exit configure -command {exit}\n");
//	printf("You lose\n");
	break;
      case 'D':
	fprintf (write_to, ".sf.status configure -text \"Draw\"\n");
//	printf("Draw\n");
	fprintf (write_to, ".bf.exit configure -command {exit}\n");
	break;
      case 'R':
	fprintf (write_to, ".sf.status configure -text \"Your opponent resigned. You Win.\"\n");
//      printf("Draw\n");
        fprintf (write_to, ".bf.exit configure -command {exit}\n");
      default:
	fprintf(stderr,"Invalid result code\n");
	exit(1);
      }
      finished = 1;
      break;

    default:
      protocol_error(MOVE, &inmsg);
    }
  }
  return(0);
}

void
dump_board(FILE *s, char *board, char my_symbol, int my_moves[9])
{
  if(board[0] != ' ') {
    if(my_symbol == 'X' && my_moves[0] == 1) {
      fprintf(s, ".c create line 0 0 100 100 -fill red\n");
      fprintf(s, ".c create line 100 0 0 100 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[0] == 1) {
      fprintf(s, ".c create oval 0 0 100 100\n");
    }
    else {
	if(my_symbol == 'X')
	    fprintf(s, ".c create oval 0 0 100 100\n");
	else {
	    fprintf(s, ".c create line 0 0 100 100 -fill red\n");
            fprintf(s, ".c create line 100 0 0 100 -fill red\n");
	}
    }
  }

  if(board[1] != ' ') {
    if(my_symbol == 'X' && my_moves[1] == 1) {
      fprintf(s, ".c create line 0 100 100 200 -fill red\n");
      fprintf(s, ".c create line 100 100 0 200 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[1] == 1){
      fprintf(s, ".c create oval 0 100 100 200\n");
    }
    else {
      if(my_symbol == 'X')
	fprintf(s, ".c create oval 0 100 100 200\n");
      else {
	fprintf(s, ".c create line 0 100 100 200 -fill red\n");
        fprintf(s, ".c create line 100 100 0 200 -fill red\n");
     }
    }
  }

  if(board[2] != ' ') {
    if(my_symbol == 'X' && my_moves[2] == 1) {
      fprintf(s, ".c create line 0 200 100 300 -fill red\n");
      fprintf(s, ".c create line 100 200 0 300 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[2] == 1){
      fprintf(s, ".c create oval 0 200 100 300\n");
    }
    else {
	if(my_symbol == 'X') 
	    fprintf(s, ".c create oval 0 200 100 300\n");
	else {
	    fprintf(s, ".c create line 0 200 100 300 -fill red\n");
	    fprintf(s, ".c create line 100 200 0 300 -fill red\n");
	}
    }
  }

  if(board[3] != ' ') {
    if(my_symbol == 'X'&& my_moves[3] == 1) {
      fprintf(s, ".c create line 100 0 200 100 -fill red\n");
      fprintf(s, ".c create line 200 0 100 100 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[3] == 1){
      fprintf(s, ".c create oval 100 0 200 100\n");
    }
    else {
	if(my_symbol == 'X') 
	     fprintf(s, ".c create oval 100 0 200 100\n");
	else {
	     fprintf(s, ".c create line 100 0 200 100 -fill red\n");
	     fprintf(s, ".c create line 200 0 100 100 -fill red\n");
	}
    }
  }

  if(board[4] != ' ') {
    if(my_symbol == 'X' && my_moves[4] == 1) {
      fprintf(s, ".c create line 100 100 200 200 -fill red\n");
      fprintf(s, ".c create line 200 100 100 200 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[4] == 1) {
      fprintf(s, ".c create oval 100 100 200 200\n");
    }
    else {
	if(my_symbol=='X')
	    fprintf(s, ".c create oval 100 100 200 200\n");
	else {
	     fprintf(s, ".c create line 100 100 200 200 -fill red\n");
	     fprintf(s, ".c create line 200 100 100 200 -fill red\n");
	}
    }
  }

  if(board[5] != ' ') {
    if(my_symbol == 'X' && my_moves[5] == 1) {
      fprintf(s, ".c create line 100 200 200 300 -fill red\n");
      fprintf(s, ".c create line 200 200 100 300 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[5] == 1) {
      fprintf(s, ".c create oval 100 200 200 300\n");
    }
    else {
	if(my_symbol == 'X') 
	    fprintf(s, ".c create oval 100 200 200 300\n");
	else {
	    fprintf(s, ".c create line 100 200 200 300 -fill red\n");
	    fprintf(s, ".c create line 200 200 100 300 -fill red\n");
	}
    }
  }

  if(board[6] != ' ') {
    if(my_symbol == 'X' && my_moves[6] == 1) {
      fprintf(s, ".c create line 200 0 300 100 -fill red\n");
      fprintf(s, ".c create line 300 0 200 100 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[6] == 1) {
      fprintf(s, ".c create oval 200 0 300 100\n");
    }
    else {
	if(my_symbol=='X')
	   fprintf(s, ".c create oval 200 0 300 100\n"); 
	else {
	   fprintf(s, ".c create line 200 0 300 100 -fill red\n");
	   fprintf(s, ".c create line 300 0 200 100 -fill red\n");
	}
    }
  }

  if(board[7] != ' ') {
    if(my_symbol == 'X' && my_moves[7] == 1) {
      fprintf(s, ".c create line 200 100 300 200 -fill red\n");
      fprintf(s, ".c create line 300 100 200 200 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[7] == 1) {
      fprintf(s, ".c create oval 200 100 300 200\n");
    }
    else {
	if(my_symbol=='X')
	  fprintf(s, ".c create oval 200 100 300 200\n");
	else {
	  fprintf(s, ".c create line 200 100 300 200 -fill red\n");
	  fprintf(s, ".c create line 300 100 200 200 -fill red\n");
	}
    }
  }

  if(board[8] != ' ') {
    if(my_symbol == 'X' && my_moves[8] == 1) {
      fprintf(s, ".c create line 200 200 300 300 -fill red\n");
      fprintf(s, ".c create line 300 200 200 300 -fill red\n");
    }
    else if(my_symbol == 'O' && my_moves[8] == 1) {
      fprintf(s, ".c create oval 200 200 300 300\n");
    }
    else {
	if(my_symbol=='X')
	  fprintf(s, ".c create oval 200 200 300 300\n");
	else {
	  fprintf(s, ".c create line 200 200 300 300 -fill red\n");
          fprintf(s, ".c create line 300 200 200 300 -fill red\n");
	}
    }
  }	   
//  fprintf(s,"%c | %c | %c\n", board[0], board[1], board[2]);
//  fprintf(s,"----------\n");
//  fprintf(s,"%c | %c | %c\n", board[3], board[4], board[5]);
//  fprintf(s,"----------\n");
 // fprintf(s,"%c | %c | %c\n", board[6], board[7], board[8]);
}

void draw_win(FILE *s, char board[9]) {

    if (
            (board[0] == 'X' && board[1] == 'X' && board[2] == 'X') ||
	    (board[0] == 'O' && board[1] == 'O' && board[2] == 'O')
	)
	fprintf(s, ".c create line 50 0 50 0 -fill blue\n");
    if (
            (board[0] == 'X' && board[4] == 'X' && board[8] == 'X') ||
	    (board[0] == 'O' && board[4] == 'O' && board[8] == 'O')
	)
	fprintf(s, ".c create line 0 0 300 300 -fill blue\n");
    if (
            (board[1] == 'X' && board[4] == 'X' && board[7] == 'X') ||
	    (board[1] == 'O' && board[4] == 'O' && board[7] == 'O')
	)
	fprintf(s, ".c create line 0 150 300 150 -fill blue\n");
    if (
            (board[3] == 'X' && board[4] == 'X' && board[5] == 'X') ||
	    (board[3] == 'O' && board[4] == 'O' && board[5] == 'O')
	)
	fprintf(s, ".c create line 150 0 150 300 -fill blue\n");
    if (
            (board[2] == 'X' && board[4] == 'X' && board[6] == 'X') ||
	    (board[2] == 'O' && board[4] == 'O' && board[6] == 'O')
	)
	fprintf(s, ".c create line 300 0 0 300 -fill blue\n");
    if (
            (board[2] == 'X' && board[5] == 'X' && board[8] == 'X') ||
	    (board[2] == 'O' && board[5] == 'O' && board[8] == 'O')
	)
	fprintf(s, ".c create line 0 250 300 250 -fill blue\n");
    if (
            (board[6] == 'X' && board[7] == 'X' && board[8] == 'X') ||
	    (board[6] == 'O' && board[7] == 'O' && board[8] == 'O')
       )
	fprintf(s, ".c create line 250 0 250 0 -fill blue\n");

    if (
            (board[0] == 'X' && board[3] == 'X' && board[6] == 'X') ||
            (board[0] == 'O' && board[3] == 'O' && board[6] == 'O')
       )
        fprintf(s, ".c create line 0 150 300 150 -fill blue\n");
}
int
start_child(char *cmd, FILE **readpipe, FILE **writepipe) {
    int childpid, pipe1[2], pipe2[2];
    
    if ((pipe(pipe1) < 0) || (pipe(pipe2) < 0) ) {
       perror("pipe"); exit(-1);
    }

    if ((childpid = fork()) < 0) {
       perror("fork"); exit(-1);
    } else if (childpid > 0) {   /* Parent. */
       close(pipe1[0]); close(pipe2[1]);
       /* Write to child on pipe1[1], read from child on pipe2[0]. */
       *readpipe = fdopen(pipe2[0], "r");
       *writepipe = fdopen(pipe1[1], "w");
       setlinebuf(*writepipe);
       return childpid;

    } else { /* Child. */
       close(pipe1[1]); close(pipe2[0]);
       /* Read from parent on pipe1[0], write to parent on pipe2[1]. */
       dup2(pipe1[0],0); dup2(pipe2[1],1);
       close(pipe1[0]); close(pipe2[1]);

       if (execlp(cmd, cmd, NULL) < 0)
          perror("execlp");
       /* Never returns */
    }
    return 0;  // to keep the compiler happy
}
