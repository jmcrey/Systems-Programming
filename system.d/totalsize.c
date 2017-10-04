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

struct node {
    char data[4096];
    ino_t inum;
    struct node *next;
};

void append(struct node *head, struct node *cur, char *name, ino_t inum) {	
     struct node *tmp;
     char *str;
     if((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
	fprintf(stderr, "accessed: couldn't construct node in linked list\n");
	exit(1);
    }
    str = strncpy(tmp->data, name, 4096);
    tmp->next = NILNODE;
    tmp->inum = inum;
    if(cur == NILNODE) {
	head->next = tmp;
    }
    else {
    	while(cur->next != NILNODE) {
	    cur = cur->next;
    	}
        cur->next = tmp;
    }
}

struct node *makehead(char *data) {
    struct node *tmp;
    char *str;
    if((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
        fprintf(stderr, "accessed: couldn't construct node in linked list\n");
        exit(1);
    }
    str = strncpy(tmp->data, data, 4096);
    tmp->next = NILNODE; tmp->inum = 0;
    return(tmp);
}

int main(int argc, char *argv[]) {

    struct stat info;
    struct node *makehead(), *head, *cur;
    char name[4096], *dummy;
    int already = 0;
    long total = 0, sleepy, pid;
    if (argc > 2) {
	fprintf(stderr, "totalsize: usage totalsize\n");
	exit(1);
    }

    dummy = "I am working";
    head = makehead(dummy);
    cur = head;
    while(fscanf(stdin, "%4096s", name) != EOF) {
	    if(stat(name, &info) != 0) {
		fprintf(stderr, "totalsize: could not stat %s\n", name);
		perror(NULL);
	    }
	    if(S_ISREG(info.st_mode)) {
		while( cur != NILNODE ) {
		    if( cur->inum == info.st_ino ) {
			already = 1;
			break;
		    }
		    cur = cur->next;
		}
		cur = head;
		if( already == 0 ) { 
		    append(head, head->next, name, info.st_ino);
		}
		memset(name, 0, 4096);
	   }
    }

    if(getenv("TSTALL") != NULL) {
            sleepy = strtol(getenv("TSTALL"), &dummy, 10);
            if(dummy == getenv("TSTALL") || *dummy != '\0' || ((sleepy == LONG_MIN || sleepy == LONG_MAX)
                && errno == ERANGE )) {
                fprintf(stderr, "totalsize: tstall is incorrect\n");
            }
	    if(sleepy > 100) { sleepy = 0; }
	    
    }

    head = head->next;
    while(head != NILNODE) {
	if(getenv("TSTALL") != NULL) {
	    sleep((int)sleepy);
	    stat(head->data, &info);
	    total += (long)info.st_size;
	    head = head->next;
	}
	else {
            stat(head->data, &info);
            total += (long)info.st_size;
            head = head->next;
	}
    }

    // Printing
    if(getenv("UNITS") != NULL) {
	if ( ( (strcmp(getenv("UNITS"), "K") == 0) || (strcmp(getenv("UNITS"), "k") ==0)) && strlen(getenv("UNITS")) == 1) {
	    printf("%ldkB\n", (total/1024));
	}
	else {
    	    printf("%ld\n", total);
	}
    }
    else printf("%ld\n", total);

    if(getenv("TMOM") != NULL) {
            pid = strtol(getenv("TMOM"), &dummy, 10);
            if(dummy == getenv("TMOM") || *dummy != '\0' || ((pid == LONG_MIN || pid == LONG_MAX)
                && errno == ERANGE )) {
                fprintf(stderr, "totalsize: tmom is incorrect");
            }
            else {
                kill((pid_t) pid, SIGUSR1); 
	    }
    }
//    printf("(device,i_number)=(%d/%d,%ld)", major(info.st_dev),  
//                                minor(info.st_dev),(long)info.st_ino);

//    printf(" last accessed %ld seconds ago\n", time(NULL) - info.st_atime);

//    if(S_ISREG(info.st_mode)) {
//	printf("\tdesignated a regular file\n");
//    }
    exit(0);
}
