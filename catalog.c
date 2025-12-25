#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
#define MAX_LIVRES 100

// Variables globales
Livre catalogue[MAX_LIVRES];
int nb_livres = 0;
int dernier_id_livre = 0;

// Fonction pour trouver un livre par ID
Livre* trouverLivreParId(int id_livre) {
    for (int i = 0; i < nb_livres; i++) {
        if (catalogue[i].id_livre == id_livre) {
            return &catalogue[i];
        }
    }
    return NULL;
}

// Ajouter un livre dans la table Livre
int ajouterLivre(const char* isbn, const char* titre, const char* auteur,
                 const char* categorie, float prix, int qte_stock, int id_four) {

    if (nb_livres >= MAX_LIVRES) {
        printf("Erreur: Catalogue plein!\n");
        return 0;
    }

    // Vérifier si l'ISBN existe déjà
    for (int i = 0; i < nb_livres; i++) {
        if (strcmp(catalogue[i].isbn, isbn) == 0) {
            printf("Erreur: ISBN deja existe!\n");
            return 0;
        }
    }

    // Créer le nouveau livre (utiliser l'ID de la base si disponible)
    // Si dernier_id_livre est 0, on incrémente, sinon on utilise l'ID de la base
    if (dernier_id_livre == 0) {
        dernier_id_livre++;
        catalogue[nb_livres].id_livre = dernier_id_livre;
    } else {
        // Chercher le prochain ID disponible depuis la base
        int max_id = 0;
        for (int i = 0; i < nb_livres; i++) {
            if (catalogue[i].id_livre > max_id) {
                max_id = catalogue[i].id_livre;
            }
        }
        catalogue[nb_livres].id_livre = max_id + 1;
        dernier_id_livre = max_id + 1;
    }
    strcpy(catalogue[nb_livres].isbn, isbn);
    strcpy(catalogue[nb_livres].titre, titre);
    strcpy(catalogue[nb_livres].auteur, auteur);
    strcpy(catalogue[nb_livres].categorie, categorie);
    catalogue[nb_livres].prix = prix;
    catalogue[nb_livres].qte_stock = qte_stock;
    catalogue[nb_livres].id_four = id_four;

    nb_livres++;
    printf("Livre ajoute avec succes! ID: %d\n", dernier_id_livre);
    return 1;
}

// Chercher un livre par titre, auteur ou ISBN
int chercherLivre(const char* recherche, Livre* resultats, int max_resultats) {
    int nb_resultats = 0;

    for (int i = 0; i < nb_livres && nb_resultats < max_resultats; i++) {
        // Recherche dans le titre
        if (strstr(catalogue[i].titre, recherche) != NULL) {
            resultats[nb_resultats] = catalogue[i];
            nb_resultats++;
            continue;
        }

        // Recherche dans l'auteur
        if (strstr(catalogue[i].auteur, recherche) != NULL) {
            resultats[nb_resultats] = catalogue[i];
            nb_resultats++;
            continue;
        }

        // Recherche dans l'ISBN
        if (strcmp(catalogue[i].isbn, recherche) == 0) {
            resultats[nb_resultats] = catalogue[i];
            nb_resultats++;
        }
    }

    return nb_resultats;
}

// Mettre à jour la quantité en stock après vente ou réception
int mettreAJourStock(int id_livre, int quantite) {
    Livre* livre = trouverLivreParId(id_livre);

    if (livre == NULL) {
        printf("Erreur: Livre non trouve!\n");
        return 0;
    }

    livre->qte_stock += quantite;

    if (livre->qte_stock < 0) {
        printf("Attention: Stock negatif! Remis a 0.\n");
        livre->qte_stock = 0;
        return 0;
    }

    printf("Stock mis a jour: %d exemplaires\n", livre->qte_stock);
    return 1;
}
