/*
* ============================================================
* APPLICATION DE GESTION DE FOYER UNIVERSITAIRE
* Projet ASD2 - 1LIG - Annee academique 2025-2026
* Version : Listes Chainees (Chap.5) + Fichiers Textes (Chap.4)
* ============================================================
*
* STRUCTURES :
* - Chaque module utilise une liste chainee simple (struct noeud)
* - Les donnees sont persistantes via des fichiers textes (.txt)
* - Insertion en fin de liste, suppression selon ID, parcours complet
*
* MODULES :
* 1. Gestion des Etudiants (residents)
* 2. Gestion des Chambres
* 3. Gestion des Paiements
* 4. Gestion des Reclamations
* ============================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
* CONSTANTES
* ============================================================ */
/* Noms des fichiers textes de persistance (Chapitre 4) */
#define FICHIER_ADMIN "admin.txt"
#define FICHIER_ETUDIANTS "etudiants.txt"
#define FICHIER_CHAMBRES "chambres.txt"
#define FICHIER_PAIEMENTS "paiements.txt"
#define FICHIER_RECLAMATIONS "reclamations.txt"

/* Identifiants admin en memoire (charges depuis admin.txt au demarrage) */
char g_login_admin[30] ;
char g_mdp_admin[30] ;

/* ============================================================
* STRUCTURES DE DONNEES (les champs de chaque entite)
* ============================================================ */

typedef struct {
int id;
char nom[50];
char prenom[50];
char cin[15];
char date_naissance[15];
char email[60];
char telephone[15];
int chambre_id; /* 0 = pas de chambre assignee */
char statut[20]; /* Actif / Inactif */
} Etudiant;

typedef struct {
int id;
char numero[10]; /* ex: A101, B205 */
char type[20]; /* Simple / Double / Triple */
int etage;
float prix_mensuel;
int capacite;
int nb_occupants;
char statut[20]; /* Disponible / Occupee / Maintenance */
} Chambre;

typedef struct {
int id;
int etudiant_id;
int chambre_id;
float montant;
char date_paiement[15];
char mois[20];
char mode[20]; /* Especes / Virement / Cheque */
char statut[20]; /* Paye / En_attente / Retard */
} Paiement;

typedef struct {
int id;
int etudiant_id;
int chambre_id;
char date[15];
char type[40];
char description[200];
char priorite[15]; /* Haute / Moyenne / Basse */
char statut[20]; /* Ouverte / En_cours / Resolue / Fermee */
} Reclamation;

/* ============================================================
* NOEUDS DES LISTES CHAINEES (Chapitre 5)
*
* Principe : chaque noeud contient les donnees + un pointeur
* vers le noeud suivant (suivant = NULL en queue)
*
* struct noeud {
* type_donnees data;
* struct noeud *suivant;
* };
* ============================================================ */

struct NoeudEtudiant {
Etudiant data;
struct NoeudEtudiant *suivant;
};
typedef struct NoeudEtudiant NoeudEtudiant;

struct NoeudChambre {
Chambre data;
struct NoeudChambre *suivant;
};
typedef struct NoeudChambre NoeudChambre;

struct NoeudPaiement {
Paiement data;
struct NoeudPaiement *suivant;
};
typedef struct NoeudPaiement NoeudPaiement;

struct NoeudReclamation {
Reclamation data;
struct NoeudReclamation *suivant;
};
typedef struct NoeudReclamation NoeudReclamation;

/* ============================================================
* TETES DE LISTES CHAINEES (pointeurs globaux)
* Si tete == NULL => liste vide (Chapitre 5)
* ============================================================ */
NoeudEtudiant *liste_etudiants = NULL;
NoeudChambre *liste_chambres = NULL;
NoeudPaiement *liste_paiements = NULL;
NoeudReclamation *liste_reclamations = NULL;

/* Compteurs et generateurs d'ID */
int nb_etudiants = 0, nb_chambres = 0;
int nb_paiements = 0, nb_reclamations = 0;
int id_etudiant_next = 1, id_chambre_next = 1;
int id_paiement_next = 1, id_reclamation_next = 1;

/* ============================================================
* UTILITAIRES
* ============================================================ */
void afficher_separateur() { printf("================================================================\n"); }
void afficher_ligne() { printf("----------------------------------------------------------------\n"); }
void vider_buffer() { int c; while ((c = getchar()) != '\n' && c != EOF); }
void pause_attente() { printf("\n Appuyez sur Entree pour continuer..."); vider_buffer(); }
void strip_nl(char *s) { s[strcspn(s, "\n")] = '\0'; } /* supprime le '\n' de fgets */

void afficher_entete(const char *titre) {
printf("\n");
afficher_separateur();
printf(" %s\n", titre);
afficher_separateur();
}

/* ============================================================
* AUTHENTIFICATION
* ============================================================ */
/* ============================================================
* GESTION DES IDENTIFIANTS ADMIN (fichier admin.txt)
*
* Format du fichier :
* login|mdp
* Exemple :
* admin|1234
* ============================================================ */

void sauvegarder_admin() {
FILE *f = fopen(FICHIER_ADMIN, "w");
if (f == NULL) {
printf(" [ERREUR] Impossible de sauvegarder %s.\n", FICHIER_ADMIN);
return;
}
fprintf(f, "%s|%s\n", g_login_admin, g_mdp_admin);
fclose(f);
}

void charger_admin() {
FILE *f = fopen(FICHIER_ADMIN, "r");
if (f == NULL) {
/* Premiere utilisation : creer le fichier avec les valeurs par defaut */
sauvegarder_admin();
return;
}
char ligne[80];
if (fgets(ligne, sizeof(ligne), f)) {
strip_nl(ligne);
char *pos = ligne;
char *pipe = strchr(pos, '|');
if (pipe != NULL) {
*pipe = '\0';
strncpy(g_login_admin, pos, 29); g_login_admin[29] = '\0';
strncpy(g_mdp_admin, pipe+1, 29); g_mdp_admin[29] = '\0';
}
}
fclose(f);
}

/* Modifier le login et/ou le mot de passe depuis le menu */
void modifier_identifiants_admin() {
afficher_entete("MODIFIER LES IDENTIFIANTS ADMINISTRATEUR");

/* 1 — Verification de l'ancien mot de passe avant toute modif */
char ancien_mdp[30];
printf(" Confirmez votre mot de passe actuel : ");
scanf("%29s", ancien_mdp); vider_buffer();
if (strcmp(ancien_mdp, g_mdp_admin) != 0) {
printf("\n [ERREUR] Mot de passe incorrect. Modification annulee.\n");
pause_attente();
return;
}

/* 2 — Nouveau login */
char nouveau_login[30];
printf("\n Login actuel : %s\n", g_login_admin);
printf(" Nouveau login : "); scanf("%29s", nouveau_login); vider_buffer();
if (strlen(nouveau_login) == 0) {
printf(" [ERREUR] Login vide. Modification annulee.\n");
pause_attente();
return;
}

/* 3 — Nouveau mot de passe (double saisie) */
char nouveau_mdp[30], confirm_mdp[30];
printf(" Nouveau mot de passe : "); scanf("%29s", nouveau_mdp); vider_buffer();
printf(" Confirmer mot de passe : "); scanf("%29s", confirm_mdp); vider_buffer();
if (strcmp(nouveau_mdp, confirm_mdp) != 0) {
printf("\n [ERREUR] Les deux mots de passe ne correspondent pas. Annule.\n");
pause_attente();
return;
}
if (strlen(nouveau_mdp) == 0) {
printf(" [ERREUR] Mot de passe vide. Modification annulee.\n");
pause_attente();
return;
}

/* 4 — Appliquer et sauvegarder */
strncpy(g_login_admin, nouveau_login, 29); g_login_admin[29] = '\0';
strncpy(g_mdp_admin, nouveau_mdp, 29); g_mdp_admin[29] = '\0';
sauvegarder_admin();

printf("\n [OK] Identifiants mis a jour et sauvegardes dans %s\n", FICHIER_ADMIN);
printf(" Nouveau login : %s\n", g_login_admin);
printf(" Nouveau mdp : %s\n", g_mdp_admin);
pause_attente();
}

/* ============================================================
* AUTHENTIFICATION
* ============================================================ */
int authentifier() {
char login[30], mdp[30];
int tentatives = 3;
afficher_entete("CONNEXION - GESTION DE FOYER UNIVERSITAIRE");
printf(" Systeme de gestion des residents\n");
afficher_ligne();
while (tentatives > 0) {
printf("\n Login : "); scanf("%29s", login); vider_buffer();
printf(" Mot de passe : "); scanf("%29s", mdp); vider_buffer();
if (strcmp(login, g_login_admin) == 0 && strcmp(mdp, g_mdp_admin) == 0) {
printf("\n Connexion reussie ! Bienvenue, %s.\n", login);
pause_attente();
return 1;
}
tentatives--;
if (tentatives > 0)
printf("\n [ERREUR] Identifiants incorrects. Tentatives restantes : %d\n", tentatives);
else
printf("\n [ERREUR] Acces refuse apres 3 tentatives.\n");
}
return 0;
}

/* ============================================================
* OPERATIONS SUR LES LISTES CHAINEES - ETUDIANTS
* (Chapitre 5 : creation noeud, insertion fin, longueur,
* recherche, suppression selon contenu, destruction)
* ============================================================ */

/* Creer et allouer un nouveau noeud etudiant */
NoeudEtudiant* creer_noeud_etudiant(Etudiant e) {
NoeudEtudiant *nouveau = (NoeudEtudiant*)malloc(sizeof(NoeudEtudiant));
nouveau->data = e;
nouveau->suivant = NULL;
return nouveau;
}

/* Insertion a la fin de la liste (insertion_fin du cours) */
void inserer_etudiant(Etudiant e) {
NoeudEtudiant *nouveau = creer_noeud_etudiant(e);
if (liste_etudiants == NULL) {
/* Liste vide : le nouveau noeud devient la tete */
liste_etudiants = nouveau;
} else {
/* Parcourir jusqu'au dernier noeud */
NoeudEtudiant *courant = liste_etudiants;
while (courant->suivant != NULL)
courant = courant->suivant;
courant->suivant = nouveau; /* lier au dernier noeud */
}
nb_etudiants++;
}

/* Longueur de la liste etudiants (exercice du cours Chap.5) */
int longueur_etudiants() {
int lg = 0;
NoeudEtudiant *courant = liste_etudiants;
while (courant != NULL) { lg++; courant = courant->suivant; }
return lg;
}

/* Recherche par ID : renvoie le pointeur sur le noeud (NULL si absent) */
NoeudEtudiant* trouver_etudiant(int id) {
NoeudEtudiant *courant = liste_etudiants;
while (courant != NULL) {
if (courant->data.id == id) return courant;
courant = courant->suivant;
}
return NULL;
}

/* Suppression selon le contenu (suppression par ID) */
void supprimer_noeud_etudiant(int id) {
if (liste_etudiants == NULL) return;
NoeudEtudiant *courant = liste_etudiants, *precedent = NULL;
/* Recherche du noeud a supprimer */
while (courant != NULL && courant->data.id != id) {
precedent = courant;
courant = courant->suivant;
}
if (courant == NULL) { printf(" [ERREUR] Etudiant introuvable.\n"); return; }
/* Reconnexion et liberation */
if (precedent == NULL)
liste_etudiants = courant->suivant; /* suppression en tete */
else
precedent->suivant = courant->suivant; /* suppression au milieu/fin */
free(courant);
nb_etudiants--;
}

/* Destruction complete de la liste (liberer toute la memoire) */
void detruire_liste_etudiants() {
NoeudEtudiant *courant = liste_etudiants;
while (courant != NULL) {
NoeudEtudiant *suiv = courant->suivant;
free(courant);
courant = suiv;
}
liste_etudiants = NULL;
nb_etudiants = 0;
}

/* ============================================================
* OPERATIONS SUR LES LISTES CHAINEES - CHAMBRES
* ============================================================ */

NoeudChambre* creer_noeud_chambre(Chambre c) {
NoeudChambre *nouveau = (NoeudChambre*)malloc(sizeof(NoeudChambre));
nouveau->data = c; nouveau->suivant = NULL;
return nouveau;
}

void inserer_chambre(Chambre c) {
NoeudChambre *nouveau = creer_noeud_chambre(c);
if (liste_chambres == NULL) {
liste_chambres = nouveau;
} else {
NoeudChambre *courant = liste_chambres;
while (courant->suivant != NULL) courant = courant->suivant;
courant->suivant = nouveau;
}
nb_chambres++;
}

int longueur_chambres() {
int lg = 0;
NoeudChambre *courant = liste_chambres;
while (courant != NULL) { lg++; courant = courant->suivant; }
return lg;
}

NoeudChambre* trouver_chambre(int id) {
NoeudChambre *courant = liste_chambres;
while (courant != NULL) {
if (courant->data.id == id) return courant;
courant = courant->suivant;
}
return NULL;
}

void supprimer_noeud_chambre(int id) {
if (liste_chambres == NULL) return;
NoeudChambre *courant = liste_chambres, *precedent = NULL;
while (courant != NULL && courant->data.id != id) {
precedent = courant; courant = courant->suivant;
}
if (courant == NULL) { printf(" [ERREUR] Chambre introuvable.\n"); return; }
if (precedent == NULL) liste_chambres = courant->suivant;
else precedent->suivant = courant->suivant;
free(courant);
nb_chambres--;
}

void detruire_liste_chambres() {
NoeudChambre *courant = liste_chambres;
while (courant != NULL) {
NoeudChambre *suiv = courant->suivant;
free(courant); courant = suiv;
}
liste_chambres = NULL; nb_chambres = 0;
}

/* ============================================================
* OPERATIONS SUR LES LISTES CHAINEES - PAIEMENTS
* ============================================================ */

NoeudPaiement* creer_noeud_paiement(Paiement p) {
NoeudPaiement *nouveau = (NoeudPaiement*)malloc(sizeof(NoeudPaiement));
nouveau->data = p; nouveau->suivant = NULL;
return nouveau;
}

void inserer_paiement(Paiement p) {
NoeudPaiement *nouveau = creer_noeud_paiement(p);
if (liste_paiements == NULL) {
liste_paiements = nouveau;
} else {
NoeudPaiement *courant = liste_paiements;
while (courant->suivant != NULL) courant = courant->suivant;
courant->suivant = nouveau;
}
nb_paiements++;
}

int longueur_paiements() {
int lg = 0;
NoeudPaiement *courant = liste_paiements;
while (courant != NULL) { lg++; courant = courant->suivant; }
return lg;
}

NoeudPaiement* trouver_paiement(int id) {
NoeudPaiement *courant = liste_paiements;
while (courant != NULL) {
if (courant->data.id == id) return courant;
courant = courant->suivant;
}
return NULL;
}

void supprimer_noeud_paiement(int id) {
if (liste_paiements == NULL) return;
NoeudPaiement *courant = liste_paiements, *precedent = NULL;
while (courant != NULL && courant->data.id != id) {
precedent = courant; courant = courant->suivant;
}
if (courant == NULL) { printf(" [ERREUR] Paiement introuvable.\n"); return; }
if (precedent == NULL) liste_paiements = courant->suivant;
else precedent->suivant = courant->suivant;
free(courant);
nb_paiements--;
}

void detruire_liste_paiements() {
NoeudPaiement *courant = liste_paiements;
while (courant != NULL) {
NoeudPaiement *suiv = courant->suivant;
free(courant); courant = suiv;
}
liste_paiements = NULL; nb_paiements = 0;
}

/* ============================================================
* OPERATIONS SUR LES LISTES CHAINEES - RECLAMATIONS
* ============================================================ */

NoeudReclamation* creer_noeud_reclamation(Reclamation r) {
NoeudReclamation *nouveau = (NoeudReclamation*)malloc(sizeof(NoeudReclamation));
nouveau->data = r; nouveau->suivant = NULL;
return nouveau;
}

void inserer_reclamation(Reclamation r) {
NoeudReclamation *nouveau = creer_noeud_reclamation(r);
if (liste_reclamations == NULL) {
liste_reclamations = nouveau;
} else {
NoeudReclamation *courant = liste_reclamations;
while (courant->suivant != NULL) courant = courant->suivant;
courant->suivant = nouveau;
}
nb_reclamations++;
}

int longueur_reclamations() {
int lg = 0;
NoeudReclamation *courant = liste_reclamations;
while (courant != NULL) { lg++; courant = courant->suivant; }
return lg;
}

NoeudReclamation* trouver_reclamation(int id) {
NoeudReclamation *courant = liste_reclamations;
while (courant != NULL) {
if (courant->data.id == id) return courant;
courant = courant->suivant;
}
return NULL;
}

void supprimer_noeud_reclamation(int id) {
if (liste_reclamations == NULL) return;
NoeudReclamation *courant = liste_reclamations, *precedent = NULL;
while (courant != NULL && courant->data.id != id) {
precedent = courant; courant = courant->suivant;
}
if (courant == NULL) { printf(" [ERREUR] Reclamation introuvable.\n"); return; }
if (precedent == NULL) liste_reclamations = courant->suivant;
else precedent->suivant = courant->suivant;
free(courant);
nb_reclamations--;
}

void detruire_liste_reclamations() {
NoeudReclamation *courant = liste_reclamations;
while (courant != NULL) {
NoeudReclamation *suiv = courant->suivant;
free(courant); courant = suiv;
}
liste_reclamations = NULL; nb_reclamations = 0;
}

/* ============================================================
* GESTION DES FICHIERS TEXTES (Chapitre 4)
*
* Format TABLEAU avec separateur '|' (style CSV)
* -----------------------------------------------
* Ligne 1 : nombre d'enregistrements
* Ligne 2 : en-tete des colonnes (noms des champs)
* Lignes 3+: un enregistrement par ligne, champs separes par '|'
*
* Exemple etudiants.txt :
* 3
* id|nom|prenom|cin|date_naissance|email|telephone|chambre_id|statut
* 1|Mansouri|Amine|12345678|10/05/2004|amine@email.tn|55001122|1|Actif
* 2|Gharbi|Nour|87654321|22/09/2005|nour@email.tn|55334455|2|Actif
* 3|Bouazizi|Sami|11223344|03/03/2004|sami@email.tn|50667788|3|Actif
*
* Ecriture : fprintf avec '|' entre chaque champ
* Lecture : fgets (ligne complete) + strtok(ligne, "|")
* ============================================================ */

/* Separateur de colonnes utilise dans tous les fichiers */
#define SEP "|"

/*
* extraire_champ(tok, dest, taille)
* ----------------------------------
* Copie le token courant (issu de strtok) dans dest apres avoir
* supprime le '\n' eventuel. Avance strtok au champ suivant.
* Retourne 0 si le token est NULL (ligne malformee).
*/
static int extraire_champ(char *tok, char *dest, int taille) {
if (tok == NULL) return 0;
strncpy(dest, tok, taille - 1);
dest[taille - 1] = '\0';
strip_nl(dest);
return 1;
}

/* ---------- SAUVEGARDE (ecriture dans fichier texte) ---------- */

void sauvegarder_etudiants() {
FILE *f = fopen(FICHIER_ETUDIANTS, "w"); /* ouverture en ecriture */
if (f == NULL) {
printf(" [ERREUR] Impossible d'ouvrir %s en ecriture.\n", FICHIER_ETUDIANTS);
return;
}

/* Ligne 1 : nombre d'enregistrements */
fprintf(f, "%d\n", nb_etudiants);

/* Ligne 2 : en-tete du tableau */
fprintf(f, "id|nom|prenom|cin|date_naissance|email|telephone|chambre_id|statut\n");

/* Lignes 3+ : un etudiant par ligne, champs separes par '|' */
NoeudEtudiant *courant = liste_etudiants;
while (courant != NULL) {
Etudiant *e = &courant->data;
fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%d|%s\n",
e->id, e->nom, e->prenom, e->cin, e->date_naissance,
e->email, e->telephone, e->chambre_id, e->statut);
courant = courant->suivant;
}
fclose(f); /* fermeture obligatoire */
}

void sauvegarder_chambres() {
FILE *f = fopen(FICHIER_CHAMBRES, "w");
if (f == NULL) {
printf(" [ERREUR] Impossible d'ouvrir %s en ecriture.\n", FICHIER_CHAMBRES);
return;
}

fprintf(f, "%d\n", nb_chambres);
fprintf(f, "id|numero|type|etage|prix_mensuel|capacite|nb_occupants|statut\n");

NoeudChambre *courant = liste_chambres;
while (courant != NULL) {
Chambre *c = &courant->data;
fprintf(f, "%d|%s|%s|%d|%.2f|%d|%d|%s\n",
c->id, c->numero, c->type, c->etage,
c->prix_mensuel, c->capacite, c->nb_occupants, c->statut);
courant = courant->suivant;
}
fclose(f);
}

void sauvegarder_paiements() {
FILE *f = fopen(FICHIER_PAIEMENTS, "w");
if (f == NULL) {
printf(" [ERREUR] Impossible d'ouvrir %s en ecriture.\n", FICHIER_PAIEMENTS);
return;
}

fprintf(f, "%d\n", nb_paiements);
fprintf(f, "id|etudiant_id|chambre_id|montant|date_paiement|mois|mode|statut\n");

NoeudPaiement *courant = liste_paiements;
while (courant != NULL) {
Paiement *p = &courant->data;
fprintf(f, "%d|%d|%d|%.2f|%s|%s|%s|%s\n",
p->id, p->etudiant_id, p->chambre_id, p->montant,
p->date_paiement, p->mois, p->mode, p->statut);
courant = courant->suivant;
}
fclose(f);
}

void sauvegarder_reclamations() {
FILE *f = fopen(FICHIER_RECLAMATIONS, "w");
if (f == NULL) {
printf(" [ERREUR] Impossible d'ouvrir %s en ecriture.\n", FICHIER_RECLAMATIONS);
return;
}

fprintf(f, "%d\n", nb_reclamations);
fprintf(f, "id|etudiant_id|chambre_id|date|type|description|priorite|statut\n");

NoeudReclamation *courant = liste_reclamations;
while (courant != NULL) {
Reclamation *r = &courant->data;
/*
* La description peut contenir des espaces mais pas de '|'.
* Si un '|' y est present par erreur, on le remplace par '-'
* pour preserver l'integrite du tableau.
*/
int k;
char desc_safe[200];
strncpy(desc_safe, r->description, 199);
desc_safe[199] = '\0';
for ( k = 0; desc_safe[k]; k++)
if (desc_safe[k] == '|') desc_safe[k] = '-';

fprintf(f, "%d|%d|%d|%s|%s|%s|%s|%s\n",
r->id, r->etudiant_id, r->chambre_id,
r->date, r->type, desc_safe, r->priorite, r->statut);
courant = courant->suivant;
}
fclose(f);
}

/* ---------- CHARGEMENT (lecture depuis fichier texte) ---------- */

void charger_etudiants() {
FILE *f = fopen(FICHIER_ETUDIANTS, "r"); /* ouverture en lecture */
if (f == NULL) return; /* fichier absent : rien a faire */

char ligne[300];

/* Ligne 1 : nombre d'enregistrements */
int n = 0;
int i;
if (fgets(ligne, sizeof(ligne), f)) n = atoi(ligne);

/* Ligne 2 : en-tete — on la saute */
fgets(ligne, sizeof(ligne), f);

/* Lignes 3+ : lire chaque enregistrement */
for ( i = 0; i < n; i++) {
if (!fgets(ligne, sizeof(ligne), f)) break;
strip_nl(ligne);

Etudiant e;
char *tok;
tok = strtok(ligne, SEP); if (tok) e.id = atoi(tok);
tok = strtok(NULL, SEP); extraire_champ(tok, e.nom, 50);
tok = strtok(NULL, SEP); extraire_champ(tok, e.prenom, 50);
tok = strtok(NULL, SEP); extraire_champ(tok, e.cin, 15);
tok = strtok(NULL, SEP); extraire_champ(tok, e.date_naissance, 15);
tok = strtok(NULL, SEP); extraire_champ(tok, e.email, 60);
tok = strtok(NULL, SEP); extraire_champ(tok, e.telephone, 15);
tok = strtok(NULL, SEP); if (tok) e.chambre_id = atoi(tok);
tok = strtok(NULL, SEP); extraire_champ(tok, e.statut, 20);

inserer_etudiant(e);
if (e.id >= id_etudiant_next) id_etudiant_next = e.id + 1;
}
fclose(f);
}

void charger_chambres() {
FILE *f = fopen(FICHIER_CHAMBRES, "r");
if (f == NULL) return;
int i;
char ligne[200];
int n = 0;
if (fgets(ligne, sizeof(ligne), f)) n = atoi(ligne);
fgets(ligne, sizeof(ligne), f); /* sauter l'en-tete */

for ( i = 0; i < n; i++) {
if (!fgets(ligne, sizeof(ligne), f)) break;
strip_nl(ligne);

Chambre c;
char *tok;
tok = strtok(ligne, SEP); if (tok) c.id = atoi(tok);
tok = strtok(NULL, SEP); extraire_champ(tok, c.numero, 10);
tok = strtok(NULL, SEP); extraire_champ(tok, c.type, 20);
tok = strtok(NULL, SEP); if (tok) c.etage = atoi(tok);
tok = strtok(NULL, SEP); if (tok) c.prix_mensuel = (float)atof(tok);
tok = strtok(NULL, SEP); if (tok) c.capacite = atoi(tok);
tok = strtok(NULL, SEP); if (tok) c.nb_occupants = atoi(tok);
tok = strtok(NULL, SEP); extraire_champ(tok, c.statut, 20);

inserer_chambre(c);
if (c.id >= id_chambre_next) id_chambre_next = c.id + 1;
}
fclose(f);
}

void charger_paiements() {
FILE *f = fopen(FICHIER_PAIEMENTS, "r");
if (f == NULL) return;
int i;
char ligne[200];
int n = 0;
if (fgets(ligne, sizeof(ligne), f)) n = atoi(ligne);
fgets(ligne, sizeof(ligne), f); /* sauter l'en-tete */

for ( i = 0; i < n; i++) {
if (!fgets(ligne, sizeof(ligne), f)) break;
strip_nl(ligne);

Paiement p;
char *tok;
tok = strtok(ligne, SEP); if (tok) p.id = atoi(tok);
tok = strtok(NULL, SEP); if (tok) p.etudiant_id = atoi(tok);
tok = strtok(NULL, SEP); if (tok) p.chambre_id = atoi(tok);
tok = strtok(NULL, SEP); if (tok) p.montant = (float)atof(tok);
tok = strtok(NULL, SEP); extraire_champ(tok, p.date_paiement, 15);
tok = strtok(NULL, SEP); extraire_champ(tok, p.mois, 20);
tok = strtok(NULL, SEP); extraire_champ(tok, p.mode, 20);
tok = strtok(NULL, SEP); extraire_champ(tok, p.statut, 20);

inserer_paiement(p);
if (p.id >= id_paiement_next) id_paiement_next = p.id + 1;
}
fclose(f);
}

void charger_reclamations() {
FILE *f = fopen(FICHIER_RECLAMATIONS, "r");
if (f == NULL) return;
int i;
char ligne[400];
int n = 0;
if (fgets(ligne, sizeof(ligne), f)) n = atoi(ligne);
fgets(ligne, sizeof(ligne), f); /* sauter l'en-tete */

for ( i = 0; i < n; i++) {
if (!fgets(ligne, sizeof(ligne), f)) break;
strip_nl(ligne);

Reclamation r;
char *tok;
tok = strtok(ligne, SEP); if (tok) r.id = atoi(tok);
tok = strtok(NULL, SEP); if (tok) r.etudiant_id = atoi(tok);
tok = strtok(NULL, SEP); if (tok) r.chambre_id = atoi(tok);
tok = strtok(NULL, SEP); extraire_champ(tok, r.date, 15);
tok = strtok(NULL, SEP); extraire_champ(tok, r.type, 40);
tok = strtok(NULL, SEP); extraire_champ(tok, r.description, 200);
tok = strtok(NULL, SEP); extraire_champ(tok, r.priorite, 15);
tok = strtok(NULL, SEP); extraire_champ(tok, r.statut, 20);
inserer_reclamation(r);
if (r.id >= id_reclamation_next) id_reclamation_next = r.id + 1;
}
fclose(f);
}

/* ============================================================
* DONNEES DE DEMONSTRATION (utilisees si fichiers absents)
* ============================================================ */
void charger_donnees_demo() {
/* -- Chambres -- */
Chambre ch;
ch.id=1; strcpy(ch.numero,"A101"); strcpy(ch.type,"Simple");
ch.etage=1; ch.capacite=1; ch.nb_occupants=1;
ch.prix_mensuel=250.0f; strcpy(ch.statut,"Occupee");
inserer_chambre(ch);

ch.id=2; strcpy(ch.numero,"A102"); strcpy(ch.type,"Double");
ch.etage=1; ch.capacite=2; ch.nb_occupants=1;
ch.prix_mensuel=180.0f; strcpy(ch.statut,"Disponible");
inserer_chambre(ch);

ch.id=3; strcpy(ch.numero,"B201"); strcpy(ch.type,"Triple");
ch.etage=2; ch.capacite=3; ch.nb_occupants=2;
ch.prix_mensuel=140.0f; strcpy(ch.statut,"Disponible");
inserer_chambre(ch);

ch.id=4; strcpy(ch.numero,"B202"); strcpy(ch.type,"Simple");
ch.etage=2; ch.capacite=1; ch.nb_occupants=0;
ch.prix_mensuel=250.0f; strcpy(ch.statut,"Maintenance");
inserer_chambre(ch);
id_chambre_next = 5;

/* -- Etudiants -- */
Etudiant e;
e.id=1; strcpy(e.nom,"Mansouri"); strcpy(e.prenom,"Amine");
strcpy(e.cin,"12345678"); strcpy(e.date_naissance,"10/05/2004");
strcpy(e.email,"amine.mansouri@email.tn"); strcpy(e.telephone,"55001122");
e.chambre_id=1; strcpy(e.statut,"Actif");
inserer_etudiant(e);

e.id=2; strcpy(e.nom,"Gharbi"); strcpy(e.prenom,"Nour");
strcpy(e.cin,"87654321"); strcpy(e.date_naissance,"22/09/2005");
strcpy(e.email,"nour.gharbi@email.tn"); strcpy(e.telephone,"55334455");
e.chambre_id=2; strcpy(e.statut,"Actif");
inserer_etudiant(e);

e.id=3; strcpy(e.nom,"Bouazizi"); strcpy(e.prenom,"Sami");
strcpy(e.cin,"11223344"); strcpy(e.date_naissance,"03/03/2004");
strcpy(e.email,"sami.bouazizi@email.tn"); strcpy(e.telephone,"50667788");
e.chambre_id=3; strcpy(e.statut,"Actif");
inserer_etudiant(e);
id_etudiant_next = 4;

/* -- Paiements -- */
Paiement p;
p.id=1; p.etudiant_id=1; p.chambre_id=1; p.montant=250.0f;
strcpy(p.date_paiement,"01/04/2026"); strcpy(p.mois,"Avril");
strcpy(p.mode,"Virement"); strcpy(p.statut,"Paye");
inserer_paiement(p);

p.id=2; p.etudiant_id=2; p.chambre_id=2; p.montant=180.0f;
strcpy(p.date_paiement,"05/04/2026"); strcpy(p.mois,"Avril");
strcpy(p.mode,"Especes"); strcpy(p.statut,"En_attente");
inserer_paiement(p);

p.id=3; p.etudiant_id=3; p.chambre_id=3; p.montant=140.0f;
strcpy(p.date_paiement,"10/03/2026"); strcpy(p.mois,"Mars");
strcpy(p.mode,"Cheque"); strcpy(p.statut,"Retard");
inserer_paiement(p);
id_paiement_next = 4;

/* -- Reclamations -- */
Reclamation r;
r.id=1; r.etudiant_id=1; r.chambre_id=1;
strcpy(r.date,"02/04/2026"); strcpy(r.type,"Plomberie");
strcpy(r.description,"Fuite d'eau sous le lavabo");
strcpy(r.priorite,"Haute"); strcpy(r.statut,"En_cours");
inserer_reclamation(r);

r.id=2; r.etudiant_id=2; r.chambre_id=2;
strcpy(r.date,"03/04/2026"); strcpy(r.type,"Bruit");
strcpy(r.description,"Bruit excessif la nuit dans le couloir");
strcpy(r.priorite,"Moyenne"); strcpy(r.statut,"Ouverte");
inserer_reclamation(r);

r.id=3; r.etudiant_id=3; r.chambre_id=3;
strcpy(r.date,"28/03/2026"); strcpy(r.type,"Electricite");
strcpy(r.description,"Prise electrique defaillante");
strcpy(r.priorite,"Haute"); strcpy(r.statut,"Resolue");
inserer_reclamation(r);
id_reclamation_next = 4;

/* Sauvegarder les donnees demo dans les fichiers textes */
sauvegarder_chambres();
sauvegarder_etudiants();
sauvegarder_paiements();
sauvegarder_reclamations();
printf(" [INFO] Donnees demo creees et sauvegardees dans les fichiers.\n");
}

/* Initialisation : charge depuis fichiers ou cree les donnees demo */
void initialiser_donnees() {
charger_admin(); /* toujours charger les identifiants en premier */
FILE *test = fopen(FICHIER_ETUDIANTS, "r");
if (test != NULL) {
fclose(test);
charger_chambres(); /* charger chambres en premier (references) */
charger_etudiants();
charger_paiements();
charger_reclamations();
printf(" [INFO] Donnees chargees depuis les fichiers.\n");
} else {
printf(" [INFO] Premiere utilisation : creation des donnees de demonstration.\n");
charger_donnees_demo();
}
}

/* ============================================================
* AFFICHAGE (parcours de listes + printf formate)
* ============================================================ */

void afficher_entete_etudiant() {
printf(" | %-4s | %-18s | %-18s | %-12s | %-8s | %-8s |\n",
"ID","Nom","Prenom","CIN","Chambre","Statut");
afficher_ligne();
}

void afficher_etudiant(Etudiant e) {
char ch_num[15] = "---";
if (e.chambre_id > 0) {
NoeudChambre *nc = trouver_chambre(e.chambre_id);
if (nc != NULL) snprintf(ch_num, 15, "%s", nc->data.numero);
}
printf(" | %-4d | %-18s | %-18s | %-12s | %-8s | %-8s |\n",
e.id, e.nom, e.prenom, e.cin, ch_num, e.statut);
}

void afficher_entete_chambre() {
printf(" | %-4s | %-8s | %-8s | %-5s | %-5s | %-5s | %-10s | %-12s |\n",
"ID","Numero","Type","Etage","Cap.","Occ.","Prix/mois","Statut");
afficher_ligne();
}

void afficher_chambre(Chambre c) {
printf(" | %-4d | %-8s | %-8s | %-5d | %-5d | %-5d | %-10.2f | %-12s |\n",
c.id, c.numero, c.type, c.etage,
c.capacite, c.nb_occupants, c.prix_mensuel, c.statut);
}

void afficher_entete_paiement() {
printf(" | %-4s | %-18s | %-8s | %-10s | %-12s | %-12s | %-10s | %-10s |\n",
"ID","Etudiant","Chambre","Montant","Date","Mois","Mode","Statut");
afficher_ligne();
}

void afficher_paiement(Paiement p) {
char nom_et[60] = "---", num_ch[10] = "---";
NoeudEtudiant *ne = trouver_etudiant(p.etudiant_id);
NoeudChambre *nc = trouver_chambre(p.chambre_id);
if (ne != NULL) snprintf(nom_et, 60, "%s %s", ne->data.prenom, ne->data.nom);
if (nc != NULL) snprintf(num_ch, 10, "%s", nc->data.numero);
printf(" | %-4d | %-18s | %-8s | %-10.2f | %-12s | %-12s | %-10s | %-10s |\n",
p.id, nom_et, num_ch, p.montant,
p.date_paiement, p.mois, p.mode, p.statut);
}

void afficher_entete_reclamation() {
printf(" | %-4s | %-18s | %-8s | %-12s | %-15s | %-8s | %-10s |\n",
"ID","Etudiant","Chambre","Date","Type","Priorite","Statut");
afficher_ligne();
}

void afficher_reclamation(Reclamation r) {
char nom_et[60] = "---", num_ch[10] = "---";
NoeudEtudiant *ne = trouver_etudiant(r.etudiant_id);
NoeudChambre *nc = trouver_chambre(r.chambre_id);
if (ne != NULL) snprintf(nom_et, 60, "%s %s", ne->data.prenom, ne->data.nom);
if (nc != NULL) snprintf(num_ch, 10, "%s", nc->data.numero);
printf(" | %-4d | %-18s | %-8s | %-12s | %-15s | %-8s | %-10s |\n",
r.id, nom_et, num_ch, r.date, r.type, r.priorite, r.statut);
}

/* ============================================================
* SAISIE DES DONNEES
* ============================================================ */

void saisir_etudiant(Etudiant *e) {
e->id = id_etudiant_next++;
printf(" Nom : "); scanf("%49s", e->nom); vider_buffer();
printf(" Prenom : "); scanf("%49s", e->prenom); vider_buffer();
printf(" CIN : "); scanf("%14s", e->cin); vider_buffer();
printf(" Date naiss. : "); scanf("%14s", e->date_naissance); vider_buffer();
printf(" Email : "); scanf("%59s", e->email); vider_buffer();
printf(" Telephone : "); scanf("%14s", e->telephone); vider_buffer();
printf(" Statut (Actif/Inactif): "); scanf("%19s", e->statut); vider_buffer();
e->chambre_id = 0;
}

void saisir_chambre(Chambre *c) {
c->id = id_chambre_next++;
printf(" Numero (ex A101) : "); scanf("%9s", c->numero); vider_buffer();
printf(" Type (Simple/Double/Triple) : "); scanf("%19s", c->type); vider_buffer();
printf(" Etage : "); scanf("%d", &c->etage); vider_buffer();
printf(" Capacite : "); scanf("%d", &c->capacite); vider_buffer();
printf(" Prix mensuel (DT) : "); scanf("%f", &c->prix_mensuel); vider_buffer();
printf(" Statut (Disponible/Occupee/Maintenance) : "); scanf("%19s", c->statut); vider_buffer();
c->nb_occupants = 0;
}

void saisir_paiement(Paiement *p) {
p->id = id_paiement_next++;
printf(" ID Etudiant : "); scanf("%d", &p->etudiant_id); vider_buffer();
printf(" ID Chambre : "); scanf("%d", &p->chambre_id); vider_buffer();
printf(" Montant (DT) : "); scanf("%f", &p->montant); vider_buffer();
printf(" Date paiement (JJ/MM/AAAA) : "); scanf("%14s", p->date_paiement); vider_buffer();
printf(" Mois concerne : "); scanf("%19s", p->mois); vider_buffer();
printf(" Mode (Especes/Virement/Cheque) : "); scanf("%19s", p->mode); vider_buffer();
printf(" Statut (Paye/En_attente/Retard) : "); scanf("%19s", p->statut); vider_buffer();
}

void saisir_reclamation(Reclamation *r) {
r->id = id_reclamation_next++;
printf(" ID Etudiant : "); scanf("%d", &r->etudiant_id); vider_buffer();
printf(" ID Chambre concernee : "); scanf("%d", &r->chambre_id); vider_buffer();
printf(" Date (JJ/MM/AAAA) : "); scanf("%14s", r->date); vider_buffer();
printf(" Type (Plomberie/Electricite/Bruit/Autre): "); scanf("%39s", r->type); vider_buffer();
printf(" Description : ");
fgets(r->description, 200, stdin);
strip_nl(r->description);
printf(" Priorite (Haute/Moyenne/Basse) : "); scanf("%14s", r->priorite); vider_buffer();
printf(" Statut (Ouverte/En_cours/Resolue/Fermee): "); scanf("%19s", r->statut); vider_buffer();
}

/* ============================================================
* MODULE 1 : ETUDIANTS
* ============================================================ */

void ajouter_etudiant() {
afficher_entete("AJOUTER UN ETUDIANT RESIDENT");
Etudiant e;
saisir_etudiant(&e);
inserer_etudiant(e); /* insertion fin de liste chainee */
sauvegarder_etudiants(); /* persistance dans fichier texte */
printf("\n [OK] Etudiant ajoute (ID: %d). Fichier : %s\n", e.id, FICHIER_ETUDIANTS);
pause_attente();
}

void afficher_tous_etudiants() {
afficher_entete("LISTE DES ETUDIANTS RESIDENTS");
if (liste_etudiants == NULL) { printf(" Aucun etudiant.\n"); pause_attente(); return; }
afficher_entete_etudiant();
/* Parcours de la liste chainee */
NoeudEtudiant *courant = liste_etudiants;
while (courant != NULL) {
afficher_etudiant(courant->data);
courant = courant->suivant;
}
printf("\n Total : %d etudiant(s).\n", longueur_etudiants());
pause_attente();
}

void rechercher_etudiant() {
afficher_entete("RECHERCHER UN ETUDIANT");
printf(" 1. Par ID\n 2. Par Nom\n 3. Par CIN\n 4. Par Statut\n Votre choix : ");
int choix; scanf("%d", &choix); vider_buffer();
int trouve = 0;
afficher_entete_etudiant();

if (choix == 1) {
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudEtudiant *n = trouver_etudiant(id);
if (n != NULL) { afficher_etudiant(n->data); trouve = 1; }
} else {
char val[60];
if (choix == 2) { printf(" Nom : "); scanf("%49s", val); }
else if (choix == 3) { printf(" CIN : "); scanf("%14s", val); }
else if (choix == 4) { printf(" Statut (Actif/Inactif) : "); scanf("%19s", val); }
vider_buffer();
NoeudEtudiant *courant = liste_etudiants;
while (courant != NULL) {
char *cible = (choix==2) ? courant->data.nom :
(choix==3) ? courant->data.cin : courant->data.statut;
if (strcasecmp(cible, val) == 0) { afficher_etudiant(courant->data); trouve = 1; }
courant = courant->suivant;
}
}
if (!trouve) printf(" Aucun etudiant trouve.\n");
pause_attente();
}

void modifier_etudiant() {
afficher_entete("MODIFIER UN ETUDIANT");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudEtudiant *n = trouver_etudiant(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
int old_id = n->data.id, old_ch = n->data.chambre_id;
saisir_etudiant(&n->data);
n->data.id = old_id;
n->data.chambre_id = old_ch;
id_etudiant_next--;
sauvegarder_etudiants();
printf("\n [OK] Etudiant modifie. Fichier : %s\n", FICHIER_ETUDIANTS);
pause_attente();
}

void supprimer_etudiant() {
afficher_entete("SUPPRIMER UN ETUDIANT");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudEtudiant *n = trouver_etudiant(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
printf(" Supprimer %s %s ? (o/n) : ", n->data.prenom, n->data.nom);
char conf; scanf("%c", &conf); vider_buffer();
if (conf == 'o' || conf == 'O') {
supprimer_noeud_etudiant(id); /* suppression dans la liste */
sauvegarder_etudiants(); /* mise a jour du fichier */
printf(" [OK] Supprime. Fichier : %s\n", FICHIER_ETUDIANTS);
} else printf(" Annule.\n");
pause_attente();
}

void assigner_chambre() {
afficher_entete("ASSIGNER UNE CHAMBRE A UN ETUDIANT");
int eid; printf(" ID Etudiant : "); scanf("%d", &eid); vider_buffer();
NoeudEtudiant *ne = trouver_etudiant(eid);
if (ne == NULL) { printf(" [ERREUR] Etudiant introuvable.\n"); pause_attente(); return; }

int cid; printf(" ID Chambre : "); scanf("%d", &cid); vider_buffer();
NoeudChambre *nc = trouver_chambre(cid);
if (nc == NULL) { printf(" [ERREUR] Chambre introuvable.\n"); pause_attente(); return; }

if (nc->data.nb_occupants >= nc->data.capacite) {
printf(" [ERREUR] Chambre pleine (capacite : %d).\n", nc->data.capacite);
pause_attente(); return;
}
ne->data.chambre_id = cid;
nc->data.nb_occupants++;
if (nc->data.nb_occupants >= nc->data.capacite)
strcpy(nc->data.statut, "Occupee");

sauvegarder_etudiants();
sauvegarder_chambres();
printf("\n [OK] Chambre %s assignee a %s %s.\n",
nc->data.numero, ne->data.prenom, ne->data.nom);
pause_attente();
}

/* ============================================================
* MODULE 2 : CHAMBRES
* ============================================================ */

void ajouter_chambre() {
afficher_entete("AJOUTER UNE CHAMBRE");
Chambre c;
saisir_chambre(&c);
inserer_chambre(c);
sauvegarder_chambres();
printf("\n [OK] Chambre ajoutee (ID: %d). Fichier : %s\n", c.id, FICHIER_CHAMBRES);
pause_attente();
}

void afficher_toutes_chambres() {
afficher_entete("LISTE DES CHAMBRES");
if (liste_chambres == NULL) { printf(" Aucune chambre.\n"); pause_attente(); return; }
afficher_entete_chambre();
NoeudChambre *courant = liste_chambres;
while (courant != NULL) {
afficher_chambre(courant->data);
courant = courant->suivant;
}
printf("\n Total : %d chambre(s).\n", longueur_chambres());
pause_attente();
}

void rechercher_chambre() {
afficher_entete("RECHERCHER UNE CHAMBRE");
printf(" 1. Par ID\n 2. Par Numero\n 3. Par Type\n 4. Par Statut\n 5. Chambres disponibles\n Votre choix : ");
int choix; scanf("%d", &choix); vider_buffer();
int trouve = 0;
afficher_entete_chambre();

if (choix == 1) {
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudChambre *n = trouver_chambre(id);
if (n != NULL) { afficher_chambre(n->data); trouve = 1; }
} else {
char val[20] = "";
if (choix == 2) { printf(" Numero : "); scanf("%9s", val); vider_buffer(); }
else if (choix == 3) { printf(" Type : "); scanf("%19s", val); vider_buffer(); }
else if (choix == 4) { printf(" Statut : "); scanf("%19s", val); vider_buffer(); }
else if (choix == 5) { strcpy(val, "Disponible"); }
NoeudChambre *courant = liste_chambres;
while (courant != NULL) {
char *cible = (choix==2) ? courant->data.numero :
(choix==3) ? courant->data.type : courant->data.statut;
if (strcasecmp(cible, val) == 0) { afficher_chambre(courant->data); trouve = 1; }
courant = courant->suivant;
}
}
if (!trouve) printf(" Aucune chambre trouvee.\n");
pause_attente();
}

void modifier_chambre() {
afficher_entete("MODIFIER UNE CHAMBRE");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudChambre *n = trouver_chambre(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
int old_id = n->data.id, old_occ = n->data.nb_occupants;
saisir_chambre(&n->data);
n->data.id = old_id;
n->data.nb_occupants = old_occ;
id_chambre_next--;
sauvegarder_chambres();
printf("\n [OK] Chambre modifiee. Fichier : %s\n", FICHIER_CHAMBRES);
pause_attente();
}

void supprimer_chambre() {
afficher_entete("SUPPRIMER UNE CHAMBRE");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudChambre *n = trouver_chambre(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
if (n->data.nb_occupants > 0) {
printf(" [ERREUR] Chambre occupee (%d resident(s)), impossible de supprimer.\n",
n->data.nb_occupants);
pause_attente(); return;
}
printf(" Supprimer chambre %s ? (o/n) : ", n->data.numero);
char conf; scanf("%c", &conf); vider_buffer();
if (conf == 'o' || conf == 'O') {
supprimer_noeud_chambre(id);
sauvegarder_chambres();
printf(" [OK] Chambre supprimee. Fichier : %s\n", FICHIER_CHAMBRES);
} else printf(" Annule.\n");
pause_attente();
}

/* ============================================================
* MODULE 3 : PAIEMENTS
* ============================================================ */

void ajouter_paiement() {
afficher_entete("ENREGISTRER UN PAIEMENT");

/* Validation : l'etudiant doit exister */
int eid;
NoeudEtudiant *ne = NULL;
do {
printf(" ID Etudiant : "); scanf("%d", &eid); vider_buffer();
ne = trouver_etudiant(eid);
if (ne == NULL)
printf(" [ERREUR] Etudiant ID %d introuvable. Reessayez.\n", eid);
} while (ne == NULL);

/* Validation : la chambre doit exister */
int cid;
NoeudChambre *nc = NULL;
do {
printf(" ID Chambre : "); scanf("%d", &cid); vider_buffer();
nc = trouver_chambre(cid);
if (nc == NULL)
printf(" [ERREUR] Chambre ID %d introuvable. Reessayez.\n", cid);
} while (nc == NULL);

Paiement p;
p.id = id_paiement_next++;
p.etudiant_id = eid;
p.chambre_id = cid;

printf(" Montant (DT) : "); scanf("%f", &p.montant); vider_buffer();
printf(" Date paiement (JJ/MM/AAAA) : "); scanf("%14s", p.date_paiement); vider_buffer();
printf(" Mois concerne : "); scanf("%19s", p.mois); vider_buffer();
printf(" Mode (Especes/Virement/Cheque) : "); scanf("%19s", p.mode); vider_buffer();
printf(" Statut (Paye/En_attente/Retard) : "); scanf("%19s", p.statut); vider_buffer();

inserer_paiement(p);
sauvegarder_paiements();
printf("\n [OK] Paiement de %s %s enregistre (ID: %d).\n",
ne->data.prenom, ne->data.nom, p.id);
pause_attente();
}

void afficher_tous_paiements() {
afficher_entete("LISTE DES PAIEMENTS");
if (liste_paiements == NULL) { printf(" Aucun paiement.\n"); pause_attente(); return; }
afficher_entete_paiement();
NoeudPaiement *courant = liste_paiements;
while (courant != NULL) {
afficher_paiement(courant->data);
courant = courant->suivant;
}
printf("\n Total : %d paiement(s).\n", longueur_paiements());
pause_attente();
}

void rechercher_paiement() {
afficher_entete("RECHERCHER UN PAIEMENT");
printf(" 1. Par ID Etudiant\n 2. Par Mois\n 3. Par Statut\n 4. Par Mode\n Votre choix : ");
int choix; scanf("%d", &choix); vider_buffer();
int trouve = 0;
afficher_entete_paiement();

NoeudPaiement *courant = liste_paiements;
if (choix == 1) {
int id; printf(" ID Etudiant : "); scanf("%d", &id); vider_buffer();
while (courant != NULL) {
if (courant->data.etudiant_id == id) { afficher_paiement(courant->data); trouve = 1; }
courant = courant->suivant;
}
} else {
char val[20];
if (choix == 2) { printf(" Mois : "); scanf("%19s", val); }
else if (choix == 3) { printf(" Statut (Paye/En_attente/Retard) : "); scanf("%19s", val); }
else if (choix == 4) { printf(" Mode : "); scanf("%19s", val); }
vider_buffer();
while (courant != NULL) {
char *cible = (choix==2) ? courant->data.mois :
(choix==3) ? courant->data.statut : courant->data.mode;
if (strcasecmp(cible, val) == 0) { afficher_paiement(courant->data); trouve = 1; }
courant = courant->suivant;
}
}
if (!trouve) printf(" Aucun paiement trouve.\n");
pause_attente();
}

void bilan_financier() {
afficher_entete("BILAN FINANCIER");
float paye = 0, attente = 0, retard = 0;
NoeudPaiement *courant = liste_paiements;
while (courant != NULL) {
Paiement *p = &courant->data;
if (strcasecmp(p->statut, "Paye") == 0) paye += p->montant;
else if (strcasecmp(p->statut, "En_attente") == 0) attente += p->montant;
else if (strcasecmp(p->statut, "Retard") == 0) retard += p->montant;
courant = courant->suivant;
}
printf(" Paiements enregistres : %d\n", longueur_paiements());
printf(" Total encaisse (Paye) : %.2f DT\n", paye);
printf(" En attente : %.2f DT\n", attente);
printf(" En retard : %.2f DT\n", retard);
printf(" TOTAL GLOBAL : %.2f DT\n", paye + attente + retard);
pause_attente();
}

void modifier_paiement() {
afficher_entete("MODIFIER UN PAIEMENT");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudPaiement *n = trouver_paiement(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }

/* Validation : nouvel etudiant_id */
int eid;
NoeudEtudiant *ne = NULL;
do {
printf(" ID Etudiant : "); scanf("%d", &eid); vider_buffer();
ne = trouver_etudiant(eid);
if (ne == NULL) printf(" [ERREUR] Etudiant ID %d introuvable. Reessayez.\n", eid);
} while (ne == NULL);

/* Validation : nouvelle chambre_id */
int cid;
NoeudChambre *nc = NULL;
do {
printf(" ID Chambre : "); scanf("%d", &cid); vider_buffer();
nc = trouver_chambre(cid);
if (nc == NULL) printf(" [ERREUR] Chambre ID %d introuvable. Reessayez.\n", cid);
} while (nc == NULL);

n->data.etudiant_id = eid;
n->data.chambre_id = cid;
printf(" Montant (DT) : "); scanf("%f", &n->data.montant); vider_buffer();
printf(" Date paiement (JJ/MM/AAAA) : "); scanf("%14s", n->data.date_paiement); vider_buffer();
printf(" Mois concerne : "); scanf("%19s", n->data.mois); vider_buffer();
printf(" Mode (Especes/Virement/Cheque) : "); scanf("%19s", n->data.mode); vider_buffer();
printf(" Statut (Paye/En_attente/Retard) : "); scanf("%19s", n->data.statut); vider_buffer();

sauvegarder_paiements();
printf("\n [OK] Paiement modifie. Fichier : %s\n", FICHIER_PAIEMENTS);
pause_attente();
}

void supprimer_paiement() {
afficher_entete("SUPPRIMER UN PAIEMENT");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudPaiement *n = trouver_paiement(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
printf(" Supprimer paiement ID %d ? (o/n) : ", id);
char conf; scanf("%c", &conf); vider_buffer();
if (conf == 'o' || conf == 'O') {
supprimer_noeud_paiement(id);
sauvegarder_paiements();
printf(" [OK] Supprime. Fichier : %s\n", FICHIER_PAIEMENTS);
} else printf(" Annule.\n");
pause_attente();
}

/* ============================================================
* MODULE 4 : RECLAMATIONS
* ============================================================ */

void ajouter_reclamation() {
afficher_entete("DEPOSER UNE RECLAMATION");

/* Validation : l'etudiant doit exister dans la liste */
int eid;
NoeudEtudiant *ne = NULL;
do {
printf(" ID Etudiant : "); scanf("%d", &eid); vider_buffer();
ne = trouver_etudiant(eid);
if (ne == NULL)
printf(" [ERREUR] Etudiant ID %d introuvable. Reessayez.\n", eid);
} while (ne == NULL);

/* Validation : la chambre doit exister dans la liste */
int cid;
NoeudChambre *nc = NULL;
do {
printf(" ID Chambre : "); scanf("%d", &cid); vider_buffer();
nc = trouver_chambre(cid);
if (nc == NULL)
printf(" [ERREUR] Chambre ID %d introuvable. Reessayez.\n", cid);
} while (nc == NULL);

Reclamation r;
r.id = id_reclamation_next++;
r.etudiant_id = eid;
r.chambre_id = cid;

printf(" Date (JJ/MM/AAAA) : "); scanf("%14s", r.date); vider_buffer();
printf(" Type (Plomberie/Electricite/Bruit/Autre) : "); scanf("%39s", r.type); vider_buffer();
printf(" Description : "); fgets(r.description, 200, stdin); strip_nl(r.description);
printf(" Priorite (Haute/Moyenne/Basse) : "); scanf("%14s", r.priorite); vider_buffer();
printf(" Statut (Ouverte/En_cours/Resolue/Fermee): "); scanf("%19s", r.statut); vider_buffer();

inserer_reclamation(r);
sauvegarder_reclamations();
printf("\n [OK] Reclamation de %s %s enregistree (ID: %d).\n",
ne->data.prenom, ne->data.nom, r.id);
pause_attente();
}

void afficher_toutes_reclamations() {
afficher_entete("LISTE DES RECLAMATIONS");
if (liste_reclamations == NULL) { printf(" Aucune reclamation.\n"); pause_attente(); return; }
afficher_entete_reclamation();
NoeudReclamation *courant = liste_reclamations;
while (courant != NULL) {
afficher_reclamation(courant->data);
courant = courant->suivant;
}
printf("\n Total : %d reclamation(s).\n", longueur_reclamations());
pause_attente();
}

void rechercher_reclamation() {
afficher_entete("RECHERCHER UNE RECLAMATION");
printf(" 1. Par ID Etudiant\n 2. Par Type\n 3. Par Priorite\n 4. Par Statut\n Votre choix : ");
int choix; scanf("%d", &choix); vider_buffer();
int trouve = 0;
afficher_entete_reclamation();

NoeudReclamation *courant = liste_reclamations;
if (choix == 1) {
int id; printf(" ID Etudiant : "); scanf("%d", &id); vider_buffer();
while (courant != NULL) {
if (courant->data.etudiant_id == id) { afficher_reclamation(courant->data); trouve = 1; }
courant = courant->suivant;
}
} else {
char val[40];
if (choix == 2) { printf(" Type : "); scanf("%39s", val); }
else if (choix == 3) { printf(" Priorite (Haute/Moyenne/Basse) : "); scanf("%14s", val); }
else if (choix == 4) { printf(" Statut (Ouverte/En_cours/Resolue/Fermee) : "); scanf("%19s", val); }
vider_buffer();
while (courant != NULL) {
char *cible = (choix==2) ? courant->data.type :
(choix==3) ? courant->data.priorite : courant->data.statut;
if (strcasecmp(cible, val) == 0) { afficher_reclamation(courant->data); trouve = 1; }
courant = courant->suivant;
}
}
if (!trouve) printf(" Aucune reclamation trouvee.\n");
pause_attente();
}

void changer_statut_reclamation() {
afficher_entete("METTRE A JOUR LE STATUT D'UNE RECLAMATION");
int id; printf(" ID Reclamation : "); scanf("%d", &id); vider_buffer();
NoeudReclamation *n = trouver_reclamation(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
printf(" Statut actuel : %s\n", n->data.statut);
printf(" Nouveau statut (Ouverte/En_cours/Resolue/Fermee) : ");
scanf("%19s", n->data.statut); vider_buffer();
sauvegarder_reclamations();
printf(" [OK] Statut mis a jour. Fichier : %s\n", FICHIER_RECLAMATIONS);
pause_attente();
}

void modifier_reclamation() {
afficher_entete("MODIFIER UNE RECLAMATION");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudReclamation *n = trouver_reclamation(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }

/* Validation : nouvel etudiant_id */
int eid;
NoeudEtudiant *ne = NULL;
do {
printf(" ID Etudiant : "); scanf("%d", &eid); vider_buffer();
ne = trouver_etudiant(eid);
if (ne == NULL) printf(" [ERREUR] Etudiant ID %d introuvable. Reessayez.\n", eid);
} while (ne == NULL);

/* Validation : nouvelle chambre_id */
int cid;
NoeudChambre *nc = NULL;
do {
printf(" ID Chambre : "); scanf("%d", &cid); vider_buffer();
nc = trouver_chambre(cid);
if (nc == NULL) printf(" [ERREUR] Chambre ID %d introuvable. Reessayez.\n", cid);
} while (nc == NULL);

n->data.etudiant_id = eid;
n->data.chambre_id = cid;
printf(" Date (JJ/MM/AAAA) : "); scanf("%14s", n->data.date); vider_buffer();
printf(" Type (Plomberie/Electricite/Bruit/Autre) : "); scanf("%39s", n->data.type); vider_buffer();
printf(" Description : "); fgets(n->data.description, 200, stdin); strip_nl(n->data.description);
printf(" Priorite (Haute/Moyenne/Basse) : "); scanf("%14s", n->data.priorite); vider_buffer();
printf(" Statut (Ouverte/En_cours/Resolue/Fermee): "); scanf("%19s", n->data.statut); vider_buffer();

sauvegarder_reclamations();
printf("\n [OK] Reclamation modifiee. Fichier : %s\n", FICHIER_RECLAMATIONS);
pause_attente();
}

void supprimer_reclamation() {
afficher_entete("SUPPRIMER UNE RECLAMATION");
int id; printf(" ID : "); scanf("%d", &id); vider_buffer();
NoeudReclamation *n = trouver_reclamation(id);
if (n == NULL) { printf(" [ERREUR] Introuvable.\n"); pause_attente(); return; }
printf(" Supprimer reclamation ID %d ? (o/n) : ", id);
char conf; scanf("%c", &conf); vider_buffer();
if (conf == 'o' || conf == 'O') {
supprimer_noeud_reclamation(id);
sauvegarder_reclamations();
printf(" [OK] Supprimee. Fichier : %s\n", FICHIER_RECLAMATIONS);
} else printf(" Annule.\n");
pause_attente();
}

/* ============================================================
* TABLEAU DE BORD (parcours des quatre listes)
* ============================================================ */
void tableau_de_bord() {
afficher_entete("TABLEAU DE BORD - FOYER UNIVERSITAIRE");

/* Taux d'occupation : parcourir liste des chambres */
int total_cap = 0, total_occ = 0;
NoeudChambre *nc = liste_chambres;
while (nc != NULL) {
total_cap += nc->data.capacite;
total_occ += nc->data.nb_occupants;
nc = nc->suivant;
}
float taux = (total_cap > 0) ? (float)total_occ / total_cap * 100.0f : 0;

printf(" --- Residents ---\n");
printf(" Etudiants inscrits : %d\n", longueur_etudiants());

printf("\n --- Chambres ---\n");
printf(" Chambres enregistrees : %d\n", longueur_chambres());
printf(" Taux d'occupation : %.1f%% (%d / %d places)\n", taux, total_occ, total_cap);

int dispo = 0, maint = 0;
nc = liste_chambres;
while (nc != NULL) {
if (strcasecmp(nc->data.statut, "Disponible") == 0) dispo++;
if (strcasecmp(nc->data.statut, "Maintenance") == 0) maint++;
nc = nc->suivant;
}
printf(" Chambres disponibles : %d\n", dispo);
printf(" En maintenance : %d\n", maint);

printf("\n --- Paiements ---\n");
float paye = 0, attente = 0, retard = 0;
NoeudPaiement *np = liste_paiements;
while (np != NULL) {
if (strcasecmp(np->data.statut, "Paye") == 0) paye += np->data.montant;
else if (strcasecmp(np->data.statut, "En_attente") == 0) attente += np->data.montant;
else if (strcasecmp(np->data.statut, "Retard") == 0) retard += np->data.montant;
np = np->suivant;
}
printf(" Total encaisse : %.2f DT\n", paye);
printf(" En attente : %.2f DT\n", attente);
printf(" En retard : %.2f DT\n", retard);

printf("\n --- Reclamations ---\n");
int ouv = 0, enc = 0, res = 0;
NoeudReclamation *nr = liste_reclamations;
while (nr != NULL) {
if (strcasecmp(nr->data.statut, "Ouverte") == 0) ouv++;
else if (strcasecmp(nr->data.statut, "En_cours") == 0) enc++;
else if (strcasecmp(nr->data.statut, "Resolue") == 0) res++;
nr = nr->suivant;
}
printf(" Ouvertes : %d\n", ouv);
printf(" En cours : %d\n", enc);
printf(" Resolues : %d\n", res);

pause_attente();
}

/* ============================================================
* MENUS
* ============================================================ */

void menu_etudiants() {
int choix;
do {
afficher_entete("MODULE - GESTION DES ETUDIANTS RESIDENTS");
printf(" 1. Ajouter un etudiant resident\n");
printf(" 2. Afficher tous les etudiants\n");
printf(" 3. Rechercher un etudiant\n");
printf(" 4. Modifier un etudiant\n");
printf(" 5. Supprimer un etudiant\n");
printf(" 6. Assigner une chambre\n");
printf(" 0. Retour\n");
afficher_ligne();
printf(" Votre choix : "); scanf("%d", &choix); vider_buffer();
switch (choix) {
case 1: ajouter_etudiant(); break;
case 2: afficher_tous_etudiants(); break;
case 3: rechercher_etudiant(); break;
case 4: modifier_etudiant(); break;
case 5: supprimer_etudiant(); break;
case 6: assigner_chambre(); break;
case 0: break;
default: printf(" [ERREUR] Choix invalide.\n"); pause_attente();
}
} while (choix != 0);
}

void menu_chambres() {
int choix;
do {
afficher_entete("MODULE - GESTION DES CHAMBRES");
printf(" 1. Ajouter une chambre\n");
printf(" 2. Afficher toutes les chambres\n");
printf(" 3. Rechercher une chambre\n");
printf(" 4. Modifier une chambre\n");
printf(" 5. Supprimer une chambre\n");
printf(" 0. Retour\n");
afficher_ligne();
printf(" Votre choix : "); scanf("%d", &choix); vider_buffer();
switch (choix) {
case 1: ajouter_chambre(); break;
case 2: afficher_toutes_chambres(); break;
case 3: rechercher_chambre(); break;
case 4: modifier_chambre(); break;
case 5: supprimer_chambre(); break;
case 0: break;
default: printf(" [ERREUR] Choix invalide.\n"); pause_attente();
}
} while (choix != 0);
}

void menu_paiements() {
int choix;
do {
afficher_entete("MODULE - GESTION DES PAIEMENTS");
printf(" 1. Enregistrer un paiement\n");
printf(" 2. Afficher tous les paiements\n");
printf(" 3. Rechercher un paiement\n");
printf(" 4. Bilan financier\n");
printf(" 5. Modifier un paiement\n");
printf(" 6. Supprimer un paiement\n");
printf(" 0. Retour\n");
afficher_ligne();
printf(" Votre choix : "); scanf("%d", &choix); vider_buffer();
switch (choix) {
case 1: ajouter_paiement(); break;
case 2: afficher_tous_paiements(); break;
case 3: rechercher_paiement(); break;
case 4: bilan_financier(); break;
case 5: modifier_paiement(); break;
case 6: supprimer_paiement(); break;
case 0: break;
default: printf(" [ERREUR] Choix invalide.\n"); pause_attente();
}
} while (choix != 0);
}

void menu_reclamations() {
int choix;
do {
afficher_entete("MODULE - GESTION DES RECLAMATIONS");
printf(" 1. Deposer une reclamation\n");
printf(" 2. Afficher toutes les reclamations\n");
printf(" 3. Rechercher une reclamation\n");
printf(" 4. Mettre a jour le statut\n");
printf(" 5. Modifier une reclamation\n");
printf(" 6. Supprimer une reclamation\n");
printf(" 0. Retour\n");
afficher_ligne();
printf(" Votre choix : "); scanf("%d", &choix); vider_buffer();
switch (choix) {
case 1: ajouter_reclamation(); break;
case 2: afficher_toutes_reclamations(); break;
case 3: rechercher_reclamation(); break;
case 4: changer_statut_reclamation(); break;
case 5: modifier_reclamation(); break;
case 6: supprimer_reclamation(); break;
case 0: break;
default: printf(" [ERREUR] Choix invalide.\n"); pause_attente();
}
} while (choix != 0);
}

void menu_principal() {
int choix;
do {
afficher_entete("MENU PRINCIPAL - GESTION DE FOYER UNIVERSITAIRE");
printf(" 1. Gestion des Etudiants (Residents)\n");
printf(" 2. Gestion des Chambres\n");
printf(" 3. Gestion des Paiements\n");
printf(" 4. Gestion des Reclamations\n");
printf(" 5. Tableau de bord\n");
printf(" 6. Modifier login / mot de passe admin\n");
printf(" 0. Quitter\n");
afficher_ligne();
printf(" Votre choix : "); scanf("%d", &choix); vider_buffer();
switch (choix) {
case 1: menu_etudiants(); break;
case 2: menu_chambres(); break;
case 3: menu_paiements(); break;
case 4: menu_reclamations(); break;
case 5: tableau_de_bord(); break;
case 6: modifier_identifiants_admin(); break;
case 0: printf("\n Au revoir !\n\n"); break;
default: printf(" [ERREUR] Choix invalide.\n"); pause_attente();
}
} while (choix != 0);
}

/* ============================================================
* LIBERATION MEMOIRE (detruire toutes les listes)
* ============================================================ */
void liberer_memoire() {
detruire_liste_etudiants();
detruire_liste_chambres();
detruire_liste_paiements();
detruire_liste_reclamations();
}

/* ============================================================
* MAIN
* ============================================================ */
int main() {
initialiser_donnees(); /* charger depuis fichiers ou demo */

if (!authentifier()) {
printf("\n Acces refuse. Fermeture du programme.\n\n");
liberer_memoire();
return 1;
}

menu_principal();

liberer_memoire(); /* liberer tous les noeuds (free) */
return 0;
}
