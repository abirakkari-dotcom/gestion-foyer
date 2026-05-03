/*
 * ============================================================
 * APPLICATION DE GESTION DE FOYER UNIVERSITAIRE
 * Version : Listes chaînées
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

/* ============================================================
 * STRUCTURES DE DONNEES
 * ============================================================ */

typedef struct {
    int   id;
    char  nom[50];
    char  prenom[50];
    char  cin[15];
    char  date_naissance[15];
    char  email[60];
    char  telephone[15];
    int   chambre_id;
    char  statut[20];
} Etudiant;

typedef struct {
    int   id;
    char  numero[10];
    char  type[20];
    int   etage;
    float prix_mensuel;
    int   capacite;
    int   nb_occupants;
    char  statut[20];
} Chambre;

typedef struct {
    int   id;
    int   etudiant_id;
    int   chambre_id;
    float montant;
    char  date_paiement[15];
    char  mois[20];
    char  mode[20];
    char  statut[20];
} Paiement;

typedef struct {
    int   id;
    int   etudiant_id;
    int   chambre_id;
    char  date[15];
    char  type[40];
    char  description[200];
    char  priorite[15];
    char  statut[20];
} Reclamation;

/* ============================================================
 * STRUCTURES POUR LISTES CHAINEES
 * ============================================================ */

typedef struct CelluleEtudiant {
    Etudiant data;
    struct CelluleEtudiant* suivant;
} CelluleEtudiant;

typedef struct CelluleChambre {
    Chambre data;
    struct CelluleChambre* suivant;
} CelluleChambre;

typedef struct CellulePaiement {
    Paiement data;
    struct CellulePaiement* suivant;
} CellulePaiement;

typedef struct CelluleReclamation {
    Reclamation data;
    struct CelluleReclamation* suivant;
} CelluleReclamation;

/* ============================================================
 * VARIABLES GLOBALES
 * ============================================================ */
CelluleEtudiant* liste_etudiants = NULL;
CelluleChambre* liste_chambres = NULL;
CellulePaiement* liste_paiements = NULL;
CelluleReclamation* liste_reclamations = NULL;

int nb_etudiants    = 0;
int nb_chambres     = 0;
int nb_paiements    = 0;
int nb_reclamations = 0;

int id_etudiant_next    = 1;
int id_chambre_next     = 1;
int id_paiement_next    = 1;
int id_reclamation_next = 1;

/* ============================================================
 * UTILITAIRES
 * ============================================================ */

void afficher_separateur() {
    printf("================================================================\n");
}
void afficher_ligne() {
    printf("----------------------------------------------------------------\n");
}
void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
void pause_attente() {
    printf("\n  Appuyez sur Entree pour continuer...");
    vider_buffer();
}
void afficher_entete(const char *titre) {
    printf("\n");
    afficher_separateur();
    printf("  %s\n", titre);
    afficher_separateur();
}

/* ============================================================
 * FONCTIONS DE RECHERCHE DANS LES LISTES
 * ============================================================ */

CelluleEtudiant* trouver_etudiant_ptr(int id) {
    CelluleEtudiant* temp = liste_etudiants;
    while(temp) { if(temp->data.id == id) return temp; temp = temp->suivant; }
    return NULL;
}

CelluleChambre* trouver_chambre_ptr(int id) {
    CelluleChambre* temp = liste_chambres;
    while(temp) { if(temp->data.id == id) return temp; temp = temp->suivant; }
    return NULL;
}

CellulePaiement* trouver_paiement_ptr(int id) {
    CellulePaiement* temp = liste_paiements;
    while(temp) { if(temp->data.id == id) return temp; temp = temp->suivant; }
    return NULL;
}

CelluleReclamation* trouver_reclamation_ptr(int id) {
    CelluleReclamation* temp = liste_reclamations;
    while(temp) { if(temp->data.id == id) return temp; temp = temp->suivant; }
    return NULL;
}

/* ============================================================
 * AUTHENTIFICATION PAR FICHIER
 * ============================================================ */

int authentifier() {
    char login_fichier[30] ;
    char mdp_fichier[30] ;
    char login[30], mdp[30];
    int tentatives = 3;

    FILE *fichier = fopen("auth.txt", "r");
    if (fichier != NULL) {
        fscanf(fichier, "%s", login_fichier);
        fscanf(fichier, "%s", mdp_fichier);
        fclose(fichier);
    } else {
        fichier = fopen("auth.txt", "w");
        if(fichier) {
            fprintf(fichier, "admin\n1234\n");
            fclose(fichier);
        }
    }

    afficher_entete("CONNEXION - GESTION DE FOYER UNIVERSITAIRE");
    printf("  Systeme de gestion des residents\n");
    afficher_ligne();

    while (tentatives > 0) {
        printf("\n  Login        : "); scanf("%29s", login); vider_buffer();
        printf("  Mot de passe : "); scanf("%29s", mdp);   vider_buffer();

        if (strcmp(login, login_fichier) == 0 && strcmp(mdp, mdp_fichier) == 0) {
            printf("\n  Connexion reussie ! Bienvenue, %s.\n", login);
            pause_attente();
            return 1;
        } else {
            tentatives--;
            if (tentatives > 0)
                printf("\n  [ERREUR] Identifiants incorrects. Tentatives restantes : %d\n", tentatives);
            else
                printf("\n  [ERREUR] Acces refuse apres 3 tentatives.\n");
        }
    }
    return 0;
}

/* ============================================================
 * GESTION DES FICHIERS DE DONNEES (.TXT) - SEULE MODIFICATION
 * ============================================================ */

void sauvegarder_donnees() {
    FILE* fe = fopen("etudiants.txt", "w");
    if (fe) {
        CelluleEtudiant* te = liste_etudiants;
        while(te) {
            fprintf(fe, "%d %s %s %s %s %s %s %d %s\n",
                    te->data.id, te->data.nom, te->data.prenom, te->data.cin,
                    te->data.date_naissance, te->data.email, te->data.telephone,
                    te->data.chambre_id, te->data.statut);
            te = te->suivant;
        }
        fclose(fe);
    }

    FILE* fc = fopen("chambres.txt", "w");
    if (fc) {
        CelluleChambre* tc = liste_chambres;
        while(tc) {
            fprintf(fc, "%d %s %s %d %f %d %d %s\n",
                    tc->data.id, tc->data.numero, tc->data.type, tc->data.etage,
                    tc->data.prix_mensuel, tc->data.capacite, tc->data.nb_occupants, tc->data.statut);
            tc = tc->suivant;
        }
        fclose(fc);
    }

    FILE* fp = fopen("paiements.txt", "w");
    if (fp) {
        CellulePaiement* tp = liste_paiements;
        while(tp) {
            fprintf(fp, "%d %d %d %f %s %s %s %s\n",
                    tp->data.id, tp->data.etudiant_id, tp->data.chambre_id,
                    tp->data.montant, tp->data.date_paiement, tp->data.mois,
                    tp->data.mode, tp->data.statut);
            tp = tp->suivant;
        }
        fclose(fp);
    }

    FILE* fr = fopen("reclamations.txt", "w");
    if (fr) {
        CelluleReclamation* tr = liste_reclamations;
        while(tr) {
            // La description contient des espaces, on la met sur une ligne séparée
            fprintf(fr, "%d %d %d %s %s %s %s\n%s\n",
                    tr->data.id, tr->data.etudiant_id, tr->data.chambre_id,
                    tr->data.date, tr->data.type, tr->data.priorite, tr->data.statut,
                    tr->data.description);
            tr = tr->suivant;
        }
        fclose(fr);
    }
    printf("  [INFO] Toutes les donnees ont ete sauvegardees avec succes en format TXT.\n");
}

void charger_donnees_fichiers() {
    FILE* f;
    Etudiant e; Chambre c; Paiement p; Reclamation r;

    f = fopen("etudiants.txt", "r");
    if (f) {
        while (fscanf(f, "%d %s %s %s %s %s %s %d %s\n",
                      &e.id, e.nom, e.prenom, e.cin, e.date_naissance,
                      e.email, e.telephone, &e.chambre_id, e.statut) == 9) {
            CelluleEtudiant* nouv = (CelluleEtudiant*)malloc(sizeof(CelluleEtudiant));
            nouv->data = e; nouv->suivant = liste_etudiants; liste_etudiants = nouv;
            nb_etudiants++; if (e.id >= id_etudiant_next) id_etudiant_next = e.id + 1;
        }
        fclose(f);
    }

    f = fopen("chambres.txt", "r");
    if (f) {
        while (fscanf(f, "%d %s %s %d %f %d %d %s\n",
                      &c.id, c.numero, c.type, &c.etage, &c.prix_mensuel,
                      &c.capacite, &c.nb_occupants, c.statut) == 8) {
            CelluleChambre* nouv = (CelluleChambre*)malloc(sizeof(CelluleChambre));
            nouv->data = c; nouv->suivant = liste_chambres; liste_chambres = nouv;
            nb_chambres++; if (c.id >= id_chambre_next) id_chambre_next = c.id + 1;
        }
        fclose(f);
    }

    f = fopen("paiements.txt", "r");
    if (f) {
        while (fscanf(f, "%d %d %d %f %s %s %s %s\n",
                      &p.id, &p.etudiant_id, &p.chambre_id, &p.montant,
                      p.date_paiement, p.mois, p.mode, p.statut) == 8) {
            CellulePaiement* nouv = (CellulePaiement*)malloc(sizeof(CellulePaiement));
            nouv->data = p; nouv->suivant = liste_paiements; liste_paiements = nouv;
            nb_paiements++; if (p.id >= id_paiement_next) id_paiement_next = p.id + 1;
        }
        fclose(f);
    }

    f = fopen("reclamations.txt", "r");
    if (f) {
        while (fscanf(f, "%d %d %d %s %s %s %s\n",
                      &r.id, &r.etudiant_id, &r.chambre_id, r.date, r.type, r.priorite, r.statut) == 7) {
            // Lecture de la description qui contient des espaces
            fgets(r.description, 200, f);
            r.description[strcspn(r.description, "\n")] = 0; // Enlever le saut de ligne à la fin
            CelluleReclamation* nouv = (CelluleReclamation*)malloc(sizeof(CelluleReclamation));
            nouv->data = r; nouv->suivant = liste_reclamations; liste_reclamations = nouv;
            nb_reclamations++; if (r.id >= id_reclamation_next) id_reclamation_next = r.id + 1;
        }
        fclose(f);
    }
}

/* ============================================================
 * MODULE 1 : ETUDIANTS
 * ============================================================ */

void saisir_etudiant(Etudiant *e) {
    e->id = id_etudiant_next++;
    printf("  Nom             : "); scanf("%49s", e->nom);            vider_buffer();
    printf("  Prenom          : "); scanf("%49s", e->prenom);         vider_buffer();
    printf("  CIN             : "); scanf("%14s", e->cin);            vider_buffer();
    printf("  Date naiss.     : "); scanf("%14s", e->date_naissance); vider_buffer();
    printf("  Email           : "); scanf("%59s", e->email);          vider_buffer();
    printf("  Telephone       : "); scanf("%14s", e->telephone);      vider_buffer();
    printf("  Statut (Actif/Inactif): "); scanf("%19s", e->statut);   vider_buffer();
    e->chambre_id = 0;
}

void afficher_entete_etudiant() {
    printf("  | %-4s | %-18s | %-18s | %-12s | %-8s | %-8s |\n", "ID", "Nom", "Prenom", "CIN", "Chambre", "Statut");
    afficher_ligne();
}

void afficher_etudiant(Etudiant e) {
    char ch_num[15] = "---";
    if (e.chambre_id > 0) {
        CelluleChambre* ch = trouver_chambre_ptr(e.chambre_id);
        if (ch) snprintf(ch_num, 15, "%s", ch->data.numero);
    }
    printf("  | %-4d | %-18s | %-18s | %-12s | %-8s | %-8s |\n", e.id, e.nom, e.prenom, e.cin, ch_num, e.statut);
}

void ajouter_etudiant() {
    afficher_entete("AJOUTER UN ETUDIANT RESIDENT");
    CelluleEtudiant* nouv = (CelluleEtudiant*)malloc(sizeof(CelluleEtudiant));
    saisir_etudiant(&(nouv->data));
    nouv->suivant = liste_etudiants; liste_etudiants = nouv; nb_etudiants++;
    printf("\n  [OK] Etudiant ajoute (ID: %d).\n", nouv->data.id);
    pause_attente();
}

void afficher_tous_etudiants() {
    afficher_entete("LISTE DES ETUDIANTS RESIDENTS");
    if (liste_etudiants == NULL) { printf("  Aucun etudiant.\n"); pause_attente(); return; }
    afficher_entete_etudiant();
    CelluleEtudiant* temp = liste_etudiants;
    while(temp != NULL) { afficher_etudiant(temp->data); temp = temp->suivant; }
    printf("\n  Total : %d etudiant(s).\n", nb_etudiants);
    pause_attente();
}

void rechercher_etudiant() {
    afficher_entete("RECHERCHER UN ETUDIANT");
    printf("  1. Par ID\n  2. Par Nom\n  Votre choix : ");
    int choix; scanf("%d", &choix); vider_buffer();
    int trouve = 0; afficher_entete_etudiant();
    CelluleEtudiant* temp = liste_etudiants;
    if (choix == 1) {
        int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
        while(temp) { if(temp->data.id == id) { afficher_etudiant(temp->data); trouve = 1; break; } temp = temp->suivant; }
    } else if (choix == 2) {
        char nom[50]; printf("  Nom : "); scanf("%49s", nom); vider_buffer();
        while(temp) { if(strcasecmp(temp->data.nom, nom) == 0) { afficher_etudiant(temp->data); trouve = 1; } temp = temp->suivant; }
    }
    if (!trouve) printf("  Aucun etudiant trouve.\n");
    pause_attente();
}

void modifier_etudiant() {
    afficher_entete("MODIFIER UN ETUDIANT");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    CelluleEtudiant* cible = trouver_etudiant_ptr(id);
    if (cible == NULL) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int old_id = cible->data.id; int old_ch = cible->data.chambre_id;
    saisir_etudiant(&(cible->data));
    cible->data.id = old_id; cible->data.chambre_id = old_ch; id_etudiant_next--;
    printf("\n  [OK] Etudiant modifie.\n"); pause_attente();
}

void supprimer_etudiant() {
    afficher_entete("SUPPRIMER UN ETUDIANT");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    CelluleEtudiant *courant = liste_etudiants, *precedent = NULL;
    while(courant != NULL && courant->data.id != id) { precedent = courant; courant = courant->suivant; }
    if (courant == NULL) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    printf("  Supprimer %s %s ? (o/n) : ", courant->data.prenom, courant->data.nom);
    char conf; scanf("%c", &conf); vider_buffer();
    if (conf == 'o' || conf == 'O') {
        if(precedent == NULL) liste_etudiants = courant->suivant; else precedent->suivant = courant->suivant;
        free(courant); nb_etudiants--; printf("  [OK] Supprime.\n");
    }
    pause_attente();
}

void assigner_chambre() {
    afficher_entete("ASSIGNER UNE CHAMBRE");
    int eid; printf("  ID Etudiant : "); scanf("%d", &eid); vider_buffer();
    CelluleEtudiant* etud = trouver_etudiant_ptr(eid);
    if (!etud) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int cid; printf("  ID Chambre  : "); scanf("%d", &cid); vider_buffer();
    CelluleChambre* cham = trouver_chambre_ptr(cid);
    if (!cham) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    if (cham->data.nb_occupants >= cham->data.capacite) { printf("  [ERREUR] Chambre pleine.\n"); pause_attente(); return; }
    etud->data.chambre_id = cid; cham->data.nb_occupants++;
    if (cham->data.nb_occupants >= cham->data.capacite) strcpy(cham->data.statut, "Occupee");
    printf("\n  [OK] Chambre assignee.\n"); pause_attente();
}

/* ============================================================
 * MODULE 2 : CHAMBRES
 * ============================================================ */

void saisir_chambre(Chambre *c) {
    c->id = id_chambre_next++;
    printf("  Numero (ex A101): "); scanf("%9s", c->numero);        vider_buffer();
    printf("  Type            : "); scanf("%19s", c->type);         vider_buffer();
    printf("  Etage           : "); scanf("%d", &c->etage);         vider_buffer();
    printf("  Capacite        : "); scanf("%d", &c->capacite);      vider_buffer();
    printf("  Prix mensuel(DT): "); scanf("%f", &c->prix_mensuel);  vider_buffer();
    printf("  Statut          : "); scanf("%19s", c->statut);       vider_buffer();
    c->nb_occupants = 0;
}

void afficher_entete_chambre() {
    printf("  | %-4s | %-8s | %-8s | %-5s | %-5s | %-5s | %-10s | %-12s |\n", "ID","Numero","Type","Etage","Cap.","Occ.","Prix/mois","Statut");
    afficher_ligne();
}

void afficher_chambre(Chambre c) {
    printf("  | %-4d | %-8s | %-8s | %-5d | %-5d | %-5d | %-10.2f | %-12s |\n", c.id, c.numero, c.type, c.etage, c.capacite, c.nb_occupants, c.prix_mensuel, c.statut);
}

void ajouter_chambre() {
    afficher_entete("AJOUTER UNE CHAMBRE");
    CelluleChambre* nouv = (CelluleChambre*)malloc(sizeof(CelluleChambre));
    saisir_chambre(&(nouv->data));
    nouv->suivant = liste_chambres; liste_chambres = nouv; nb_chambres++;
    printf("\n  [OK] Chambre ajoutee (ID: %d).\n", nouv->data.id);
    pause_attente();
}

void afficher_toutes_chambres() {
    afficher_entete("LISTE DES CHAMBRES");
    if (liste_chambres == NULL) { printf("  Aucune chambre.\n"); pause_attente(); return; }
    afficher_entete_chambre();
    CelluleChambre* temp = liste_chambres;
    while(temp != NULL) { afficher_chambre(temp->data); temp = temp->suivant; }
    printf("\n  Total : %d chambre(s).\n", nb_chambres);
    pause_attente();
}

void rechercher_chambre() {
    afficher_entete("RECHERCHER UNE CHAMBRE");
    printf("  1. Par ID\n  2. Par Numero\n  Votre choix : ");
    int choix; scanf("%d", &choix); vider_buffer();
    int trouve = 0; afficher_entete_chambre();
    CelluleChambre* temp = liste_chambres;
    if (choix == 1) {
        int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
        while(temp) { if(temp->data.id == id) { afficher_chambre(temp->data); trouve = 1; break; } temp = temp->suivant; }
    } else if (choix == 2) {
        char num[10]; printf("  Numero : "); scanf("%9s", num); vider_buffer();
        while(temp) { if(strcasecmp(temp->data.numero, num) == 0) { afficher_chambre(temp->data); trouve = 1; } temp = temp->suivant; }
    }
    if (!trouve) printf("  Aucune chambre trouvee.\n");
    pause_attente();
}

void modifier_chambre() {
    afficher_entete("MODIFIER UNE CHAMBRE");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    CelluleChambre* cible = trouver_chambre_ptr(id);
    if (!cible) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int old_id = cible->data.id; int old_occ = cible->data.nb_occupants;
    saisir_chambre(&(cible->data));
    cible->data.id = old_id; cible->data.nb_occupants = old_occ; id_chambre_next--;
    printf("\n  [OK] Chambre modifiee.\n"); pause_attente();
}

void supprimer_chambre() {
    afficher_entete("SUPPRIMER UNE CHAMBRE");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    CelluleChambre *courant = liste_chambres, *precedent = NULL;
    while(courant != NULL && courant->data.id != id) { precedent = courant; courant = courant->suivant; }
    if (!courant) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    if (courant->data.nb_occupants > 0) { printf("  [ERREUR] Chambre occupee.\n"); pause_attente(); return; }
    printf("  Supprimer chambre %s ? (o/n) : ", courant->data.numero);
    char conf; scanf("%c", &conf); vider_buffer();
    if (conf == 'o' || conf == 'O') {
        if(!precedent) liste_chambres = courant->suivant; else precedent->suivant = courant->suivant;
        free(courant); nb_chambres--; printf("  [OK] Chambre supprimee.\n");
    }
    pause_attente();
}

/* ============================================================
 * MODULE 3 : PAIEMENTS
 * ============================================================ */

void saisir_paiement(Paiement *p) {
    p->id = id_paiement_next++;
    printf("  ID Etudiant     : "); scanf("%d", &p->etudiant_id);      vider_buffer();
    printf("  ID Chambre      : "); scanf("%d", &p->chambre_id);       vider_buffer();
    printf("  Montant (DT)    : "); scanf("%f", &p->montant);          vider_buffer();
    printf("  Date paiement   : "); scanf("%14s", p->date_paiement);   vider_buffer();
    printf("  Mois concerne   : "); scanf("%19s", p->mois);            vider_buffer();
    printf("  Mode            : "); scanf("%19s", p->mode);            vider_buffer();
    printf("  Statut          : "); scanf("%19s", p->statut);          vider_buffer();
}

void afficher_entete_paiement() {
    printf("  | %-4s | %-18s | %-8s | %-10s | %-12s | %-12s | %-10s | %-10s |\n", "ID","Etudiant","Chambre","Montant","Date","Mois","Mode","Statut");
    afficher_ligne();
}

void afficher_paiement(Paiement p) {
    char nom_et[40] = "---"; char num_ch[10] = "---";
    CelluleEtudiant* etud = trouver_etudiant_ptr(p.etudiant_id);
    CelluleChambre* cham = trouver_chambre_ptr(p.chambre_id);
    if (etud) snprintf(nom_et, 40, "%s %s", etud->data.prenom, etud->data.nom);
    if (cham) snprintf(num_ch, 10, "%s", cham->data.numero);
    printf("  | %-4d | %-18s | %-8s | %-10.2f | %-12s | %-12s | %-10s | %-10s |\n", p.id, nom_et, num_ch, p.montant, p.date_paiement, p.mois, p.mode, p.statut);
}

void ajouter_paiement() {
    afficher_entete("ENREGISTRER UN PAIEMENT");
    CellulePaiement* nouv = (CellulePaiement*)malloc(sizeof(CellulePaiement));
    saisir_paiement(&(nouv->data));
    nouv->suivant = liste_paiements; liste_paiements = nouv; nb_paiements++;
    printf("\n  [OK] Paiement enregistre (ID: %d).\n", nouv->data.id);
    pause_attente();
}

void afficher_tous_paiements() {
    afficher_entete("LISTE DES PAIEMENTS");
    if (!liste_paiements) { printf("  Aucun paiement.\n"); pause_attente(); return; }
    afficher_entete_paiement();
    CellulePaiement* temp = liste_paiements;
    while(temp) { afficher_paiement(temp->data); temp = temp->suivant; }
    printf("\n  Total : %d paiement(s).\n", nb_paiements);
    pause_attente();
}

void bilan_financier() {
    afficher_entete("BILAN FINANCIER");
    float total_paye = 0; CellulePaiement* temp = liste_paiements;
    while(temp) { if (strcasecmp(temp->data.statut, "Paye") == 0) total_paye += temp->data.montant; temp = temp->suivant; }
    printf("  Paiements enregistres   : %d\n", nb_paiements);
    printf("  Total encaisse (Paye)   : %.2f DT\n", total_paye);
    pause_attente();
}

/* ============================================================
 * MODULE 4 : RECLAMATIONS
 * ============================================================ */

void saisir_reclamation(Reclamation *r) {
    r->id = id_reclamation_next++;
    printf("  ID Etudiant            : "); scanf("%d", &r->etudiant_id);   vider_buffer();
    printf("  ID Chambre             : "); scanf("%d", &r->chambre_id);    vider_buffer();
    printf("  Date (JJ/MM/AAAA)      : "); scanf("%14s", r->date);         vider_buffer();
    printf("  Type                   : "); scanf("%39s", r->type);         vider_buffer();
    printf("  Description            : "); fgets(r->description, 200, stdin); r->description[strcspn(r->description, "\n")] = 0;
    printf("  Priorite               : "); scanf("%14s", r->priorite);     vider_buffer();
    printf("  Statut                 : "); scanf("%19s", r->statut);       vider_buffer();
}

void afficher_entete_reclamation() {
    printf("  | %-4s | %-18s | %-8s | %-12s | %-15s | %-8s | %-10s |\n", "ID","Etudiant","Chambre","Date","Type","Priorite","Statut");
    afficher_ligne();
}

void afficher_reclamation(Reclamation r) {
    printf("  | %-4d | %-18d | %-8d | %-12s | %-15s | %-8s | %-10s |\n", r.id, r.etudiant_id, r.chambre_id, r.date, r.type, r.priorite, r.statut);
}

void ajouter_reclamation() {
    afficher_entete("DEPOSER UNE RECLAMATION");
    CelluleReclamation* nouv = (CelluleReclamation*)malloc(sizeof(CelluleReclamation));
    saisir_reclamation(&(nouv->data));
    nouv->suivant = liste_reclamations; liste_reclamations = nouv; nb_reclamations++;
    printf("\n  [OK] Reclamation enregistree (ID: %d).\n", nouv->data.id);
    pause_attente();
}

void afficher_toutes_reclamations() {
    afficher_entete("LISTE DES RECLAMATIONS");
    if (!liste_reclamations) { printf("  Aucune reclamation.\n"); pause_attente(); return; }
    afficher_entete_reclamation();
    CelluleReclamation* temp = liste_reclamations;
    while(temp) { afficher_reclamation(temp->data); temp = temp->suivant; }
    pause_attente();
}

/* ============================================================
 * MENUS COMPLETS (RESTAURES A L'IDENTIQUE)
 * ============================================================ */

void menu_etudiants() {
    int choix;
    do {
        afficher_entete("MODULE - GESTION DES ETUDIANTS RESIDENTS");
        printf("    1. Ajouter un etudiant resident\n");
        printf("    2. Afficher tous les etudiants\n");
        printf("    3. Rechercher un etudiant\n");
        printf("    4. Modifier un etudiant\n");
        printf("    5. Supprimer un etudiant\n");
        printf("    6. Assigner une chambre\n");
        printf("    0. Retour\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: ajouter_etudiant(); break;
            case 2: afficher_tous_etudiants(); break;
            case 3: rechercher_etudiant(); break;
            case 4: modifier_etudiant(); break;
            case 5: supprimer_etudiant(); break;
            case 6: assigner_chambre(); break;
        }
    } while (choix != 0);
}

void menu_chambres() {
    int choix;
    do {
        afficher_entete("MODULE - GESTION DES CHAMBRES");
        printf("    1. Ajouter une chambre\n");
        printf("    2. Afficher toutes les chambres\n");
        printf("    3. Rechercher une chambre\n");
        printf("    4. Modifier une chambre\n");
        printf("    5. Supprimer une chambre\n");
        printf("    0. Retour\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: ajouter_chambre(); break;
            case 2: afficher_toutes_chambres(); break;
            case 3: rechercher_chambre(); break;
            case 4: modifier_chambre(); break;
            case 5: supprimer_chambre(); break;
        }
    } while (choix != 0);
}

void menu_paiements() {
    int choix;
    do {
        afficher_entete("MODULE - GESTION DES PAIEMENTS");
        printf("    1. Enregistrer un paiement\n");
        printf("    2. Afficher tous les paiements\n");
        printf("    3. Bilan financier\n");
        printf("    0. Retour\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: ajouter_paiement(); break;
            case 2: afficher_tous_paiements(); break;
            case 3: bilan_financier(); break;
        }
    } while (choix != 0);
}

void menu_reclamations() {
    int choix;
    do {
        afficher_entete("MODULE - GESTION DES RECLAMATIONS");
        printf("    1. Deposer une reclamation\n");
        printf("    2. Afficher toutes les reclamations\n");
        printf("    0. Retour\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: ajouter_reclamation(); break;
            case 2: afficher_toutes_reclamations(); break;
        }
    } while (choix != 0);
}

void menu_principal() {
    int choix;
    do {
        afficher_entete("MENU PRINCIPAL - GESTION DE FOYER UNIVERSITAIRE");
        printf("    1. Gestion des Etudiants\n");
        printf("    2. Gestion des Chambres\n");
        printf("    3. Gestion des Paiements\n");
        printf("    4. Gestion des Reclamations\n");
        printf("    0. Quitter et Sauvegarder\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: menu_etudiants(); break;
            case 2: menu_chambres(); break;
            case 3: menu_paiements(); break;
            case 4: menu_reclamations(); break;
            case 0:
                sauvegarder_donnees();
                printf("\n  Au revoir !\n\n");
                break;
            default: printf("  [ERREUR] Choix invalide.\n"); pause_attente();
        }
    } while (choix != 0);
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main() {
    charger_donnees_fichiers();
    if (!authentifier()) {
        printf("\n  Acces refuse. Fermeture du programme.\n\n");
        return 1;
    }
    menu_principal();
    return 0;
}
