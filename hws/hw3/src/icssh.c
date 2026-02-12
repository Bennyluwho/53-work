#include "icssh.h"
#include "helpers.h"
#include "linkedlist.h"
#include <readline/readline.h>

void cleanup(job_info* job, char* curline)
{
	free_job(job);  // Free job struct and associated memory (allocated by validate_input)
	free(curline);  // Free user-entered command string (allocated by readline)

	// validate_input(NULL) frees internal dynamically allocated memory reused call after call to process the user-entered job command repeatedly
	// This call ensures valgrind is happy! 
	validate_input(NULL);  
}

int main(int argc, char* argv[]) {
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;

	// Refers to memory allocated by the readline(). This space is allocated for each user-entered job (the command-line entry). 
	char* curline = NULL;

#ifdef GS  // DO NOT MODIFY. FOR AUTOGRADER
    rl_outstream = fopen("/dev/null", "w");
#endif

	// Setup segmentation fault handler (provided)
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

    // print the prompt & wait for the user to enter commands string
	while ((curline = readline(SHELL_PROMPT)) != NULL) {
        
		// validate_input() parses the user command string in curline into the job struct format. 
		// Dynamically allocates the job struct and memory for line field (copy of curline) 
		// The job struct and the memory referenced by job.line is deallocated by free_job
        // On error, no dynamic memory is allocated and function prints an error message 
		job_info* job = validate_input(curline);
        if (job == NULL) { // Command was empty string or invalid
			free(curline);
			continue;
		}

        //Prints out the job linked list struture for debugging
        #ifdef DEBUG   // If DEBUG flag removed in makefile, this will not longer print
     		debug_print_job(job);
        #endif

		// Example built-in: basic exit (modify for assignment)
		if (strcmp(job->procs->cmd, "exit") == 0) {
			cleanup(job, curline);
            return 0;
		}

		// example of good error handling!
        // create the child proccess
		if ((pid = fork()) < 0) {
			cleanup(job, curline);
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {  //If zero, then it's the child process
            //get the first command in the job list to execute
		    proc_info* proc = job->procs;
			exec_result = execvp(proc->cmd, proc->argv);
			if (exec_result < 0) {  //Error checking
				printf(EXEC_ERR, proc->cmd); 
			    cleanup(job, curline);
				exit(EXIT_FAILURE);
			}
		} else {
        	// As the parent, wait for the foreground job to finish
			wait_result = waitpid(pid, &exit_status, 0);
			if (wait_result < 0) {
			    cleanup(job, curline);
				exit(EXIT_FAILURE);
			}
		}

		free_job(job);  // if a foreground job, we no longer need the data
		free(curline);
	}
    
	validate_input(NULL);

#ifndef GS // DO NOT MODIFY. FOR AUTOGRADER
	fclose(rl_outstream);
#endif

	return 0;
}
