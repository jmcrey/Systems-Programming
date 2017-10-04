#include <sys/stat.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <limits.h>
#include <sys/sysmacros.h> 
#include <time.h> 
#include <ctype.h>
#include <string.h>
#define NILNODE (struct node *)0

struct node {
     char data[4096];
     struct node *next;
};

void append(struct node *head, struct node *cur, char *name) {	
     struct node *tmp;
     char *str;
     if((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
	fprintf(stderr, "accessed: couldn't construct node in linked list\n");
	exit(1);
    }
    str = strncpy(tmp->data, name, 4096);
    tmp->next = NILNODE;
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
    tmp->next = NILNODE;
    return(tmp);
}

int main(int argc, char *argv[]) {

    struct stat info;
    struct node *makehead(), *head;
    char name[4096], *dummy;
    int within;
    long days, accessed;
    
    if (argc != 2) {
	fprintf(stderr, "accessed: usage accessed filename\n");
	exit(1);
    }

    if ( argv[1][0] == '-' ) {
	within = 1;
	argv[1]++;
    }
    else {
	within = 0;
    }

    days = strtol(argv[1], &dummy, 10);
    if(dummy == argv[1] || *dummy != '\0' || 
	((days == LONG_MIN || days == LONG_MAX) && errno == ERANGE)) {
	fprintf(stderr, "accessed: usage accessed num\n");
	exit(1);
    }
    if( days == 0 ) {
	fprintf(stderr, "accessed: usage cannot be 0 days\n");
	exit(1);
    }

    dummy = "I am working";
    head = makehead(dummy);
    while(fscanf(stdin, "%4096s", name) != EOF) {
	    if(stat(name, &info) != 0) {
		fprintf(stderr, "accessed: could not stat %s\n", name);
		perror(NULL);
	    }
	    if(S_ISREG(info.st_mode)) {
		append(head, head->next, name);
		memset(name, 0, 4096);
	   }
    }

    head = head->next;

    while(head != NILNODE) {
	stat(head->data, &info);
	accessed = (time(NULL) - info.st_atime) / (60*60*24);
	if( within == 1 && accessed <= days ) {
	    printf("%s\n", head->data);
	}
	if( within == 0 && accessed > days ) {
	    printf("%s\n", head->data);
	}
	head = head->next;
    }

    exit(0);
}
