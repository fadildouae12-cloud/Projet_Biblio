#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structures.h"

#define MAX_CLIENTS 50
#define MAX_COMMANDES 200



// Variables globales
Client clients[MAX_CLIENTS];
CommandeClient commandes[MAX_COMMANDES];
int nb_clients = 0;
int nb_commandes = 0;
int dernier_id_client = 0;
int dernier_id_commande = 0;



extern Livre catalogue[];
extern int nb_livres;

// Déclaration de la fonction (définie dans catalog.c)
Livre* trouverLivreParId(int id_livre);

// Fonction pour trouver un client par ID
Client* trouverClientParId(int id_client) {
    for (int i = 0; i < nb_clients; i++) {
        if (clients[i].id_client == id_client) {
            return &clients[i];
        }
    }
    return NULL;
}

// Fonction pour obtenir la date actuelle
void obtenirDateActuelle(char* buffer, int taille) {
    time_t maintenant = time(NULL);
    struct tm* tm_info = localtime(&maintenant);
    strftime(buffer, taille, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Créer une commande client (dans commande_client)
int creerCommande(int id_client) {
    Client* client = trouverClientParId(id_client);
    if (client == NULL) {
        printf("Erreur: Client non trouve!\n");
        return -1;
    }

    if (nb_commandes >= MAX_COMMANDES) {
        printf("Erreur: Nombre maximum de commandes atteint!\n");
        return -1;
    }

    dernier_id_commande++;
    printf("Commande creee avec succes! ID: %d pour %s\n",
           dernier_id_commande, client->nom_complet);

    return dernier_id_commande;
}

// Ajouter un livre à la commande (décrémenter qte_stock)
int ajouterALaCommande(int id_commande, int id_client, int id_livre, int qte_vente) {
    if (qte_vente <= 0) {
        printf("Erreur: Quantite invalide!\n");
        return 0;
    }

    Client* client = trouverClientParId(id_client);
    if (client == NULL) {
        printf("Erreur: Client non trouve!\n");
        return 0;
    }

    Livre* livre = trouverLivreParId(id_livre);
    if (livre == NULL) {
        printf("Erreur: Livre non trouve!\n");
        return 0;
    }

    if (livre->qte_stock < qte_vente) {
        printf("Erreur: Stock insuffisant! Disponible: %d\n", livre->qte_stock);
        return 0;
    }

    if (nb_commandes >= MAX_COMMANDES) {
        printf("Erreur: Nombre maximum de commandes atteint!\n");
        return 0;
    }

    // Ajouter la commande
    commandes[nb_commandes].id_commande = id_commande;
    commandes[nb_commandes].id_client = id_client;
    commandes[nb_commandes].id_livre = id_livre;
    commandes[nb_commandes].qte_vente = qte_vente;
    obtenirDateActuelle(commandes[nb_commandes].date_vente, 20);
    nb_commandes++;

    // Décrémenter le stock
    livre->qte_stock -= qte_vente;

    printf("Livre ajoute a la commande: %s (x%d) - Stock restant: %d\n",
           livre->titre, qte_vente, livre->qte_stock);

    return 1;
}

// Calculer le total de la commande (prix × quantité)
float calculerTotal(int id_commande) {
    float total = 0.0;
    int commande_trouvee = 0;

    for (int i = 0; i < nb_commandes; i++) {
        if (commandes[i].id_commande == id_commande) {
            commande_trouvee = 1;
            Livre* livre = trouverLivreParId(commandes[i].id_livre);
            if (livre != NULL) {
                total += livre->prix * commandes[i].qte_vente;
            }
        }
    }

    if (!commande_trouvee) {
        printf("Erreur: Commande non trouvee!\n");
        return -1;
    }

    return total;
}

// Appliquer les points de fidélité (1 point pour 10 DH)
void appliquerPointsFidelite(int id_client, float montant) {
    Client* client = trouverClientParId(id_client);
    if (client == NULL) {
        return;
    }

    int nouveaux_points = (int)(montant / 10.0);
    client->points_fid += nouveaux_points;

    printf("Points de fidelite ajoutes: +%d points. Total: %d points\n",
           nouveaux_points, client->points_fid);
}

// Fonction utilitaire pour ajouter un client
int ajouterClient(const char* nom_complet, const char* num_telephone, int points_fid) {
    if (nb_clients >= MAX_CLIENTS) {
        return -1;
    }

    dernier_id_client++;
    clients[nb_clients].id_client = dernier_id_client;
    strcpy(clients[nb_clients].nom_complet, nom_complet);
    strcpy(clients[nb_clients].num_telephone, num_telephone);
    clients[nb_clients].points_fid = points_fid;
    nb_clients++;

    return dernier_id_client;
}
