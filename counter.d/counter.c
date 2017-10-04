# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <limits.h>
# include <pthread.h>
# include <time.h>
# define NILNODE (struct node *)0


int numlines, maxcounter, readerdone, numcounters, empty;
long filedelay, threaddelay;
char names[26] = { "abcdefghijklmnopqrstuvwxyz" };
// Lovely Data that is controlled by mutexes
struct data {
    char *linebuffer[2048];
    int readpos, writepos;
    int length;
    pthread_mutex_t lock;
    pthread_cond_t notfull;
    pthread_cond_t notempty;
    struct timespec readdelay, countdelay;
};

// global data
struct data *data;

// Lovely node structure
struct node {
    char word[256];
    int count;
    struct node *next, *prev;
};

// global lists
struct lists {
    struct node *even, *odd;
    pthread_mutex_t lock;
};

struct lists *lists;

// Lovely args data holder
struct readholder {
    int argc;
    pthread_t th_count;
    char *argv[];
};

// Makes the head of the node and any of the file name nodes
struct node *makehead() {
    struct node *tmp; 
    if((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
	fprintf(stderr, "rgpp: malloc failed in makehead\n");
	exit(1);
    }
    //strncpy(tmp->word, data, 256);
    tmp->prev = NILNODE; tmp->next = NILNODE; tmp->count=1;
    return(tmp);
}

// Makes the tree of line numbers associated with the filename
void addnode(char val[256], struct node *head) {
    int set=0;
    struct node *tmp, *cur;
    cur= head;
    while(cur != NILNODE) {
	if(cur->word[0] == '\0') {
	    strncpy(cur->word, val, 256); set++;
	    break;
	}
	if(strcmp(val, cur->word) < 0) {
    	    if ((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
 	        fprintf(stderr, "rgpp: malloc failed in making tree\n");
	        exit(1);
    	    }
    	    strncpy(tmp->word, val, 256); tmp->count = 1;
	    if(cur->prev == NILNODE) {
		tmp->next = head; head->prev = tmp; tmp->prev = NILNODE;
		head = tmp;
		set++;
		break;
	    }
	    else {
		cur->prev->next = tmp;
		tmp->next = cur; tmp->prev = cur->prev;
		cur->prev = tmp;
		set++;
		break;
	    }
	}
	else if(strcmp(val, cur->word) == 0) {
	    cur->count++; set++;
	    break;
	}
	else {	
	    if(cur->next != NILNODE)
		cur = cur->next;
	    else
		break;
	}
    }

    if(set != 1) {
	if ((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
            fprintf(stderr, "rgpp: malloc failed in making tree\n");
            exit(1);
        }
        strncpy(tmp->word, val, 256); tmp->count = 1;
	tmp->prev = cur; tmp->next = NILNODE;
	cur->next = tmp;
    }	
    	
}

void init(struct data *data, struct lists *lists) {
    // For line buffer
    pthread_mutex_init(&data->lock, NULL);
    pthread_cond_init(&data->notempty, NULL);
    pthread_cond_init(&data->notfull, NULL);
    data->readpos = 0; data->writepos = 0; data->length = 0;
    data->readdelay.tv_sec = (time_t) (((int)filedelay)/1000); data->readdelay.tv_nsec = 0;
    data->countdelay.tv_sec = (time_t) (((int)threaddelay)/1000); data->countdelay.tv_nsec = 0;

    // For linked lists
    lists->even = makehead();
    lists->odd = makehead();
    pthread_mutex_init(&lists->lock, NULL);
}

static void *counter(void * nodes) {
    int i;
    char line[2048], word[256];
    int name = numcounters;
    void * retval;
    empty = 0;
    // Handlind the type casting
    while (empty == 0) {
        // Critical section
	pthread_mutex_lock(&data->lock);
		
        while(data->readpos == data->writepos) {
	    if(readerdone == 1 && data->length == 0) {
		// This will make sure all counter threads terminate once the first one does
		empty++; numcounters--;
		pthread_cond_signal(&data->notempty);
                pthread_mutex_unlock(&data->lock);
		pthread_exit(&retval);
	    }
            pthread_cond_wait(&data->notempty, &data->lock);
        }
	if(data->length != 0) {
            strcpy(line, data->linebuffer[data->readpos]);
            data->linebuffer[data->readpos][0] = '\0';
            data->length--; data->readpos++;
            if(data->readpos == numlines)
                data->readpos = 0;
	}
        pthread_cond_signal(&data->notfull);
        pthread_mutex_unlock(&data->lock);
        // End critical section
        // Made thread sleep
        nanosleep(&(data->countdelay), NULL);
        // Critical section when multiple counter threads
        pthread_mutex_lock(&lists->lock);
        if(line[0] != '\0') {
                // Get word
                for(i = 0; sscanf(line+i, "%s", word) == 1; i+=strlen(word)+1 ) {
                    if( (strlen(word) % 2) == 0 ){
                        addnode(word, lists->even);
                    }
                    else {
                        addnode(word, lists->odd);
                    }
                }
        }
        printf("counter: My name is %c\n", names[name]);
        pthread_mutex_unlock(&lists->lock);
        // End counter critical section
        line[0] = '\0';
    }
    empty++; numcounters--;
    return NULL;
}

static void *counterone(void * nodes) {
    int i;
    char line[2048], word[256];
    int name = numcounters;
    void * retval;
    empty = 0;
    // Handlind the type casting
    while (empty == 0) {
        // Critical section
        pthread_mutex_lock(&data->lock);

        while(data->length == 0) {
            if(readerdone == 1) {
                // This will make sure all counter threads terminate once the first one does
                empty++; numcounters--;
                pthread_cond_signal(&data->notempty);
                pthread_mutex_unlock(&data->lock);
                pthread_exit(&retval);
            }
            pthread_cond_wait(&data->notempty, &data->lock);
        }
        if(data->length != 0) {
            strcpy(line, data->linebuffer[data->readpos]);
            data->linebuffer[data->readpos][0] = '\0';
            data->length--; data->readpos++;
            if(data->readpos == numlines)
                data->readpos = 0;
        }
        pthread_cond_signal(&data->notfull);
        pthread_mutex_unlock(&data->lock);
        // End critical section
        // Made thread sleep
        nanosleep(&(data->countdelay), NULL);
        // Critical section when multiple counter threads
        pthread_mutex_lock(&lists->lock);
        if(line[0] != '\0') {
                // Get word
                for(i = 0; sscanf(line+i, "%s", word) == 1; i+=strlen(word)+1 ) {
                    if( (strlen(word) % 2) == 0 ){
                        addnode(word, lists->even);
                    }
                    else {
                        addnode(word, lists->odd);
                    }
                }
        }
        printf("counter: My name is %c\n", names[name]);
        pthread_mutex_unlock(&lists->lock);
        // End counter critical section
        line[0] = '\0';
    }
    empty++; numcounters--;
    return NULL;
}

static void *reader(void *read) {
    int i, readfile=0, count=0;
    char line[2048];
    FILE *fp;
    numcounters = 0; // this is for naming the new threads when printing

    // Handling the type casting
    int argc = ((struct readholder *)read)->argc;
    for(i = 9; i< argc; i++) {
	if( (fp = fopen(((struct readholder *)read)->argv[i], "r")) != NULL) {
	    readfile=1;
	    while(fgets(line, 2048, fp) != NULL) {
		// Critical Section
		pthread_mutex_lock(&data->lock);
		while(((data->writepos + 1) % numlines == data->readpos)) {
		    if( (numcounters+1) != maxcounter) { // the length is here to account for spurious wake ups
			numcounters++;
		        pthread_create(&(((struct readholder *)read)->th_count), NULL, counter, 0); 
		    }
		    pthread_cond_wait(&data->notfull, &data->lock);
		}
		data->linebuffer[data->writepos] = strdup(line);
		data->length++; data->writepos++; count++;
		if(data->writepos == numlines) 
		    data->writepos = 0;
		pthread_cond_signal(&data->notempty);
		pthread_mutex_unlock(&data->lock);
		// End Critical section
		if (count == 2) {
		    count = 0;
		    nanosleep(&(data->readdelay), NULL);
		}
	     }
	}
	else {
	    printf("File %s could not be opened. Continuing.\n", ((struct readholder *)read)->argv[i]);
	    readfile = 0;
	}
    }
    // Reader is done - letting the counter threads know
    readerdone = 1;
    if(readfile!=1) { // Makes sure the last file was actually correctly read
	pthread_cond_signal(&data->notempty); // If not read signal children to stop
    }
    return NULL;
}

static void *readerone(void *read) {
    int i, readfile=0, count=0;
    char line[2048];
    FILE *fp;
    numcounters = 0; // this is for naming the new threads when printing

    // Handling the type casting
    int argc = ((struct readholder *)read)->argc;
    for(i = 9; i< argc; i++) {
        if( (fp = fopen(((struct readholder *)read)->argv[i], "r")) != NULL) {
	    readfile=1; // File was correctly read
            while(fgets(line, 2048, fp) != NULL) {
                // Critical Section
                pthread_mutex_lock(&data->lock);
                while(data->length == 1) {
                    if( (numcounters+1) != maxcounter) { // the length is here to account for spurious wake ups
                        numcounters++;
                        pthread_create(&(((struct readholder *)read)->th_count), NULL, counter, 0);
                    }
                    pthread_cond_wait(&data->notfull, &data->lock);
                }
                data->linebuffer[data->writepos] = strdup(line);
                data->length++; data->writepos++; count++;
                if(data->writepos == numlines)
                    data->writepos = 0;
                pthread_cond_signal(&data->notempty);
                pthread_mutex_unlock(&data->lock);
                // End Critical section
                if (count == 2) {
                    count = 0;
                    nanosleep(&(data->readdelay), NULL);
                }
             }
        }
        else {
            printf("File %s could not be opened. Continuing.\n", ((struct readholder *)read)->argv[i]);
	    readfile = 0;
        }
    }
    // Reader is done - letting the counter threads know
    readerdone = 1;
    if(readfile!=1) { // Makes sure a file was actually correctly read
        pthread_cond_signal(&data->notempty); // If not read signal children to stop
    }
    return NULL;
}	

void printlists(struct node *even, struct node *odd) {
    printf("\n\nPrinting even list:\n\n"); 
    while(even != NILNODE) {
	printf("%s, %d\n", even->word, even->count);
	even = even->next;
    }
    printf("\n\nPrinting odd list:\n\n");
    while(odd != NILNODE) {
	printf("%s, %d\n", odd->word, odd->count);
	odd = odd->next;
    }
    printf("\n\nPrinting completed\n");

}

int main(int argc, char *argv[]) {
    // All necessary variables
    int j;
    struct readholder *read;
    void * retval;
    // Check args in argv - SO MANY DUMB CASES
    char *ptr;
    // Threads 
    pthread_t th_read;

    if (argc < 9) {
	fprintf(stderr, "Usage: You don't have proper amount of arguments\n");
	exit(1);
    }

    for(j=1; j < 9; j++) {
	if((strcmp(argv[j], "-d")) == 0) {
	    j++;
	    if(argv[j] != NULL) {
	    	filedelay = strtol(argv[j], &ptr, 10);  
	    	if ( ptr == argv[j] || *ptr != '\0' || ((filedelay == LONG_MIN || filedelay == LONG_MAX) && errno == ERANGE)) {
        	    fprintf(stderr, "Usage: please enter valid input\n");
        	    exit(1);
    	        }
	    }
	    else {
		fprintf(stderr, "usage: must follow -d with integer\n");
	    }
	}
	
	else if((strcmp(argv[j], "-D")) == 0) {
            j++;
	    if(argv[j] != NULL) {
		threaddelay = strtol(argv[j], &ptr, 10);
                if ( ptr == argv[j] || *ptr != '\0' || ((threaddelay == LONG_MIN || threaddelay == LONG_MAX) && errno == ERANGE)) {
                    fprintf(stderr, "Usage: please enter valid input\n");
                    exit(1);
                }
	    }
	    else {
		fprintf(stderr, "Usage: must follow -D with integer\n");
                exit(1);
	    }
        }

	else if((strcmp(argv[j], "-b")) == 0) {
            j++;
            if(argv[j] != NULL) {    
                if ( (numlines = atoi(argv[j])) <= 0) {
                    fprintf(stderr, "Usage: please enter valid input\n");
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Usage: must follow -b with integer greater than 0\n");
                exit(1);
            }
        }

	else if((strcmp(argv[j], "-t")) == 0) {
            j++;
	    if(argv[j] != NULL) {
		maxcounter = atoi(argv[j]);
                if ( maxcounter  <= 0 || maxcounter > 26) {
                    fprintf(stderr, "Usage: please enter valid input\n");
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Usage: must follow -t with integer greater than 0\n");
                exit(1);
            }
        }

	else {
	    fprintf(stderr, "Usage: incorrect argument\n");
	    exit(1);
	} 
    }
    
    // Handling the arguments for reader
    if((read = (struct readholder *) malloc(sizeof(struct readholder))) == NULL) {
        fprintf(stderr, "rgpp: malloc failed in makehead\n");
        exit(1);
    }
    read->argc = argc;
    for(j=0; j<argc; j++) {
        read->argv[j] = strdup(argv[j]);
    }
    if((data = (struct data *) malloc(sizeof(struct data))) == NULL) {
        fprintf(stderr, "rgpp: malloc failed in makehead\n");
        exit(1);
    }
    if((lists = (struct lists *) malloc(sizeof(struct lists))) == NULL) {
        fprintf(stderr, "rgpp: malloc failed in makehead\n");
        exit(1);
    }
    init(data, lists);
    // Calling the threads
    if (numlines == 1) {
	pthread_create(&th_read, NULL, readerone, (void *)read);
        pthread_create(&(read->th_count), NULL, counterone, 0);
    }
    else {
    	pthread_create(&th_read, NULL, reader, (void *)read);
    	pthread_create(&(read->th_count), NULL, counter, 0);
    }
    // Wait for the threads to finish
    pthread_join(th_read, &retval);
    pthread_join(read->th_count, &retval);
    // Make sure all threads terminate before printing
    while( numcounters != -1) {}
    printlists(lists->even, lists->odd); 
    exit(0);
}
