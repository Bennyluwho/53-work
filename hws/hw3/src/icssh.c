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

volatile sig_atomic_t bg_child_terminated = 0;
volatile sig_atomic_t bg_count = 0;

int main(int argc, char* argv[]) {
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	int last_exit_status = -100;
	list_t *bg_list = NULL;

	bg_list = CreateList(bgentry_compare_seconds, bgentry_printer, bgentry_deleter);
	if (bg_list == NULL) {
		perror("CreateList");
		exit(EXIT_FAILURE);
	}

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

	// Setup SIGCHLD handler (required for background jobs)
	if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
		perror("Failed to set SIGCHLD handler");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
		perror("Failed to set SIGUSR2 handler");
		exit(EXIT_FAILURE);
	}

    // print the prompt & wait for the user to enter commands string
	while ((curline = readline(SHELL_PROMPT)) != NULL) {

		if(bg_child_terminated) {
			reap_bg(bg_list);
			bg_child_terminated = 0;
		}
        
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

		if (job->in_file && job->out_file &&
			strcmp(job->in_file, job->out_file) == 0) {
			fprintf(stderr, RD_ERR);
			cleanup(job, curline);
			continue;
		}

		if (job->out_file && job->procs->err_file &&
			strcmp(job->out_file, job->procs->err_file) == 0) {
			fprintf(stderr, RD_ERR);
			cleanup(job, curline);
			continue;
		}

		proc_info* proc = job->procs;

		BUILTINS:

		if (strcmp(job->procs->cmd, "history") == 0) {
			history_print();
			free_job(job);
			free(curline);
			continue;
		}

		if (proc->cmd[0] == '!') {

			char *target = NULL;

			if (strcmp(proc->cmd, "!") == 0) {
				target = history_get(1);
			} else {
				int n = atoi(proc->cmd + 1);
				target = history_get(n);
			}

			if (target == NULL) {
				free_job(job);
				free(curline);
				continue;
			}

			printf("%s\n", target);

			free_job(job);
			free(curline);

			curline = strdup(target);
			job = validate_input(curline);
			if (job == NULL) {
				free(curline);
				continue;
			}

			proc = job->procs;
			goto BUILTINS;
		}

		history_add(curline);

		// Example built-in: basic exit (modify for assignment)
		if (strcmp(job->procs->cmd, "exit") == 0) {
			node_t *cur = bg_list->head;
			while (cur != NULL) {
				bgentry_t *e = (bgentry_t *)cur->data;
				if (e != NULL) {
					kill(e->pid, SIGTERM); 
				}
				cur = cur->next;
			}

			while (bg_list->length > 0) {
				int status;
				pid_t done = waitpid(-1, &status, 0);
				if (done > 0) remove_and_print_bgpid(bg_list, done);
			}

			history_cleanup();
			DeleteList(bg_list);
			free(bg_list);
			bg_list = NULL;
			cleanup(job, curline);
            return 0;
		}
		
		if (strcmp(job->procs->cmd, "cd") == 0) {
			const char* dir = proc->argv[1];

			if (dir == NULL) {
				dir = getenv("HOME"); 
			}

			if (dir == NULL || chdir(dir) != 0) {
				fprintf(stderr, DIR_ERR);
			} else {
				char cwd[100];
				if (getcwd(cwd, sizeof(cwd)) != NULL) {
					printf("%s\n", cwd);
				} else {
					perror("getcwd");
				}
			}

			free_job(job);
			free(curline);
			continue;
		}

		//returns the last return status (-100 if none)
		if (strcmp(job->procs->cmd, "estatus") == 0) {
			printf("%d\n", last_exit_status);

			free_job(job);
			free(curline);
			continue;
		}
		//returns a list of all background processes
		if (strcmp(job->procs->cmd, "bglist") == 0) {
			node_t *curr = bg_list->head;

			while (curr != NULL) {
				print_bgentry((bgentry_t *)curr->data);
				curr = curr->next;
			}

			free_job(job);
			free(curline);
			continue;
		}

		if (job->nproc > 1) {
			if (job->in_file != NULL || job->out_file != NULL || job->procs->err_file != NULL) {
				fprintf(stderr, RD_ERR);
				cleanup(job, curline);
				continue;
			}
		}

	if (job->nproc > 1) {
    	if (job->nproc == 2) {
				int p[2];
				if (pipe(p) < 0) {
					perror("pipe");
					cleanup(job, curline);
					continue;
				}

				proc_info *p1 = job->procs;
				proc_info *p2 = p1->next_proc;

				pid_t pid1 = fork();
				if (pid1 < 0) {
					perror("fork");
					close(p[0]); close(p[1]);
					cleanup(job, curline);
					continue;
				}
				if (pid1 == 0) {
					dup2(p[1], STDOUT_FILENO);
					close(p[0]);
					close(p[1]);

					execvp(p1->cmd, p1->argv);
					printf(EXEC_ERR, p1->cmd);
					exit(EXIT_FAILURE);
				}

				pid_t pid2 = fork();
				if (pid2 < 0) {
					perror("fork");
					close(p[0]); close(p[1]);
					cleanup(job, curline);
					continue;
				}
				if (pid2 == 0) {
					dup2(p[0], STDIN_FILENO);
					close(p[1]);
					close(p[0]);

					execvp(p2->cmd, p2->argv);
					printf(EXEC_ERR, p2->cmd);
					exit(EXIT_FAILURE);
				}

				close(p[0]);
				close(p[1]);

				if (job->bg) {
					bgentry_t *entry = malloc(sizeof(bgentry_t));
					if (entry == NULL) {
						perror("malloc");
						cleanup(job, curline);
						continue;
					}
					entry->job = job;     
					entry->pid = pid2;    
					entry->seconds = time(NULL);
					InsertInOrder(bg_list, entry);
					bg_count++;

					free(curline);
					continue;
				} else {
					waitpid(pid1, &exit_status, 0);
					waitpid(pid2, &exit_status, 0);

					if (WIFEXITED(exit_status)) last_exit_status = WEXITSTATUS(exit_status);

					cleanup(job, curline);
					continue;
				}

			} else if (job->nproc == 3) {

				int p1[2], p2[2];
				if (pipe(p1) < 0 || pipe(p2) < 0) {
					perror("pipe");
					cleanup(job, curline);
					continue;
				}

				proc_info *a = job->procs;
				proc_info *b = a->next_proc;
				proc_info *c = b->next_proc;

				pid_t pidA = fork();
				if (pidA == 0) {
					dup2(p1[1], STDOUT_FILENO);
					close(p1[0]); close(p1[1]);
					close(p2[0]); close(p2[1]);
					execvp(a->cmd, a->argv);
					printf(EXEC_ERR, a->cmd);
					exit(EXIT_FAILURE);
				}

				pid_t pidB = fork();
				if (pidB == 0) {
					dup2(p1[0], STDIN_FILENO);
					dup2(p2[1], STDOUT_FILENO);
					close(p1[0]); close(p1[1]);
					close(p2[0]); close(p2[1]);
					execvp(b->cmd, b->argv);
					printf(EXEC_ERR, b->cmd);
					exit(EXIT_FAILURE);
				}

				pid_t pidC = fork();
				if (pidC == 0) {
					dup2(p2[0], STDIN_FILENO);
					close(p1[0]); close(p1[1]);
					close(p2[0]); close(p2[1]);
					execvp(c->cmd, c->argv);
					printf(EXEC_ERR, c->cmd);
					exit(EXIT_FAILURE);
				}

				close(p1[0]); close(p1[1]);
				close(p2[0]); close(p2[1]);

				if (job->bg) {
					bgentry_t *entry = malloc(sizeof(bgentry_t));
					if (entry == NULL) {
						perror("malloc");
						cleanup(job, curline);
						continue;
					}
					entry->job = job;
					entry->pid = pidC;
					entry->seconds = time(NULL);
					InsertInOrder(bg_list, entry);
					bg_count++;

					free(curline);
					continue;
				} else {
					waitpid(pidA, NULL, 0);
					waitpid(pidB, NULL, 0);
					waitpid(pidC, &exit_status, 0);

					if (WIFEXITED(exit_status)) last_exit_status = WEXITSTATUS(exit_status);

					cleanup(job, curline);
					continue;
				}

			} else {
				
				fprintf(stderr, RD_ERR);
				cleanup(job, curline);
				continue;
			}
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
			
			// <
			if (job->in_file != NULL) {
			int in_fd = open(job->in_file, O_RDONLY);

				if (in_fd < 0) {
					fprintf(stderr, RD_ERR);
					exit(EXIT_FAILURE);
				}

				if (dup2(in_fd, STDIN_FILENO) < 0) {
					fprintf(stderr, RD_ERR);
					close(in_fd);
					exit(EXIT_FAILURE);
				}

				close(in_fd);
			}

			// >
			if (job->out_file != NULL) {
				int out_fd = open(job->out_file,
								O_WRONLY | O_CREAT | O_TRUNC,
								0644);

				if (out_fd < 0) {
					fprintf(stderr, RD_ERR);
					exit(EXIT_FAILURE);
				}

				if (dup2(out_fd, STDOUT_FILENO) < 0) {
					fprintf(stderr, RD_ERR);
					close(out_fd);
					exit(EXIT_FAILURE);
				}

				close(out_fd);
			}

			// 2>
			if (job->procs->err_file != NULL) {
				int err_fd = open(job->procs->err_file,
								O_WRONLY | O_CREAT | O_TRUNC,
								0644);

				if (err_fd < 0) {
					fprintf(stderr, RD_ERR);
					exit(EXIT_FAILURE);
				}

				if (dup2(err_fd, STDERR_FILENO) < 0) {
					fprintf(stderr, RD_ERR);
					close(err_fd);
					exit(EXIT_FAILURE);
				}

				close(err_fd);
			}

			exec_result = execvp(proc->cmd, proc->argv);
			if (exec_result < 0) {  //Error checking
				printf(EXEC_ERR, proc->cmd); 
			    cleanup(job, curline);
				exit(EXIT_FAILURE);
			}
		} else {
			if (job->bg) {
				//Background job
				//TODO: store job and pid in in background list
				//DO NOT WAIT OT FREE JOB HERE
				bgentry_t *entry = malloc(sizeof(bgentry_t));
				if (entry == NULL) {
					perror("malloc");
					free_job(job);
					free(curline);
					continue;
				}

				entry->job = job;
				entry->pid = pid;
				entry->seconds = time(NULL);

				InsertInOrder(bg_list, entry);
				bg_count++;

				free(curline);
				continue;
			} else {
				//Foreground job
				wait_result = waitpid(pid, &exit_status, 0);
				if (wait_result < 0) {
					cleanup(job, curline);
					exit(EXIT_FAILURE);
				}

				if (WIFEXITED(exit_status)) {
					last_exit_status = WEXITSTATUS(exit_status);
				}
			
				// if a foreground job, we no longer need the data
				free_job(job);
				free(curline);
			}	
		}
	}

	history_cleanup();

	DeleteList(bg_list);
	free(bg_list);
	bg_list = NULL;
	validate_input(NULL);

#ifndef GS // DO NOT MODIFY. FOR AUTOGRADER
	fclose(rl_outstream);
#endif

	return 0;
}
