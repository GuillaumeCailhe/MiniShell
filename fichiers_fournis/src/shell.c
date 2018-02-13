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


int main()
{
	while (1) {
		struct cmdline *l;
		int i, j;
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

		pid = fork();
		if(pid == 0){
			execvp(l->seq[0][0], l->seq[0]);
			exit(0);
		}else{
			int status;
			waitpid(pid, &status, 0);
		}
		

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}
			printf("\n");
		}
	}
}
