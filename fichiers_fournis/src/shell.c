/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include  <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h> 


int main()
{
	while (1) {
		struct cmdline *l;
		int i;
		pid_t pid;

		printf("shell> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l || (strcmp(l->seq[0][0],"quit") == 0) ) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}
	

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {		
			pid = fork();
			//printf("pid : %d, itération : %d", getpid(), i);
			if(pid == 0){
				if (l->in) {
					int fdIn = open(l->in, O_RDONLY, 0);
					dup2(fdIn, STDIN_FILENO);
					close(fdIn);
				}
				
				if (l->out){
					int fdOut = open(l->out, O_WRONLY, 0);
					
					if(fdOut == -1){
						fprintf(stderr, "%s: Permission denied\n", l->out);
					}else{
						dup2(fdOut, STDOUT_FILENO);
					}

					close(fdOut);
				}

				int execError;
				char* command = l->seq[i][0];
				execError = execvp(command, l->seq[i]);
				if(execError == -1){
					fprintf(stderr, "%s: command not found\n", command);
				}

				exit(0);
			}else{
				int status;
				waitpid(pid, &status, 0);
			}

			printf("\n");
		}
	}
}
