#include <unistd.h>
#ifndef _JOBS

 #define _JOBS

  typedef enum {ACTIF, SUSPENDU} Etat;
  typedef struct job_t *List_job;
#endif

/* fonction qui renvoie une nouvelle liste de jobs
 * paramètre : aucun
 * résultat : une liste de jobs vide (List_job)
 * pré-condition : aucune
 * post-condition : aucune
 */
List_job nouvelle_liste();


/* procédure qui affiche un job
 * paramètre : le job à afficher (struct job_t *) (D)
 * pré-condition : aucune
 * post-condition : aucune
 */

void afficher_job(struct job_t *job);

/* procédure qui affiche une liste de jobs
 * paramètre : la liste de jobs à afficher (List_job) (D)
 * pré-condition : aucune
 * post-condition : aucune
 */
void afficher_jobs(List_job l);

/* fonction qui rajoute un job à une liste de jobs
 * paramètres : - le numéro du nouveau processus (pid_t) (D)
 *		- la commande lancée (char **) (D)
 *              - l'état du nouveau processus (Etat) (D)
 *              - l'adresse de la liste de jobs (List_job *) (D/R)
 * résultat : l'id du job ajouté si reussi, 0 sinon
 * pré-condition : le processus n'est pas déjà présent (pas de doublon)
 * post-condition : la liste n'est plus vide
 */
int ajouter_job(pid_t pid, char **cmd, Etat e, List_job *l);

/* fonction qui supprime un certain processus d'une liste de jobs
 * paramètres : - le numéro du nouveau processus à supprimer (pid_t) (D)
 *              - l'adresse de la liste de jobs (List_job *) (D/R)
 * résultat : l'id du job si reussie, retourne 0 si aucun 
 * 	      processus n'est associé ou liste vide (struct job_t *)
 * pré-condition : aucune
 * post-condition : la liste ne contient plus de processus avec ce numéro
 */
int retirer_job(pid_t pid, List_job *l);

/* fonction qui retourne le job associé à son id dans la liste
 * paramètres : - le numéro du processus dans la liste (int) (D)
 * 		- l'adresse de la liste de jobs (List_job *) (D)
 * résultat : l'adresse du processus associé, retourne NULL si aucun 
 * 	      processus n'est associé ou liste vide (struct job_t *)
 * pré-condition : aucune
 * post-condition : aucune
 */
struct job_t *get_job(int id, List_job l);

/* fonction qui change l'état du job associé à son pid
 * paramètres : - le numéro du processus (pid_t) (D)
 * 		- le nouvel état (Etat) (D)
 * 		- l'adresse de la liste de jobs (List_job ) (D)
 * résultat : l'id du processeur associé, retourne 0 si aucun 
 * 	      processus n'est associé ou liste vide (struct job_t *)
 * pré-condition : aucune
 * post-condition : aucune
 */
int changer_etat(pid_t pid, Etat e, List_job l);
