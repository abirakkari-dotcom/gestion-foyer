/*
 * ============================================================
 *   APPLICATION DE GESTION DE FOYER UNIVERSITAIRE
 *   Projet ASD2 - 1LIG - Annee academique 2025-2026
 *   Compte Rendu 3 : Interface terminal + Tableaux de structures
 * ============================================================
 *
 *  MODULES :
 *    1. Gestion des Etudiants (residents)
 *    2. Gestion des Chambres
 *    3. Gestion des Paiements
 *    4. Gestion des Reclamations
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 *   CONSTANTES
 * ============================================================ */
#define MAX_ETUDIANTS    100
#define MAX_CHAMBRES      80
#define MAX_PAIEMENTS    300
#define MAX_RECLAMATIONS 200

#define LOGIN_ADMIN  "admin"
#define MDP_ADMIN    "1234"

/* ============================================================
 *   STRUCTURES DE DONNEES
 * ============================================================ */

typedef struct {
    int   id;
    char  nom[50];
    char  prenom[50];
    char  cin[15];               /* Carte d'identite nationale */
    char  date_naissance[15];    /* JJ/MM/AAAA */
    char  email[60];
    char  telephone[15];
    int   chambre_id;            /* 0 = pas de chambre assignee */
    char  statut[20];            /* Actif / Inactif */
} Etudiant;

typedef struct {
    int   id;
    char  numero[10];            /* ex: A101, B205 */
    char  type[20];              /* Simple / Double / Triple */
    int   etage;
    float prix_mensuel;
    int   capacite;
    int   nb_occupants;
    char  statut[20];            /* Disponible / Occupee / Maintenance */
} Chambre;

typedef struct {
    int   id;
    int   etudiant_id;
    int   chambre_id;
    float montant;
    char  date_paiement[15];     /* JJ/MM/AAAA */
    char  mois[20];              /* Janvier, Fevrier ... */
    char  mode[20];              /* Especes / Virement / Cheque */
    char  statut[20];            /* Paye / En_attente / Retard */
} Paiement;

typedef struct {
    int   id;
    int   etudiant_id;
    int   chambre_id;
    char  date[15];              /* JJ/MM/AAAA */
    char  type[40];              /* Plomberie / Electricite / Bruit / Autre */
    char  description[200];
    char  priorite[15];          /* Haute / Moyenne / Basse */
    char  statut[20];            /* Ouverte / En_cours / Resolue / Fermee */
} Reclamation;

/* ============================================================
 *   TABLEAUX GLOBAUX
 * ============================================================ */
Etudiant    tab_etudiants[MAX_ETUDIANTS];
Chambre     tab_chambres[MAX_CHAMBRES];
Paiement    tab_paiements[MAX_PAIEMENTS];
Reclamation tab_reclamations[MAX_RECLAMATIONS];

int nb_etudiants    = 0;
int nb_chambres     = 0;
int nb_paiements    = 0;
int nb_reclamations = 0;

int id_etudiant_next    = 1;
int id_chambre_next     = 1;
int id_paiement_next    = 1;
int id_reclamation_next = 1;

/* ============================================================
 *   UTILITAIRES
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
 *   AUTHENTIFICATION
 * ============================================================ */

int authentifier() {
    char login[30], mdp[30];
    int tentatives = 3;

    afficher_entete("CONNEXION - GESTION DE FOYER UNIVERSITAIRE");
    printf("  Systeme de gestion des residents\n");
    afficher_ligne();

    while (tentatives > 0) {
        printf("\n  Login        : "); scanf("%29s", login); vider_buffer();
        printf("  Mot de passe : "); scanf("%29s", mdp);   vider_buffer();

        if (strcmp(login, LOGIN_ADMIN) == 0 && strcmp(mdp, MDP_ADMIN) == 0) {
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
 *   MODULE 1 : ETUDIANTS (RESIDENTS)
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
    printf("  | %-4s | %-18s | %-18s | %-12s | %-8s | %-8s |\n",
           "ID", "Nom", "Prenom", "CIN", "Chambre", "Statut");
    afficher_ligne();
}

void afficher_etudiant(Etudiant e) {
    char ch_num[15] = "---";
    if (e.chambre_id > 0) {
        for (int i = 0; i < nb_chambres; i++)
            if (tab_chambres[i].id == e.chambre_id)
                { snprintf(ch_num, 15, "%s", tab_chambres[i].numero); break; }
    }
    printf("  | %-4d | %-18s | %-18s | %-12s | %-8s | %-8s |\n",
           e.id, e.nom, e.prenom, e.cin, ch_num, e.statut);
}

int trouver_etudiant(int id) {
    for (int i = 0; i < nb_etudiants; i++)
        if (tab_etudiants[i].id == id) return i;
    return -1;
}

void ajouter_etudiant() {
    afficher_entete("AJOUTER UN ETUDIANT RESIDENT");
    if (nb_etudiants >= MAX_ETUDIANTS) { printf("  [ERREUR] Capacite max.\n"); pause_attente(); return; }
    saisir_etudiant(&tab_etudiants[nb_etudiants]);
    nb_etudiants++;
    printf("\n  [OK] Etudiant ajoute (ID: %d).\n", tab_etudiants[nb_etudiants-1].id);
    pause_attente();
}

void afficher_tous_etudiants() {
    afficher_entete("LISTE DES ETUDIANTS RESIDENTS");
    if (nb_etudiants == 0) { printf("  Aucun etudiant.\n"); pause_attente(); return; }
    afficher_entete_etudiant();
    for (int i = 0; i < nb_etudiants; i++) afficher_etudiant(tab_etudiants[i]);
    printf("\n  Total : %d etudiant(s).\n", nb_etudiants);
    pause_attente();
}

void rechercher_etudiant() {
    afficher_entete("RECHERCHER UN ETUDIANT");
    printf("  1. Par ID\n  2. Par Nom\n  3. Par CIN\n  4. Par Statut\n  Votre choix : ");
    int choix; scanf("%d", &choix); vider_buffer();
    int trouve = 0;
    afficher_entete_etudiant();

    if (choix == 1) {
        int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
        int idx = trouver_etudiant(id);
        if (idx != -1) { afficher_etudiant(tab_etudiants[idx]); trouve = 1; }
    } else if (choix == 2) {
        char nom[50]; printf("  Nom : "); scanf("%49s", nom); vider_buffer();
        for (int i = 0; i < nb_etudiants; i++)
            if (strcasecmp(tab_etudiants[i].nom, nom) == 0) { afficher_etudiant(tab_etudiants[i]); trouve = 1; }
    } else if (choix == 3) {
        char cin[15]; printf("  CIN : "); scanf("%14s", cin); vider_buffer();
        for (int i = 0; i < nb_etudiants; i++)
            if (strcasecmp(tab_etudiants[i].cin, cin) == 0) { afficher_etudiant(tab_etudiants[i]); trouve = 1; }
    } else if (choix == 4) {
        char st[20]; printf("  Statut (Actif/Inactif) : "); scanf("%19s", st); vider_buffer();
        for (int i = 0; i < nb_etudiants; i++)
            if (strcasecmp(tab_etudiants[i].statut, st) == 0) { afficher_etudiant(tab_etudiants[i]); trouve = 1; }
    }
    if (!trouve) printf("  Aucun etudiant trouve.\n");
    pause_attente();
}

void modifier_etudiant() {
    afficher_entete("MODIFIER UN ETUDIANT");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_etudiant(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int old_id = tab_etudiants[idx].id;
    int old_ch = tab_etudiants[idx].chambre_id;
    saisir_etudiant(&tab_etudiants[idx]);
    tab_etudiants[idx].id = old_id;
    tab_etudiants[idx].chambre_id = old_ch;
    id_etudiant_next--;
    printf("\n  [OK] Etudiant modifie.\n"); pause_attente();
}

void supprimer_etudiant() {
    afficher_entete("SUPPRIMER UN ETUDIANT");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_etudiant(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    printf("  Supprimer %s %s ? (o/n) : ", tab_etudiants[idx].prenom, tab_etudiants[idx].nom);
    char conf; scanf("%c", &conf); vider_buffer();
    if (conf == 'o' || conf == 'O') {
        for (int i = idx; i < nb_etudiants - 1; i++) tab_etudiants[i] = tab_etudiants[i+1];
        nb_etudiants--;
        printf("  [OK] Supprime.\n");
    } else printf("  Annule.\n");
    pause_attente();
}

void assigner_chambre() {
    afficher_entete("ASSIGNER UNE CHAMBRE A UN ETUDIANT");
    int eid; printf("  ID Etudiant : "); scanf("%d", &eid); vider_buffer();
    int eidx = trouver_etudiant(eid);
    if (eidx == -1) { printf("  [ERREUR] Etudiant introuvable.\n"); pause_attente(); return; }

    int cid; printf("  ID Chambre  : "); scanf("%d", &cid); vider_buffer();
    int cidx = -1;
    for (int i = 0; i < nb_chambres; i++)
        if (tab_chambres[i].id == cid) { cidx = i; break; }
    if (cidx == -1) { printf("  [ERREUR] Chambre introuvable.\n"); pause_attente(); return; }

    if (tab_chambres[cidx].nb_occupants >= tab_chambres[cidx].capacite) {
        printf("  [ERREUR] Chambre pleine (capacite : %d).\n", tab_chambres[cidx].capacite);
        pause_attente(); return;
    }
    tab_etudiants[eidx].chambre_id = cid;
    tab_chambres[cidx].nb_occupants++;
    if (tab_chambres[cidx].nb_occupants >= tab_chambres[cidx].capacite)
        strcpy(tab_chambres[cidx].statut, "Occupee");
    printf("\n  [OK] Chambre %s assignee a %s %s.\n",
           tab_chambres[cidx].numero,
           tab_etudiants[eidx].prenom, tab_etudiants[eidx].nom);
    pause_attente();
}

/* ============================================================
 *   MODULE 2 : CHAMBRES
 * ============================================================ */

void saisir_chambre(Chambre *c) {
    c->id = id_chambre_next++;
    printf("  Numero (ex A101): "); scanf("%9s", c->numero);        vider_buffer();
    printf("  Type (Simple/Double/Triple): "); scanf("%19s", c->type); vider_buffer();
    printf("  Etage           : "); scanf("%d", &c->etage);          vider_buffer();
    printf("  Capacite        : "); scanf("%d", &c->capacite);       vider_buffer();
    printf("  Prix mensuel(DT): "); scanf("%f", &c->prix_mensuel);   vider_buffer();
    printf("  Statut (Disponible/Occupee/Maintenance): "); scanf("%19s", c->statut); vider_buffer();
    c->nb_occupants = 0;
}

void afficher_entete_chambre() {
    printf("  | %-4s | %-8s | %-8s | %-5s | %-5s | %-5s | %-10s | %-12s |\n",
           "ID","Numero","Type","Etage","Cap.","Occ.","Prix/mois","Statut");
    afficher_ligne();
}

void afficher_chambre(Chambre c) {
    printf("  | %-4d | %-8s | %-8s | %-5d | %-5d | %-5d | %-10.2f | %-12s |\n",
           c.id, c.numero, c.type, c.etage, c.capacite, c.nb_occupants, c.prix_mensuel, c.statut);
}

int trouver_chambre(int id) {
    for (int i = 0; i < nb_chambres; i++)
        if (tab_chambres[i].id == id) return i;
    return -1;
}

void ajouter_chambre() {
    afficher_entete("AJOUTER UNE CHAMBRE");
    if (nb_chambres >= MAX_CHAMBRES) { printf("  [ERREUR] Capacite max.\n"); pause_attente(); return; }
    saisir_chambre(&tab_chambres[nb_chambres]);
    nb_chambres++;
    printf("\n  [OK] Chambre ajoutee (ID: %d).\n", tab_chambres[nb_chambres-1].id);
    pause_attente();
}

void afficher_toutes_chambres() {
    afficher_entete("LISTE DES CHAMBRES");
    if (nb_chambres == 0) { printf("  Aucune chambre.\n"); pause_attente(); return; }
    afficher_entete_chambre();
    for (int i = 0; i < nb_chambres; i++) afficher_chambre(tab_chambres[i]);
    printf("\n  Total : %d chambre(s).\n", nb_chambres);
    pause_attente();
}

void rechercher_chambre() {
    afficher_entete("RECHERCHER UNE CHAMBRE");
    printf("  1. Par ID\n  2. Par Numero\n  3. Par Type\n  4. Par Statut\n  5. Chambres disponibles\n  Votre choix : ");
    int choix; scanf("%d", &choix); vider_buffer();
    int trouve = 0;
    afficher_entete_chambre();

    if (choix == 1) {
        int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
        int idx = trouver_chambre(id);
        if (idx != -1) { afficher_chambre(tab_chambres[idx]); trouve = 1; }
    } else if (choix == 2) {
        char num[10]; printf("  Numero : "); scanf("%9s", num); vider_buffer();
        for (int i = 0; i < nb_chambres; i++)
            if (strcasecmp(tab_chambres[i].numero, num) == 0) { afficher_chambre(tab_chambres[i]); trouve = 1; }
    } else if (choix == 3) {
        char type[20]; printf("  Type : "); scanf("%19s", type); vider_buffer();
        for (int i = 0; i < nb_chambres; i++)
            if (strcasecmp(tab_chambres[i].type, type) == 0) { afficher_chambre(tab_chambres[i]); trouve = 1; }
    } else if (choix == 4) {
        char st[20]; printf("  Statut : "); scanf("%19s", st); vider_buffer();
        for (int i = 0; i < nb_chambres; i++)
            if (strcasecmp(tab_chambres[i].statut, st) == 0) { afficher_chambre(tab_chambres[i]); trouve = 1; }
    } else if (choix == 5) {
        for (int i = 0; i < nb_chambres; i++)
            if (strcasecmp(tab_chambres[i].statut, "Disponible") == 0) { afficher_chambre(tab_chambres[i]); trouve = 1; }
    }
    if (!trouve) printf("  Aucune chambre trouvee.\n");
    pause_attente();
}

void modifier_chambre() {
    afficher_entete("MODIFIER UNE CHAMBRE");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_chambre(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int old_id = tab_chambres[idx].id;
    int old_occ = tab_chambres[idx].nb_occupants;
    saisir_chambre(&tab_chambres[idx]);
    tab_chambres[idx].id = old_id;
    tab_chambres[idx].nb_occupants = old_occ;
    id_chambre_next--;
    printf("\n  [OK] Chambre modifiee.\n"); pause_attente();
}

void supprimer_chambre() {
    afficher_entete("SUPPRIMER UNE CHAMBRE");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_chambre(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    if (tab_chambres[idx].nb_occupants > 0) {
        printf("  [ERREUR] Chambre occupee (%d resident(s)), impossible de supprimer.\n", tab_chambres[idx].nb_occupants);
        pause_attente(); return;
    }
    printf("  Supprimer chambre %s ? (o/n) : ", tab_chambres[idx].numero);
    char conf; scanf("%c", &conf); vider_buffer();
    if (conf == 'o' || conf == 'O') {
        for (int i = idx; i < nb_chambres - 1; i++) tab_chambres[i] = tab_chambres[i+1];
        nb_chambres--;
        printf("  [OK] Chambre supprimee.\n");
    } else printf("  Annule.\n");
    pause_attente();
}

/* ============================================================
 *   MODULE 3 : PAIEMENTS
 * ============================================================ */

void saisir_paiement(Paiement *p) {
    p->id = id_paiement_next++;
    printf("  ID Etudiant     : "); scanf("%d", &p->etudiant_id);      vider_buffer();
    printf("  ID Chambre      : "); scanf("%d", &p->chambre_id);       vider_buffer();
    printf("  Montant (DT)    : "); scanf("%f", &p->montant);          vider_buffer();
    printf("  Date paiement   : "); scanf("%14s", p->date_paiement);   vider_buffer();
    printf("  Mois concerne   : "); scanf("%19s", p->mois);            vider_buffer();
    printf("  Mode (Especes/Virement/Cheque): "); scanf("%19s", p->mode); vider_buffer();
    printf("  Statut (Paye/En_attente/Retard): "); scanf("%19s", p->statut); vider_buffer();
}

void afficher_entete_paiement() {
    printf("  | %-4s | %-18s | %-8s | %-10s | %-12s | %-12s | %-10s | %-10s |\n",
           "ID","Etudiant","Chambre","Montant","Date","Mois","Mode","Statut");
    afficher_ligne();
}

void afficher_paiement(Paiement p) {
    char nom_et[40] = "---"; char num_ch[10] = "---";
    int ie = trouver_etudiant(p.etudiant_id);
    int ic = trouver_chambre(p.chambre_id);
    if (ie != -1) snprintf(nom_et, 40, "%s %s", tab_etudiants[ie].prenom, tab_etudiants[ie].nom);
    if (ic != -1) snprintf(num_ch, 10, "%s", tab_chambres[ic].numero);
    printf("  | %-4d | %-18s | %-8s | %-10.2f | %-12s | %-12s | %-10s | %-10s |\n",
           p.id, nom_et, num_ch, p.montant, p.date_paiement, p.mois, p.mode, p.statut);
}

int trouver_paiement(int id) {
    for (int i = 0; i < nb_paiements; i++)
        if (tab_paiements[i].id == id) return i;
    return -1;
}

void ajouter_paiement() {
    afficher_entete("ENREGISTRER UN PAIEMENT");
    if (nb_paiements >= MAX_PAIEMENTS) { printf("  [ERREUR] Capacite max.\n"); pause_attente(); return; }
    saisir_paiement(&tab_paiements[nb_paiements]);
    nb_paiements++;
    printf("\n  [OK] Paiement enregistre (ID: %d).\n", tab_paiements[nb_paiements-1].id);
    pause_attente();
}

void afficher_tous_paiements() {
    afficher_entete("LISTE DES PAIEMENTS");
    if (nb_paiements == 0) { printf("  Aucun paiement.\n"); pause_attente(); return; }
    afficher_entete_paiement();
    for (int i = 0; i < nb_paiements; i++) afficher_paiement(tab_paiements[i]);
    printf("\n  Total : %d paiement(s).\n", nb_paiements);
    pause_attente();
}

void rechercher_paiement() {
    afficher_entete("RECHERCHER UN PAIEMENT");
    printf("  1. Par ID Etudiant\n  2. Par Mois\n  3. Par Statut\n  4. Par Mode de paiement\n  Votre choix : ");
    int choix; scanf("%d", &choix); vider_buffer();
    int trouve = 0;
    afficher_entete_paiement();

    if (choix == 1) {
        int id; printf("  ID Etudiant : "); scanf("%d", &id); vider_buffer();
        for (int i = 0; i < nb_paiements; i++)
            if (tab_paiements[i].etudiant_id == id) { afficher_paiement(tab_paiements[i]); trouve = 1; }
    } else if (choix == 2) {
        char mois[20]; printf("  Mois : "); scanf("%19s", mois); vider_buffer();
        for (int i = 0; i < nb_paiements; i++)
            if (strcasecmp(tab_paiements[i].mois, mois) == 0) { afficher_paiement(tab_paiements[i]); trouve = 1; }
    } else if (choix == 3) {
        char st[20]; printf("  Statut (Paye/En_attente/Retard) : "); scanf("%19s", st); vider_buffer();
        for (int i = 0; i < nb_paiements; i++)
            if (strcasecmp(tab_paiements[i].statut, st) == 0) { afficher_paiement(tab_paiements[i]); trouve = 1; }
    } else if (choix == 4) {
        char mode[20]; printf("  Mode : "); scanf("%19s", mode); vider_buffer();
        for (int i = 0; i < nb_paiements; i++)
            if (strcasecmp(tab_paiements[i].mode, mode) == 0) { afficher_paiement(tab_paiements[i]); trouve = 1; }
    }
    if (!trouve) printf("  Aucun paiement trouve.\n");
    pause_attente();
}

void bilan_financier() {
    afficher_entete("BILAN FINANCIER");
    float total_paye = 0, total_attente = 0, total_retard = 0;
    for (int i = 0; i < nb_paiements; i++) {
        if (strcasecmp(tab_paiements[i].statut, "Paye") == 0)         total_paye    += tab_paiements[i].montant;
        else if (strcasecmp(tab_paiements[i].statut, "En_attente") == 0) total_attente += tab_paiements[i].montant;
        else if (strcasecmp(tab_paiements[i].statut, "Retard") == 0)  total_retard  += tab_paiements[i].montant;
    }
    printf("  Paiements enregistres   : %d\n", nb_paiements);
    printf("  Total encaisse (Paye)   : %.2f DT\n", total_paye);
    printf("  En attente              : %.2f DT\n", total_attente);
    printf("  En retard               : %.2f DT\n", total_retard);
    printf("  TOTAL GLOBAL            : %.2f DT\n", total_paye + total_attente + total_retard);
    pause_attente();
}

void modifier_paiement() {
    afficher_entete("MODIFIER UN PAIEMENT");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_paiement(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int old_id = tab_paiements[idx].id;
    saisir_paiement(&tab_paiements[idx]);
    tab_paiements[idx].id = old_id;
    id_paiement_next--;
    printf("\n  [OK] Paiement modifie.\n"); pause_attente();
}

void supprimer_paiement() {
    afficher_entete("SUPPRIMER UN PAIEMENT");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_paiement(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    printf("  Supprimer paiement ID %d ? (o/n) : ", id);
    char conf; scanf("%c", &conf); vider_buffer();
    if (conf == 'o' || conf == 'O') {
        for (int i = idx; i < nb_paiements - 1; i++) tab_paiements[i] = tab_paiements[i+1];
        nb_paiements--;
        printf("  [OK] Supprime.\n");
    } else printf("  Annule.\n");
    pause_attente();
}

/* ============================================================
 *   MODULE 4 : RECLAMATIONS
 * ============================================================ */

void saisir_reclamation(Reclamation *r) {
    r->id = id_reclamation_next++;
    printf("  ID Etudiant            : "); scanf("%d", &r->etudiant_id);   vider_buffer();
    printf("  ID Chambre concernee   : "); scanf("%d", &r->chambre_id);    vider_buffer();
    printf("  Date (JJ/MM/AAAA)      : "); scanf("%14s", r->date);         vider_buffer();
    printf("  Type (Plomberie/Electricite/Bruit/Autre): "); scanf("%39s", r->type); vider_buffer();
    printf("  Description            : ");
    fgets(r->description, 200, stdin);
    r->description[strcspn(r->description, "\n")] = 0;
    printf("  Priorite (Haute/Moyenne/Basse): "); scanf("%14s", r->priorite); vider_buffer();
    printf("  Statut (Ouverte/En_cours/Resolue/Fermee): "); scanf("%19s", r->statut); vider_buffer();
}

void afficher_entete_reclamation() {
    printf("  | %-4s | %-18s | %-8s | %-12s | %-15s | %-8s | %-10s |\n",
           "ID","Etudiant","Chambre","Date","Type","Priorite","Statut");
    afficher_ligne();
}

void afficher_reclamation(Reclamation r) {
    char nom_et[40] = "---"; char num_ch[10] = "---";
    int ie = trouver_etudiant(r.etudiant_id);
    int ic = trouver_chambre(r.chambre_id);
    if (ie != -1) snprintf(nom_et, 40, "%s %s", tab_etudiants[ie].prenom, tab_etudiants[ie].nom);
    if (ic != -1) snprintf(num_ch, 10, "%s", tab_chambres[ic].numero);
    printf("  | %-4d | %-18s | %-8s | %-12s | %-15s | %-8s | %-10s |\n",
           r.id, nom_et, num_ch, r.date, r.type, r.priorite, r.statut);
}

int trouver_reclamation(int id) {
    for (int i = 0; i < nb_reclamations; i++)
        if (tab_reclamations[i].id == id) return i;
    return -1;
}

void ajouter_reclamation() {
    afficher_entete("DEPOSER UNE RECLAMATION");
    if (nb_reclamations >= MAX_RECLAMATIONS) { printf("  [ERREUR] Capacite max.\n"); pause_attente(); return; }
    saisir_reclamation(&tab_reclamations[nb_reclamations]);
    nb_reclamations++;
    printf("\n  [OK] Reclamation enregistree (ID: %d).\n", tab_reclamations[nb_reclamations-1].id);
    pause_attente();
}

void afficher_toutes_reclamations() {
    afficher_entete("LISTE DES RECLAMATIONS");
    if (nb_reclamations == 0) { printf("  Aucune reclamation.\n"); pause_attente(); return; }
    afficher_entete_reclamation();
    for (int i = 0; i < nb_reclamations; i++) afficher_reclamation(tab_reclamations[i]);
    printf("\n  Total : %d reclamation(s).\n", nb_reclamations);
    pause_attente();
}

void rechercher_reclamation() {
    afficher_entete("RECHERCHER UNE RECLAMATION");
    printf("  1. Par ID Etudiant\n  2. Par Type\n  3. Par Priorite\n  4. Par Statut\n  Votre choix : ");
    int choix; scanf("%d", &choix); vider_buffer();
    int trouve = 0;
    afficher_entete_reclamation();

    if (choix == 1) {
        int id; printf("  ID Etudiant : "); scanf("%d", &id); vider_buffer();
        for (int i = 0; i < nb_reclamations; i++)
            if (tab_reclamations[i].etudiant_id == id) { afficher_reclamation(tab_reclamations[i]); trouve = 1; }
    } else if (choix == 2) {
        char type[40]; printf("  Type : "); scanf("%39s", type); vider_buffer();
        for (int i = 0; i < nb_reclamations; i++)
            if (strcasecmp(tab_reclamations[i].type, type) == 0) { afficher_reclamation(tab_reclamations[i]); trouve = 1; }
    } else if (choix == 3) {
        char prio[15]; printf("  Priorite (Haute/Moyenne/Basse) : "); scanf("%14s", prio); vider_buffer();
        for (int i = 0; i < nb_reclamations; i++)
            if (strcasecmp(tab_reclamations[i].priorite, prio) == 0) { afficher_reclamation(tab_reclamations[i]); trouve = 1; }
    } else if (choix == 4) {
        char st[20]; printf("  Statut (Ouverte/En_cours/Resolue/Fermee) : "); scanf("%19s", st); vider_buffer();
        for (int i = 0; i < nb_reclamations; i++)
            if (strcasecmp(tab_reclamations[i].statut, st) == 0) { afficher_reclamation(tab_reclamations[i]); trouve = 1; }
    }
    if (!trouve) printf("  Aucune reclamation trouvee.\n");
    pause_attente();
}

void changer_statut_reclamation() {
    afficher_entete("METTRE A JOUR LE STATUT D'UNE RECLAMATION");
    int id; printf("  ID Reclamation : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_reclamation(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    printf("  Statut actuel : %s\n", tab_reclamations[idx].statut);
    printf("  Nouveau statut (Ouverte/En_cours/Resolue/Fermee) : ");
    scanf("%19s", tab_reclamations[idx].statut); vider_buffer();
    printf("  [OK] Statut mis a jour.\n"); pause_attente();
}

void modifier_reclamation() {
    afficher_entete("MODIFIER UNE RECLAMATION");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_reclamation(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    int old_id = tab_reclamations[idx].id;
    saisir_reclamation(&tab_reclamations[idx]);
    tab_reclamations[idx].id = old_id;
    id_reclamation_next--;
    printf("\n  [OK] Reclamation modifiee.\n"); pause_attente();
}

void supprimer_reclamation() {
    afficher_entete("SUPPRIMER UNE RECLAMATION");
    int id; printf("  ID : "); scanf("%d", &id); vider_buffer();
    int idx = trouver_reclamation(id);
    if (idx == -1) { printf("  [ERREUR] Introuvable.\n"); pause_attente(); return; }
    printf("  Supprimer reclamation ID %d ? (o/n) : ", id);
    char conf; scanf("%c", &conf); vider_buffer();
    if (conf == 'o' || conf == 'O') {
        for (int i = idx; i < nb_reclamations - 1; i++) tab_reclamations[i] = tab_reclamations[i+1];
        nb_reclamations--;
        printf("  [OK] Supprimee.\n");
    } else printf("  Annule.\n");
    pause_attente();
}

/* ============================================================
 *   DONNEES DE DEMONSTRATION
 * ============================================================ */

void charger_donnees_demo() {
    /* Chambres */
    strcpy(tab_chambres[0].numero, "A101"); strcpy(tab_chambres[0].type, "Simple");
    tab_chambres[0].etage = 1; tab_chambres[0].capacite = 1; tab_chambres[0].nb_occupants = 1;
    tab_chambres[0].prix_mensuel = 250.0f; strcpy(tab_chambres[0].statut, "Occupee");
    tab_chambres[0].id = id_chambre_next++;

    strcpy(tab_chambres[1].numero, "A102"); strcpy(tab_chambres[1].type, "Double");
    tab_chambres[1].etage = 1; tab_chambres[1].capacite = 2; tab_chambres[1].nb_occupants = 1;
    tab_chambres[1].prix_mensuel = 180.0f; strcpy(tab_chambres[1].statut, "Disponible");
    tab_chambres[1].id = id_chambre_next++;

    strcpy(tab_chambres[2].numero, "B201"); strcpy(tab_chambres[2].type, "Triple");
    tab_chambres[2].etage = 2; tab_chambres[2].capacite = 3; tab_chambres[2].nb_occupants = 2;
    tab_chambres[2].prix_mensuel = 140.0f; strcpy(tab_chambres[2].statut, "Disponible");
    tab_chambres[2].id = id_chambre_next++;

    strcpy(tab_chambres[3].numero, "B202"); strcpy(tab_chambres[3].type, "Simple");
    tab_chambres[3].etage = 2; tab_chambres[3].capacite = 1; tab_chambres[3].nb_occupants = 0;
    tab_chambres[3].prix_mensuel = 250.0f; strcpy(tab_chambres[3].statut, "Maintenance");
    tab_chambres[3].id = id_chambre_next++;
    nb_chambres = 4;

    /* Etudiants */
    strcpy(tab_etudiants[0].nom, "Mansouri");  strcpy(tab_etudiants[0].prenom, "Amine");
    strcpy(tab_etudiants[0].cin, "12345678");  strcpy(tab_etudiants[0].date_naissance, "10/05/2004");
    strcpy(tab_etudiants[0].email, "amine.mansouri@email.tn"); strcpy(tab_etudiants[0].telephone, "55001122");
    tab_etudiants[0].chambre_id = 1; strcpy(tab_etudiants[0].statut, "Actif");
    tab_etudiants[0].id = id_etudiant_next++;

    strcpy(tab_etudiants[1].nom, "Gharbi");    strcpy(tab_etudiants[1].prenom, "Nour");
    strcpy(tab_etudiants[1].cin, "87654321");  strcpy(tab_etudiants[1].date_naissance, "22/09/2005");
    strcpy(tab_etudiants[1].email, "nour.gharbi@email.tn"); strcpy(tab_etudiants[1].telephone, "55334455");
    tab_etudiants[1].chambre_id = 2; strcpy(tab_etudiants[1].statut, "Actif");
    tab_etudiants[1].id = id_etudiant_next++;

    strcpy(tab_etudiants[2].nom, "Bouazizi");  strcpy(tab_etudiants[2].prenom, "Sami");
    strcpy(tab_etudiants[2].cin, "11223344");  strcpy(tab_etudiants[2].date_naissance, "03/03/2004");
    strcpy(tab_etudiants[2].email, "sami.bouazizi@email.tn"); strcpy(tab_etudiants[2].telephone, "50667788");
    tab_etudiants[2].chambre_id = 3; strcpy(tab_etudiants[2].statut, "Actif");
    tab_etudiants[2].id = id_etudiant_next++;
    nb_etudiants = 3;

    /* Paiements */
    tab_paiements[0].id = id_paiement_next++; tab_paiements[0].etudiant_id = 1; tab_paiements[0].chambre_id = 1;
    tab_paiements[0].montant = 250.0f; strcpy(tab_paiements[0].date_paiement, "01/04/2026");
    strcpy(tab_paiements[0].mois, "Avril"); strcpy(tab_paiements[0].mode, "Virement"); strcpy(tab_paiements[0].statut, "Paye");

    tab_paiements[1].id = id_paiement_next++; tab_paiements[1].etudiant_id = 2; tab_paiements[1].chambre_id = 2;
    tab_paiements[1].montant = 180.0f; strcpy(tab_paiements[1].date_paiement, "05/04/2026");
    strcpy(tab_paiements[1].mois, "Avril"); strcpy(tab_paiements[1].mode, "Especes"); strcpy(tab_paiements[1].statut, "En_attente");

    tab_paiements[2].id = id_paiement_next++; tab_paiements[2].etudiant_id = 3; tab_paiements[2].chambre_id = 3;
    tab_paiements[2].montant = 140.0f; strcpy(tab_paiements[2].date_paiement, "10/03/2026");
    strcpy(tab_paiements[2].mois, "Mars"); strcpy(tab_paiements[2].mode, "Cheque"); strcpy(tab_paiements[2].statut, "Retard");
    nb_paiements = 3;

    /* Reclamations */
    tab_reclamations[0].id = id_reclamation_next++; tab_reclamations[0].etudiant_id = 1; tab_reclamations[0].chambre_id = 1;
    strcpy(tab_reclamations[0].date, "02/04/2026"); strcpy(tab_reclamations[0].type, "Plomberie");
    strcpy(tab_reclamations[0].description, "Fuite d'eau sous le lavabo");
    strcpy(tab_reclamations[0].priorite, "Haute"); strcpy(tab_reclamations[0].statut, "En_cours");

    tab_reclamations[1].id = id_reclamation_next++; tab_reclamations[1].etudiant_id = 2; tab_reclamations[1].chambre_id = 2;
    strcpy(tab_reclamations[1].date, "03/04/2026"); strcpy(tab_reclamations[1].type, "Bruit");
    strcpy(tab_reclamations[1].description, "Bruit excessif la nuit dans le couloir");
    strcpy(tab_reclamations[1].priorite, "Moyenne"); strcpy(tab_reclamations[1].statut, "Ouverte");

    tab_reclamations[2].id = id_reclamation_next++; tab_reclamations[2].etudiant_id = 3; tab_reclamations[2].chambre_id = 3;
    strcpy(tab_reclamations[2].date, "28/03/2026"); strcpy(tab_reclamations[2].type, "Electricite");
    strcpy(tab_reclamations[2].description, "Prise electrique defaillante");
    strcpy(tab_reclamations[2].priorite, "Haute"); strcpy(tab_reclamations[2].statut, "Resolue");
    nb_reclamations = 3;
}

/* ============================================================
 *   TABLEAU DE BORD
 * ============================================================ */

void tableau_de_bord() {
    afficher_entete("TABLEAU DE BORD - FOYER UNIVERSITAIRE");

    /* Taux d'occupation */
    int total_cap = 0, total_occ = 0;
    for (int i = 0; i < nb_chambres; i++) {
        total_cap += tab_chambres[i].capacite;
        total_occ += tab_chambres[i].nb_occupants;
    }
    float taux = (total_cap > 0) ? (float)total_occ / total_cap * 100 : 0;

    printf("  --- Residents ---\n");
    printf("  Etudiants inscrits       : %d / %d\n", nb_etudiants, MAX_ETUDIANTS);

    printf("\n  --- Chambres ---\n");
    printf("  Chambres enregistrees    : %d / %d\n", nb_chambres, MAX_CHAMBRES);
    printf("  Taux d'occupation        : %.1f%% (%d / %d places)\n", taux, total_occ, total_cap);

    int dispo = 0, maint = 0;
    for (int i = 0; i < nb_chambres; i++) {
        if (strcasecmp(tab_chambres[i].statut, "Disponible") == 0)  dispo++;
        if (strcasecmp(tab_chambres[i].statut, "Maintenance") == 0) maint++;
    }
    printf("  Chambres disponibles     : %d\n", dispo);
    printf("  En maintenance           : %d\n", maint);

    printf("\n  --- Paiements ---\n");
    float paye = 0, attente = 0, retard = 0;
    for (int i = 0; i < nb_paiements; i++) {
        if (strcasecmp(tab_paiements[i].statut, "Paye") == 0)         paye    += tab_paiements[i].montant;
        else if (strcasecmp(tab_paiements[i].statut, "En_attente") == 0) attente += tab_paiements[i].montant;
        else if (strcasecmp(tab_paiements[i].statut, "Retard") == 0)  retard  += tab_paiements[i].montant;
    }
    printf("  Total encaisse           : %.2f DT\n", paye);
    printf("  En attente               : %.2f DT\n", attente);
    printf("  En retard                : %.2f DT\n", retard);

    printf("\n  --- Reclamations ---\n");
    int ouv = 0, enc = 0, res = 0;
    for (int i = 0; i < nb_reclamations; i++) {
        if (strcasecmp(tab_reclamations[i].statut, "Ouverte") == 0)   ouv++;
        else if (strcasecmp(tab_reclamations[i].statut, "En_cours") == 0) enc++;
        else if (strcasecmp(tab_reclamations[i].statut, "Resolue") == 0)  res++;
    }
    printf("  Ouvertes                 : %d\n", ouv);
    printf("  En cours                 : %d\n", enc);
    printf("  Resolues                 : %d\n", res);

    pause_attente();
}

/* ============================================================
 *   MENUS
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
            case 1: ajouter_etudiant();      break;
            case 2: afficher_tous_etudiants(); break;
            case 3: rechercher_etudiant();   break;
            case 4: modifier_etudiant();     break;
            case 5: supprimer_etudiant();    break;
            case 6: assigner_chambre();      break;
            case 0: break;
            default: printf("  [ERREUR] Choix invalide.\n"); pause_attente();
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
            case 1: ajouter_chambre();         break;
            case 2: afficher_toutes_chambres(); break;
            case 3: rechercher_chambre();      break;
            case 4: modifier_chambre();        break;
            case 5: supprimer_chambre();       break;
            case 0: break;
            default: printf("  [ERREUR] Choix invalide.\n"); pause_attente();
        }
    } while (choix != 0);
}

void menu_paiements() {
    int choix;
    do {
        afficher_entete("MODULE - GESTION DES PAIEMENTS");
        printf("    1. Enregistrer un paiement\n");
        printf("    2. Afficher tous les paiements\n");
        printf("    3. Rechercher un paiement\n");
        printf("    4. Bilan financier\n");
        printf("    5. Modifier un paiement\n");
        printf("    6. Supprimer un paiement\n");
        printf("    0. Retour\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: ajouter_paiement();        break;
            case 2: afficher_tous_paiements(); break;
            case 3: rechercher_paiement();     break;
            case 4: bilan_financier();         break;
            case 5: modifier_paiement();       break;
            case 6: supprimer_paiement();      break;
            case 0: break;
            default: printf("  [ERREUR] Choix invalide.\n"); pause_attente();
        }
    } while (choix != 0);
}

void menu_reclamations() {
    int choix;
    do {
        afficher_entete("MODULE - GESTION DES RECLAMATIONS");
        printf("    1. Deposer une reclamation\n");
        printf("    2. Afficher toutes les reclamations\n");
        printf("    3. Rechercher une reclamation\n");
        printf("    4. Mettre a jour le statut\n");
        printf("    5. Modifier une reclamation\n");
        printf("    6. Supprimer une reclamation\n");
        printf("    0. Retour\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: ajouter_reclamation();           break;
            case 2: afficher_toutes_reclamations();  break;
            case 3: rechercher_reclamation();        break;
            case 4: changer_statut_reclamation();    break;
            case 5: modifier_reclamation();          break;
            case 6: supprimer_reclamation();         break;
            case 0: break;
            default: printf("  [ERREUR] Choix invalide.\n"); pause_attente();
        }
    } while (choix != 0);
}

void menu_principal() {
    int choix;
    do {
        afficher_entete("MENU PRINCIPAL - GESTION DE FOYER UNIVERSITAIRE");
        printf("    1. Gestion des Etudiants (Residents)\n");
        printf("    2. Gestion des Chambres\n");
        printf("    3. Gestion des Paiements\n");
        printf("    4. Gestion des Reclamations\n");
        printf("    5. Tableau de bord\n");
        printf("    0. Quitter\n");
        afficher_ligne();
        printf("  Votre choix : "); scanf("%d", &choix); vider_buffer();
        switch (choix) {
            case 1: menu_etudiants();    break;
            case 2: menu_chambres();     break;
            case 3: menu_paiements();    break;
            case 4: menu_reclamations(); break;
            case 5: tableau_de_bord();   break;
            case 0: printf("\n  Au revoir !\n\n"); break;
            default: printf("  [ERREUR] Choix invalide.\n"); pause_attente();
        }
    } while (choix != 0);
}

/* ============================================================
 *   MAIN
 * ============================================================ */

int main() {
    charger_donnees_demo();
    if (!authentifier()) {
        printf("\n  Acces refuse. Fermeture du programme.\n\n");
        return 1;
    }
    menu_principal();
    return 0;
}
