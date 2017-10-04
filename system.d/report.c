#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <limits.h>
#include <sys/sysmacros.h> 
#include <time.h> 
#include <ctype.h>
#include <string.h>
#include <signal.h>
#define NILNODE (struct node *)0

int flag=1;
void usr1handler() { flag = 0; }

int main(int argc, char *argv[]) {

//    struct stat info;
//    struct node *makehead(), *head, *cur;
    char tmp[4096], unaccessed[4096], accessed[4096], pidstr[4096], delaystr[4096], *argv1[2], *argv2[1]; //, name[4096], *dummy, *units, *tstall, *tmom;
//  Setting up the pipes 
    int p2a1[2], p2a2[2], a1_to_ts1[2], a2_to_ts2[2], ts1_to_p[2], ts2_to_p[2];
    int count=0, days, i, kb, delay=0;
    size_t buf = 1;
    pid_t pid;
//    long total = 0, sleepy, pid;

    if (argc < 2) {
	fprintf(stderr, "totalsize: usage totalsize\n");
	exit(1);
    }

    if ((days = atoi(argv[1])) <= 0) {
	fprintf(stderr, "report: first argument must be non-zero positive integer\n");
	exit(1);
    }
    argv1[0] = "./accessed";
    argv1[1] = argv[1];
    argv2[0] = "./totalsize"; 

    for(i=2; i < argc; i++) {
	if (strcmp(argv[i], "-k") == 0) {
	    kb = 1;
	}
	else if (strcmp(argv[i], "-d") == 0) {
	    i++;
	    if(argv[i] != NULL) {
	    	if((delay = atoi(argv[i])) <= 0) {
		    fprintf(stderr, "report: postive non-zero int after -d\n");
		    exit(1);
	    	}
	    }
	    else {
		fprintf(stderr, "report: postive non-zero int after -d\n");
                exit(1);
	    }
	}
	else {
	    fprintf(stderr, "report: usage wrong argument\n");
	    exit(1);
	}
    }

    pid = getpid();
    sprintf(pidstr, "%d", (int)pid);

    pipe(p2a1); pipe(a1_to_ts1); pipe(ts1_to_p); //first system
    pipe(p2a2); pipe(a2_to_ts2); pipe(ts2_to_p); //Second system

    // First accessed
    if(fork() == 0) {
	sprintf(argv1[1], "%d", days);
	close(p2a1[1]); 
	// Close all of 2nd system pipes
	close(p2a2[0]); close(p2a2[1]); 
	close(a2_to_ts2[0]); close(a2_to_ts2[1]);
	close(ts2_to_p[0]); close(ts2_to_p[1]);
	// Close other unnessary pipes
	close(ts1_to_p[0]); close(ts1_to_p[1]);
	close(a1_to_ts1[0]);
	// Take stdin from the parent
	close(0); dup(p2a1[0]); close(p2a1[0]);
	// Make sure to output onto the pipe
	close(1); dup(a1_to_ts1[1]); close(a1_to_ts1[1]);
	execv("./accessed", (char *[]) { "./accessed", argv1[1], NULL} );
	fprintf(stderr, "report: first accessed exec failed\n");
    }
//    close(p2a1[0]); close(p2a1[1]);    

    // Second accessed
    if(fork() == 0) {
	sprintf(argv1[1], "%d", (days*-1));
        close(p2a2[1]);
	// Close all of first system
	close(p2a1[0]); close(p2a1[1]);
        close(a1_to_ts1[0]); close(a1_to_ts1[1]);
        close(ts1_to_p[0]); close(ts1_to_p[1]);
	// Close other unncessary pipes
	close(a1_to_ts1[0]);
	close(ts2_to_p[0]); close(ts2_to_p[1]);
        // Take stdin from the parent
        close(0); dup(p2a2[0]); close(p2a2[0]);
        // Make sure to output onto the pipe
        close(1); dup(a2_to_ts2[1]); close(a2_to_ts2[1]);
        execv("./accessed", (char *[]) { "./accessed", argv1[1], NULL });
	fprintf(stderr, "report: second accessed exec failed\n");
    }
//    close(p2a2[0]); close(p2a2[1]);

    // First totalsize
    if(fork() == 0) {
	// Seting the environment
	setenv("TMOM", pidstr, 1);
        if( kb == 1 ) {
            putenv("UNITS=k");
        }
        if(delay > 0) {
            sprintf(delaystr, "%d", delay);
            setenv("TSTALL", delaystr, 1);
        }

        close(p2a1[1]); close(p2a1[0]);
        close(a1_to_ts1[1]);
        // Close all of 2nd system pipers
        close(p2a2[0]); close(p2a2[1]);
        close(a2_to_ts2[0]); close(a2_to_ts2[1]);
        close(ts2_to_p[0]); close(ts2_to_p[1]);
	// Close other unnessary pipes
	close(ts1_to_p[0]);
	// Dup output of a1 to stdin and close it
        close(0); dup(a1_to_ts1[0]); close(a1_to_ts1[0]);
	// Dup output of ts1 to stdout and close it
	close(1); dup(ts1_to_p[1]); close(ts1_to_p[1]);
        execv("./totalsize", argv2);
    }

//    else {
//	close(a1_to_ts1[0]); close(a1_to_ts1[1]); // Close pipes to from a1 to ts1
//	close(ts1_to_p[1]);
//	wait();
//	close(0); dup(ts1_to_p[0]); close(ts1_to_p[0]);
//    } 


    // Second totalsize
    if(fork() == 0) {
	// Setting the environment
	setenv("TMOM", pidstr, 1);
        if( kb == 1 ) {
            putenv("UNITS=k");
        }
        if(delay > 0) {
            sprintf(delaystr, "%d", delay);
            setenv("TSTALL", delaystr, 1);
        }

        close(p2a2[1]); close(p2a2[0]);
        close(a2_to_ts2[1]);
	// Close all of 1st system
	close(p2a1[0]); close(p2a1[1]);
        close(a1_to_ts1[0]); close(a1_to_ts1[1]);
        close(ts1_to_p[0]); close(ts1_to_p[1]);
	// Close other unncessary pipes
	close(ts2_to_p[0]);
        // Dup output of a1 to stdin and close it
        close(0); dup(a2_to_ts2[0]); close(a2_to_ts2[0]);
        // Dup output of ts1 to stdout and close it
        close(1); dup(ts2_to_p[1]); close(ts2_to_p[1]);
        execv("./totalsize", argv2);
    }

//    else {
        
//    }

    while(read(STDIN_FILENO, tmp, buf) > 0) {
        write(p2a1[1], &tmp, 1); // write stdin to first pipe
        write(p2a2[1], &tmp, 1); // write stdin to second pipe
    }

    // Close everything
    // After 1st accessed
    close(p2a1[0]); close(p2a1[1]);
    // After 2nd accessed
    close(p2a2[0]); close(p2a2[1]);
    // After 1st totalsize
    close(a1_to_ts1[0]); close(a1_to_ts1[1]); // Close pipes to from a1 to ts1
    close(ts1_to_p[1]);
    // After 2nd totalsize 
    close(a2_to_ts2[0]); close(a2_to_ts2[1]); // Close pipes to from a1 to ts1
    close(ts2_to_p[1]);

    signal(SIGUSR1, usr1handler);
    while(flag) { sleep(1); printf("*"); }
    printf("\n");    

    // Read off of both input pipes
    while(read(ts1_to_p[0], tmp, buf) > 0) {
        if(isspace(tmp[0])) {
            unaccessed[count] = '\0';
        }
        else {
            unaccessed[count] = tmp[0];
            count++;
        }
    }
    count = 0;
    while(read(ts2_to_p[0], tmp, buf) > 0) {
	if(isspace(tmp[0])) {
	    accessed[count] = '\0';
	}
	else {
            accessed[count] = tmp[0];
            count++;
	}
    }
    count = 0;
    
    
    printf("A total of %s bytes are in regular files not accessed for %d days\n", unaccessed, days);
    printf("-----------------------------\n");
    printf("A total of %s bytes are in regular files accessed within %d days\n", accessed, days);
    exit(0);
}
