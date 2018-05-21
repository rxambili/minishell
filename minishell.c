#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <fcntl.h>
#include "readcmd.h"
#include "jobs.h"

struct job_t {
	int id;
	pid_t pid;
	Etat e;
	char **commande;
	List_job suivant;
};

pid_t pid;
List_job l;


/* Commande interne pour changer de répertoire.
 * Paramètre : la commande
 */
void changeDir(struct cmdline *cmd) {
	int code;
	if(cmd->seq[0][1] == NULL) {
		chdir(getenv("HOME"));
	} else {
		code = chdir(cmd->seq[0][1]);
		if(code == -1) {
			printf(" %s : Ce répertoire n'existe pas\n", cmd->seq[0][1]);
		}
	}
}

void suivi_fils (int sig) {
   	int status;
	int id;
	pid_t pid_fils;
	while ((pid_fils = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
       		if (WIFSTOPPED(status)) {
               		/* traiter la suspension */
			//printf("Le processus %d a été suspendu par le signal %d\n", pid_fils, WSTOPSIG(status));
			id = changer_etat(pid, SUSPENDU, l);
			printf("[%d] %d SUSPENDU\n", id, pid_fils);
            	} else if (WIFCONTINUED(status)) {
                	/* traiter la reprise */
			//printf("Le processus %d a été repris\n", pid_fils);
			id = retirer_job(pid_fils, &l);
			printf("[%d] %d REPRIS\n", id, pid_fils);
            	} else if (WIFEXITED(status)) {
			/* traiter exit */
			id = retirer_job(pid_fils, &l);
			printf("[%d] %d FINI", id, pid_fils);
        	} else if (WIFSIGNALED(status)) {
        		/* traiter signal */
			//printf("Le processus %d a été terminé par le signal %d\n", pid_fils, WTERMSIG(status));
			id = retirer_job(pid_fils, &l);
			printf("[%d] %d TERMINÉ\n", id, pid_fils);
        	}
	}
}

/* Redirige les entrées et sorties.
 */
int redirection(char *in, char *out) {
	int dest;
	if (in != NULL) {
		if ((dest = open(in, O_RDONLY, S_IRUSR | S_IWUSR)) <= -1) {
			perror("open");
			return -1;
		} else {
			if (dup2(dest, 0) < 0) {
				return -1;
			}
			close(dest);
		}
	}
	if (out != NULL) {
		if ((dest = open(out , O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) <= -1) {
			perror("open");
			return -1;
		} else {
			if (dup2(dest, 1) < 0) {
				return -1;
			}
			close(dest);
		}
	}
	return 0;
}

/* Execute une commande en pipeline.
 */
int exec_pipeline(char ***seq) {
	int nbTubes;
	int i;
	int j;
	int ret;
	/* On compte le nombre de tubes.*/
	nbTubes = 0;
	i = 1;
	while (seq[i] != NULL) {
		nbTubes++;
		i++;
	}
					
	int p[nbTubes][2];


	for (i = 0; i < nbTubes; i++) {
		if (pipe(p[i]) < 0) {
			perror("pipe");
			return -1;
		}
	}

	for (i = 0; i < nbTubes + 1; i++) {
		switch (ret = fork()) {
		  case -1:
			perror("fork");
			return -1;
		  case 0:
			if (i < nbTubes) {	
				if (i > 0) { 
					dup2(p[i-1][0], 0);
				}
				dup2(p[i][1], 1);
							
				for (j = 0; j < nbTubes; j++) {
					close(p[j][0]);
					close(p[j][1]);
				}
				execvp(seq[i][0], seq[i]);
				perror("Erreur");
				return -1;
			}
			break;			
		  default:
			if (i == nbTubes) {
				dup2(p[i-1][0], 0);
				for (j = 0; j < nbTubes; j++) {
					close(p[j][0]);
					close(p[j][1]);
				}

				execvp(seq[i][0], seq[i]);
				perror("Erreur");
				return -1;	
			}
			break;
		}		
	}
}

/* Execute les commandes internes.
 * Renvoie 1 si une commande interne a été executé 0 sinon.
 */
int commandeInterne(struct cmdline *cmd, List_job *l) {
	char *ptr;
	int status;
	int id;
	struct job_t *job;
	pid_t pid_fils;



	if(strcmp(cmd->seq[0][0], "cd") == 0) {
		changeDir(cmd);
		return 1;



	} if(strcmp(cmd->seq[0][0], "jobs") == 0) {
		afficher_jobs(*l);
		return 1;



	} if(strcmp(cmd->seq[0][0], "cont") == 0) {
		job = get_job(strtol(cmd->seq[0][1], &ptr, 10), *l);

		if(job != NULL) {
			kill(job->pid, SIGCONT);
		
			pid_fils = waitpid(job->pid, &status, WUNTRACED);
			if(pid_fils == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			if(WIFSTOPPED(status)) {
				//printf("Le processus %d a été suspendu par le signal %d\n", job->pid, WSTOPSIG(status));
				id = ajouter_job(job->pid, job->commande, SUSPENDU, l);
				printf("[%d] %d\n", id, pid_fils);
			} else if(WIFSIGNALED(status)) {
				//printf("Le processus %d a été arreté par le signal %d\n", pid_fils, WTERMSIG(status));
			} else if(!WIFEXITED(status)) {
				printf("Une erreur s'est produite!\n");
			}
			return 1;
		} else {
			return 0;
		}


	} if(strcmp(cmd->seq[0][0], "stop") == 0) {
		job = get_job(strtol(cmd->seq[0][1], &ptr, 10), *l);
		
		if(job != NULL) {
			kill(job->pid, SIGSTOP);
			return 1;
		} else {
			return 0;
		}


	} if(strcmp(cmd->seq[0][0], "exit") == 0) {
		exit(EXIT_SUCCESS);
		return 1;
	}


	return 0;
}

		
	
void handler_INT(int s) {
	kill(pid,SIGKILL);
}

void handler_TSTP(int s) {
	kill(pid,SIGSTOP);
}


int main() { 
	int code;
	int status;
	struct cmdline *cmd;
	pid_t pid_fils;
	int id;

	/* Masques de blocage de signaux */
	sigset_t masque;
	sigset_t anciens;

	signal(SIGINT,handler_INT);
	signal(SIGTSTP,handler_TSTP);
	signal(SIGCHLD,suivi_fils);
	l = nouvelle_liste();

		
	do {
	
		printf("$ ");
		cmd = readcmd();
		
		if(cmd->err != NULL) {
			printf("erreur: %s\n",cmd->err);
		} else {
			code = commandeInterne(cmd, &l);
			if(code == 0) {
				switch(pid = fork()) {
					case 0:
						sigemptyset(&masque);
						sigaddset(&masque,SIGINT);
						sigaddset(&masque,SIGTSTP);
					
						sigprocmask(SIG_SETMASK,&masque,&anciens);

						/* Redirections */
						if (redirection(cmd->in, cmd->out) < 0) {
							exit(2);
						}

						/*Tubes*/
												
						if (exec_pipeline(cmd->seq) < 0) {
							exit(2);
						}						
						exit(1);
						
					case -1:
						perror("fork");
						exit(2);
					default:
						if(cmd->backgrounded == NULL) {
						
							pid_fils = waitpid(pid, &status, WUNTRACED);
							if(pid_fils == -1) {
								perror("waitpid");
								exit(EXIT_FAILURE);
							}
							if(WIFSTOPPED(status)) {
								//printf("Le processus %d a été suspendu par le signal %d\n", pid_fils, WSTOPSIG(status));
								id = ajouter_job(pid, cmd->seq[0], SUSPENDU, &l);
								printf("[%d] %d\n", id, pid_fils);
							} else if(WIFSIGNALED(status)) {
								//printf("Le processus %d a été arreté par le signal %d\n", pid_fils, WTERMSIG(status));
							} else if(!WIFEXITED(status)) {
								printf("Une erreur s'est produite!\n");
							}
						} else {
							id = ajouter_job(pid, cmd->seq[0], ACTIF, &l);
							printf("[%d] %d\n", id, pid);
						}
						break;
				}
			}
		}
		fflush(stdout);
	} while(1);
	return 0;
}
