#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "jobs.h"


/* définition du type qui servira de chaînon à la liste chaïnée */
struct job_t {
	int id;
	pid_t pid;
	Etat e;
	char **commande;
	List_job suivant;
};

List_job nouvelle_liste(){
	return NULL;
}

void afficher_job(struct job_t *job) {
	int j;

	if (job->e == ACTIF) {
		printf("[%d] %d ACTIF ", job->id, job->pid);
	} else {
		printf("[%d] %d SUSPENDU ", job->id, job->pid);
	}
	
	j = 0;
	while(job->commande[j] != NULL) {
		printf("%s ", job->commande[j]);
		j++;
	}
	printf("\n");
}


void afficher_jobs(List_job l){
 	while (l!=NULL) {
		afficher_job(l);
		l=l->suivant;
	} 
	/*  l==NULL  */
}

int ajouter_job(pid_t pid, char **cmd, Etat e, List_job *l) {
	List_job nouveau;
	int i;
	nouveau = (List_job) malloc(sizeof(struct job_t));
	if (nouveau == NULL) {
		return 0;
	}
	if (*l == NULL) {
		nouveau->id = 1;
	} else {
		nouveau->id = (*l)->id + 1;
	}

	nouveau->pid = pid;
	nouveau->e = e;
	
	nouveau->commande = (char **) malloc(sizeof(char *));
	if (nouveau->commande == NULL) {
		return 0;
	}

	i = 0;
	while(cmd[i] != NULL) {
		nouveau->commande[i] = (char *) malloc(sizeof(char));
		strcpy(nouveau->commande[i], cmd[i]);
		i++;
	}
	nouveau->commande[i] = NULL;

	nouveau->suivant = *l;
	*l = nouveau;
	return nouveau->id;
}


int retirer_job(pid_t pid, List_job *l){
	List_job l_prec;
	List_job l_cour;
	int id;

	if (*l == NULL){
		return 0;
	}

	if ((*l)->pid == pid){
		id = (*l)->id;
		*l = (*l)->suivant;
		return id;
	}

	l_prec = *l;
	l_cour = (*l)->suivant;
	
	while (l_cour != NULL && l_cour->pid != pid){
		l_prec = l_cour;
		l_cour = l_cour->suivant;
	}
	/* l_cour == NULL ou l_cour->pid == pid */
	if (l_cour == NULL) {
		return 0;
	}
	else {
		l_prec->suivant = l_cour->suivant;
		id = l_cour->id;
		free(l_cour);
		return id;
	}
}

struct job_t *get_job(int id, List_job l){
	List_job l_cour;

	l_cour = l;
	
	while (l_cour != NULL && l_cour->id != id){
		l_cour = l_cour->suivant;
	}
	/* l_cour == NULL ou l_cour->id == id */
	if (l_cour == NULL) {
		return NULL;
	}
	else {
		return l_cour;
	}
}

int changer_etat(pid_t pid, Etat e, List_job l) {
	List_job l_cour;

	l_cour = l;
	
	while (l_cour != NULL && l_cour->pid != pid){
		l_cour = l_cour->suivant;
	}
	/* l_cour == NULL ou l_cour->id == id */
	if (l_cour == NULL) {
		return 0;
	}
	else {
		l_cour->e = e;
		return l_cour->id;
	}
}


