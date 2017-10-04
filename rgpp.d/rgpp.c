# define _GNU_SOURCE
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define NILNODE (struct node *)0
# define RED "\x1b[31m"
# define RESET "\x1b[0m"

// Lovely node structure
struct node {
    char data[256];
    struct node *tree, *right, *next;
};


// Makes the head of the node and any of the file name nodes
struct node *makehead(char *data, struct node *tree, struct node *right, struct node *next) {
    struct node *tmp;
    char *str; 
    if((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
	fprintf(stderr, "rgpp: malloc failed in makehead\n");
	exit(1);
    }
    str = strncpy(tmp->data, data, 256); 
    tmp->tree = tree; tmp->right = right; tmp->next = next;
    return(tmp);
}

// Makes the tree of line numbers associated with the filename
void maketree(char *val, struct node *head, struct node *tree, struct node *right, struct node *next) {
    if(tree == NILNODE) {
	struct node *tmp;
    	char *str;
    	if ((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
            fprintf(stderr, "rgpp: malloc failed in making tree\n");
            exit(1);
        }
    	str = strncpy(tmp->data, val, 256);
	head->tree = tmp;
    }
    else {
    	while(tree->right != NILNODE) {
	    tree = tree->right;
    	}
    	struct node *tmp;
    	char *str;
    	if ((tmp = (struct node *) malloc(sizeof(struct node))) == NULL) {
 	    fprintf(stderr, "rgpp: malloc failed in making tree\n");
	    exit(1);
    	}
    	str = strncpy(tmp->data, val, 256); 
    	tmp->tree = tree; tmp->right = right; tmp->next = next;
    	tree->right = tmp;
    }
}

void lineMode(struct node *head, int nummode) {
    FILE *fp;
    char * line = NULL; 
    size_t len;
    ssize_t read;
    int count, linenum;
    struct node *tree;

    if(head != NILNODE) {

    	while (head != NILNODE) { 
    	    if( (fp = fopen(head->data, "r")) != NULL ) {
		printf("========================= %s\n", head->data);
		tree = head->tree;
		count = 0;
		while( (read = getline(&line, &len, fp)) != -1 ) {
		    count++;
		// Print numbers
		    if(nummode == 1) {
			if (count == (linenum = atoi(tree->data))) {
			    if(tree->right != NILNODE) {
			    	tree = tree->right;
			    }
                     	    printf("->%d: %s", count, line);
			}
			else {
			    printf("  %d: %s", count, line);
			}
		    }
		// Don't print numbers
		    else {
			if (count == (linenum = atoi(tree->data))) {
                            if(tree->right != NILNODE) {
                                tree = tree->right;
                            }
                            printf("-> %s", line);
                        }
                        else {
                            printf("   %s", line);
                        }			
		    }
                }
	    }
	   
	    else {
		fprintf(stderr, "Usage: file does not exist");
		exit(1);
	    }
	    fclose(fp);

	    head = head->next;
	}
    }

    else {
	fprintf(stderr, "mklist: List was not built");
	exit(1);
    }
}

// Yes, I brute forced this code; DON'T JUDGE ME

void wordMode(struct node *head, char *word, int nummode) {

    FILE *fp;
    char * line = NULL;
    char color[256];
    char tmp1;
    size_t len;
    ssize_t read;
    int count, linenum, i, l, k;
    struct node *tree;

    if(head != NILNODE) {

        while (head != NILNODE) {
            if( (fp = fopen(head->data, "r")) != NULL ) {
                printf("========================= %s\n", head->data);
                tree = head->tree;
                count = 0;
                while( (read = getline(&line, &len, fp)) != -1 ) {
                    count++;
                // Print numbers
                    if(nummode == 1) {
			// Line has word
                        if (count == (linenum = atoi(tree->data))) {
                            if(tree->right != NILNODE) {
                                tree = tree->right;
                            }
			    printf("  %d: ", count);
			// Read word by word
			    for(i = 0; sscanf(line+i, "%s", color) == 1; i+=strlen(color)+1 ) {
				// Word contains substring
				if( strcasestr(color, word) ) {
				    k=0;
				    for(l = 0; l < strlen(color); l++) {
					tmp1 = color[l];
					// Found the substring
					if(tolower(tmp1) == tolower(word[k])) {
				            printf(RED "%c" RESET, color[l]);
					    k++;
					}
					else {
					     printf("%c", color[l]);
					}
				    }
				    printf(" ");
				}
				// Word does not have substring
				else {
				    printf("%s ", color);
				}
			    }
			    printf("\n");
                        }
			// Line does not contain the word according to grep
                        else {
                            printf("  %d: %s", count, line);
                        }
                    }

                // Don't print numbers
                    else {
                        if (count == (linenum = atoi(tree->data))) {
                            if(tree->right != NILNODE) {
                                tree = tree->right;
                            }
                            printf("   ");
                        // Read word by word
                            for(i = 0; sscanf(line+i, "%s", color) == 1; i+=strlen(color)+1 ) {
                                // Word contains substring
                                if( strcasestr(color, word) ) {
                                    k=0;
                                    for(l = 0; l < strlen(color); l++) {
                                        tmp1 = color[l];
                                        // Found the substring
                                        if(tolower(tmp1) == tolower(word[k])) {
                                            printf(RED "%c" RESET, color[l]);
                                            k++;
                                        }
                                        else {
                                             printf("%c", color[l]);
                                        }
                                    }
                                    printf(" ");
                                }
                                // Word does not have substring
                                else {
                                    printf("%s ", color);
                                }
                            }
                            printf("\n");
                        }

                        else {
                            printf("   %s", line);
                        }
                    }
                }
            }

            else {
                fprintf(stderr, "Usage: file does not exist");
                exit(1);
            }
            fclose(fp);

            head = head->next;
        }
    }

    else {
        fprintf(stderr, "mklist: List was not built");
        exit(1);
    }

}
	
int main(int argc, char *argv[]) {
    int i, j, done, colon, slow, fast, linecount, wordcount, banner, linemode, wordmode, nummode;
    char buffer[2305], name[256], num[256], tmp[256], word[256], blah[2305];
    tmp[0] = 0;
    char *str, *strptr;
    // Counting and looping
    done = 0; wordcount = 0; linecount = 0;
    // Keeping track of arguments from argv
    banner = 0; linemode = 0; wordmode = 0; nummode = 0;
    struct node *makehead(), *head, *cur;

    // Check args in argv - SO MANY DUMB CASES
    
    if (argc <= 1) {
	fprintf(stderr, "Usage: You don't have proper amount of arguments\n");
	exit(1);
    }
    if ( ((strcmp(argv[1], "-l")) != 0) && ((strcmp(argv[1], "-w")) != 0) ) {
	fprintf(stderr, "Usage: wrong first arugment\n");
	exit(1);
    }

    for(j=1; j < argc; j++) {
	if((strcmp(argv[j], "-l")) == 0) {
	    linemode++;
	}
	
	else if((strcmp(argv[j], "-w")) == 0) {
            wordmode++;
	    j++;
	    if(argv[j] != NULL) {

	        if( (((strcmp(argv[j], "-l")) != 0) && ((strcmp(argv[j], "-w")) != 0)
                    && ((strcmp(argv[j], "-b")) != 0) && ((strcmp(argv[j], "-n")) != 0))
		    && linemode == 0 ) {
		    str = strncpy(word, argv[j], 256);
	        }

	        else {
		    fprintf(stderr, "Usage: misuse of line and/or wordmode\n");
		    exit(1);
	        }
	    }
	    else {
		fprintf(stderr, "Usage: misuse of line and/or wordmode\n");
                exit(1);
	    }
        }

	else if((strcmp(argv[j], "-b")) == 0) {
            banner++;
        }

	else if((strcmp(argv[j], "-n")) == 0) {
            nummode++;
        }

	else {
	    fprintf(stderr, "Usage: incorrect argument\n");
	    exit(1);
	} 
    }

    if( ((wordmode + linemode + banner + nummode) > 3)  || (wordmode == 1 && linemode == 1) ||
		(wordmode > 1 || linemode > 1 || nummode > 1 || banner > 1) ) {
	fprintf(stderr, "Usage: too many arguments\n");
	exit(1);
    } 
    

    // Making the Linked List
    while (fgets(buffer, 2305, stdin) != NULL) {
	done = 0; i = 0; colon = 0; slow = 0; fast = 0;
	while(done == 0) {
	    fast++;
	    if(buffer[i] == ':') {
		colon++;	
		if(colon == 2) {
		    done++;
		// Getting the file name
		    str = strncpy(name, buffer, slow - 1);
		    name[slow-1] = '\0';
		// Getting the line number
		    str = strncpy(num, buffer+slow, fast - slow - 1);
		    num[fast-slow-1] = '\0';
		// Because no line number can be 0, if atoi is 0 it means no num was collected, thus -n wasn't invoked
		    if( atoi(num) == 0) {
			fprintf(stderr, "Usage: no -n in grep\n");
			exit(1);
		    }
		// Modify the linked list
		    if(tmp[0] == 0) {
			str = strncpy(tmp, name, 256); 
			head = makehead(name, NILNODE, NILNODE, NILNODE);
			cur = head;
			maketree(num, cur, cur->tree, NILNODE, NILNODE);
			linecount++;
		    }
		    else if(strcmp(tmp, name) == 0) {
			maketree(num, NILNODE, cur->tree, NILNODE, NILNODE);
			linecount++;
		    }
		    else {
			str = strncpy(tmp, name, 256);
			cur->next = makehead(name, NILNODE, NILNODE, NILNODE);
			cur = cur->next;
			maketree(num, cur, cur->tree, NILNODE, NILNODE);
			linecount++;
		    }			

		// Number of words
		    if( wordmode == 1) {
		        strptr = strncpy(blah, buffer, 2305); 
		    	while ((strptr = strcasestr(strptr, word)) != NULL) {
			    wordcount++;
			    strptr++;
			}
		    }
		}
		if (colon == 1) {
		    slow = fast;
		}
	    }	
	    i++;
	}
    }

    if( wordmode == 1 && wordcount < linecount ) {
	fprintf(stderr, "Usage: mismatch error, please try again\n");
	exit(1);
    }

    if (banner == 1) {

	if( linemode == 1 ) {
	    printf("THERE ARE %d LINES THAT MATCH\n\n", linecount);
	}
	else {
	    printf("THERE ARE %d MATCHING WORDS\n\n", wordcount);
	}

    }

    else {}

    if (linemode == 1) {
	lineMode(head, nummode);
    }

    else {
	wordMode(head, word, nummode);
    }
    
    exit(0);
}
