#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"

#define MAX_FOURNISSEURS 20


// Variables globales
Fournisseur fournisseurs[MAX_FOURNISSEURS];
int nb_fournisseurs = 0;
int dernier_id_four = 0;

extern Livre catalogue[];
extern int nb_livres;

// Déclaration de la fonction (définie dans catalog.c)
Livre* trouverLivreParId(int id_livre);

// Fonction pour trouver un fournisseur par ID
Fournisseur* trouverFournisseurParId(int id_four) {
    for (int i = 0; i < nb_fournisseurs; i++) {
        if (fournisseurs[i].id_four == id_four) {
            return &fournisseurs[i];
        }
    }
    return NULL;
}

// Ajouter un fournisseur (Fournisseur)
int ajouterFournisseur(const char* nom_four, const char* telephone, const char* adresse) {
    if (nb_fournisseurs >= MAX_FOURNISSEURS) {
        printf("Erreur: Nombre maximum de fournisseurs atteint!\n");
        return -1;
    }

    dernier_id_four++;
    fournisseurs[nb_fournisseurs].id_four = dernier_id_four;
    strcpy(fournisseurs[nb_fournisseurs].nom_four, nom_four);
    strcpy(fournisseurs[nb_fournisseurs].telephone, telephone);
    strcpy(fournisseurs[nb_fournisseurs].adresse, adresse);
    nb_fournisseurs++;

    printf("Fournisseur ajoute avec succes! ID: %d, Nom: %s\n",
           dernier_id_four, nom_four);

    return dernier_id_four;
}

// Passer une commande fournisseur (pour réapprovisionner)
int passerCommandeFournisseur(int id_four, int id_livre, int quantite) {
    if (quantite <= 0) {
        printf("Erreur: Quantite invalide!\n");
        return 0;
    }

    Fournisseur* fournisseur = trouverFournisseurParId(id_four);
    if (fournisseur == NULL) {
        printf("Erreur: Fournisseur non trouve!\n");
        return 0;
    }

    Livre* livre = trouverLivreParId(id_livre);
    if (livre == NULL) {
        printf("Erreur: Livre non trouve!\n");
        return 0;
    }

    printf("Commande fournisseur passee avec succes!\n");
    printf("  Fournisseur: %s (ID: %d)\n", fournisseur->nom_four, id_four);
    printf("  Livre: %s (ID: %d)\n", livre->titre, id_livre);
    printf("  Quantite commandee: %d\n", quantite);

    return 1;
}

// Mettre à jour le stock du livre après réception de la commande
int mettreAJourStockFournisseur(int id_livre, int quantite) {
    if (quantite <= 0) {
        printf("Erreur: Quantite invalide!\n");
        return 0;
    }

    Livre* livre = trouverLivreParId(id_livre);
    if (livre == NULL) {
        printf("Erreur: Livre non trouve!\n");
        return 0;
    }

    livre->qte_stock += quantite;

    printf("Stock mis a jour apres reception fournisseur!\n");
    printf("  Livre: %s (ID: %d)\n", livre->titre, id_livre);
    printf("  Quantite recue: %d\n", quantite);
    printf("  Nouveau stock: %d\n", livre->qte_stock);

    return 1;
}
