#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct {
    int id_livre;
    char isbn[20];
    char titre[100];
    char auteur[50];
    char categorie[30];
    float prix;
    int qte_stock;
    int id_four;
} Livre;

typedef struct {
    int id_client;
    char nom_complet[50];
    char num_telephone[15];
    int points_fid;
} Client;

typedef struct {
    int id_four;
    char nom_four[50];
    char telephone[15];
    char adresse[200];
} Fournisseur;

typedef struct {
    int id_commande;
    int id_client;
    int id_livre;
    char date_vente[20];
    int qte_vente;
} CommandeClient;

#endif
