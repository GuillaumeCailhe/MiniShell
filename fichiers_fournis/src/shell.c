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
#include <signal.h> 

#define INPUT 1
#define OUTPUT 0

void handler_child(int sig)
{
    pid_t pid=0;
    while ((pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {}
    return;
}

void handler_z(int sig)
{
	kill(-1, SIGSTOP);
}

void handler_c(int sig)
{	
	kill(-1, SIGINT);
}


int main()
{
	signal(SIGCHLD, handler_child);
	while (1) {
		struct cmdline *l;
		int i;
		int commandsNumber = 1;
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
		
		// Getting the number of commands
		while(l->seq[commandsNumber]!=0){
			commandsNumber++;
		}

		// Initialization of the pipes
		int pipes[commandsNumber][2];

		for(i = 0; i<commandsNumber; i++){ // checking for errors when creating the pipe
			if(pipe(pipes[i]) < -1){ 
				fprintf(stderr, "Error: in pipe number %d creation.\n",i);
				exit(2);
			}
		}

		/* Executing each command */
		for (i=0; i<commandsNumber; i++) {		
			pid = fork();

			if(pid == -1){ // Erreur
				fprintf(stderr, "Error : Fork error \n");
				exit(0);
			}else if(pid == 0){ // The child executes the command
				signal(SIGSTOP, handler_z);
				signal(SIGTERM,handler_c);
				// Piping the result of the command to the next command
				if (i!=commandsNumber-1){
					dup2(pipes[i][INPUT], STDOUT_FILENO);
				}else if (l->out){ // Redirecting outputs >
					int fdOut = open(l->out, O_WRONLY, 0);
					
					if(fdOut == -1){
						fprintf(stderr, "%s: Permission denied\n", l->out);
					}else{
						dup2(fdOut, STDOUT_FILENO);
					}

					//close(fdOut);
				}

				// Reading the pipe to get the result of the previous command
				if(i){
					dup2(pipes[i-1][OUTPUT], STDIN_FILENO);
				}else if (l->in) { // Redirecting  inputs <
					int fdIn = open(l->in, O_RDONLY, 0);
					if(fdIn == -1){
						fprintf(stderr, "%s: Permission denied\n", l->in);
					}
					dup2(fdIn, STDIN_FILENO);
					//close(fdIn);
				}

				// Closing the pipes in the current process to avoid errors
				int j = 0;
				for(j=0; j<commandsNumber; j++){
					close(pipes[j][OUTPUT]);
					close(pipes[j][INPUT]);
				}

				// Executing the current command
				int execError;
				char* command = l->seq[i][0];
				execError = execvp(command, l->seq[i]);
				if(execError == -1){
					fprintf(stderr, "%s: command not found\n", command);
				}

				exit(0);

			}else{
				close(pipes[i][INPUT]); // We won't use the input of the current
				if(l->bg == 0){
					int status;
					waitpid(pid, &status, 0);
				}
			}

			printf("\n");
		}
	}
}
