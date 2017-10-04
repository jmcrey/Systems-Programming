# include <stdio.h>
# include <math.h>
# include <stdlib.h>
# include <errno.h>
# include <limits.h>

void start(double *x, double *y, signed long size) {
    *x = size/2;
    *y = size/2;
}

int move(double *x, double *y, long *time, signed long size) {
    long rand; 
    double xshift, yshift;

    rand = random();
    xshift = cos( rand ); 
    yshift = sin( rand );
    
    *x += xshift;
    *y += yshift;


    if ( *x > size || *y > size || *x < 0 || *y < 0) {
	*time += 1;
	return 1;
    }
    else {
	*time += 2; 
	return 0; 
    }
}

//void count(int *time, int fall) {
//    if(fall == 1) {
//	*time += 1;
//    }
//    else {
//	*time += 2;
//    }
//}

void output(long time, signed long beetles, signed long size) {
    double avg = time / beetles;
    printf("%ld by %ld,  beetles %ld, mean beetle lifetime is %.1f\n", size, size, beetles, avg);
}

int main(int argc, char *argv[]) { 
    
// VALIDATION CODE FOUND ON STACKOVERFLOW: http://stackoverflow.com/questions/14176123/correct-usage-of-strtol 
    errno = 0;
    // Initializing necessary variables 
    signed long size, beetles;
    long i, time;
    double x, y;
    int fall = 0;
    int count=0;

    // Accepting and validating input
    char *ptr;
    size = strtol(argv[1], &ptr, 10);
    if ( ptr == argv[1] || *ptr != '\0' || ((size == LONG_MIN || size == LONG_MAX) && errno == ERANGE)) {
	fprintf(stderr, "Usage: please enter valid input\n");
	exit(1);
    }
    beetles = strtol(argv[2], &ptr, 10);
    if ( ptr == argv[1] || *ptr != '\0' || ((beetles == LONG_MIN || beetles == LONG_MAX) && errno == ERANGE) ) {
        fprintf(stderr, "Usage: please enter valid input\n");
        exit(1);
    }
    if( size <= 0 || beetles <= 0 ) {
	fprintf(stderr, "Usage: please enter valid input\n");
	exit(1);
    }

//    printf("size: %ld, beetles: %ld\n", size, beetles);

    // Running the program
    i = 0;
    start(&x, &y, size);
    while(i < beetles) {
	fall = move(&x, &y, &time, size);
	count++;
	if(fall) {
	    i++;
	    start(&x, &y, size);
	    fall = 0;
	}
    }

    output(time, beetles, size);
    exit(0);
}
