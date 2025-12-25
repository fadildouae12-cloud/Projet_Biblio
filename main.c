#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "catalog.c"
#include "checkout.c"
#include "supplier_order.c"
// Variables de connexion MySQL
#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASS "doudou"
#define MYSQL_DB "librairie"
// Déclarations des fonctions des modules
Livre* trouverLivreParId(int id_livre);
int ajouterLivre(const char* isbn, const char* titre, const char* auteur,const char* categorie, float prix, int qte_stock, int id_four);
int chercherLivre(const char* recherche, Livre* resultats, int max_resultats);
int mettreAJourStock(int id_livre, int quantite);
int creerCommande(int id_client);
int ajouterALaCommande(int id_commande, int id_client, int id_livre, int qte_vente);
float calculerTotal(int id_commande);
void appliquerPointsFidelite(int id_client, float montant);
int ajouterClient(const char* nom_complet, const char* num_telephone, int points_fid);
int ajouterFournisseur(const char* nom_four, const char* telephone, const char* adresse);
int passerCommandeFournisseur(int id_four, int id_livre, int quantite);
int mettreAJourStockFournisseur(int id_livre, int quantite);
// Variables globales pour MySQL
MYSQL* conn = NULL;
// Fonction pour charger les données depuis MySQL
int chargerDonneesMySQL(void) {
    MYSQL_RES* res;
    MYSQL_ROW row;
    char query[500];
      if (conn == NULL) {
        printf("Erreur: Pas de connexion MySQL!\n");
        return 0;
    }
printf("\n=== CHARGEMENT DES DONNEES ===\n");
    // Réinitialiser les compteurs
    nb_clients = 0;
    nb_fournisseurs = 0;
    nb_livres = 0;
    dernier_id_client = 0;
    dernier_id_four = 0;
    dernier_id_livre = 0;
    // Charger les clients
    strcpy(query, "SELECT id_client, nom_complet, num_telephone, points_fid FROM Client");
    if (mysql_query(conn, query)) {
        printf("Erreur: %s\n", mysql_error(conn));
        return 0;
    }
    res = mysql_store_result(conn);
    while ((row = mysql_fetch_row(res))) {
        int id = atoi(row[0]);
        clients[nb_clients].id_client = id;
        strcpy(clients[nb_clients].nom_complet, row[1]);
        strcpy(clients[nb_clients].num_telephone, row[2]);
        clients[nb_clients].points_fid = atoi(row[3]);
        if (id > dernier_id_client) dernier_id_client = id;
        nb_clients++;
    }
    mysql_free_result(res);
    printf("  %d clients charges\n", nb_clients);

    // Charger les fournisseurs
    strcpy(query, "SELECT id_four, nom_four, telephone, adresse FROM Fournisseur");
    if (mysql_query(conn, query)) {
        printf("Erreur: %s\n", mysql_error(conn));
        return 0;
    }
    res = mysql_store_result(conn);
    while ((row = mysql_fetch_row(res))) {
        int id = atoi(row[0]);
        fournisseurs[nb_fournisseurs].id_four = id;
        strcpy(fournisseurs[nb_fournisseurs].nom_four, row[1]);
        strcpy(fournisseurs[nb_fournisseurs].telephone, row[2]);
        strcpy(fournisseurs[nb_fournisseurs].adresse, row[3]);
        if (id > dernier_id_four) dernier_id_four = id;
        nb_fournisseurs++;
    }
    mysql_free_result(res);
    printf("  %d fournisseurs charges\n", nb_fournisseurs);

    // Charger les livres
    strcpy(query, "SELECT id_livre, isbn, titre, auteur, categorie, prix, qte_stock, id_four FROM Livre");
    if (mysql_query(conn, query)) {
        printf("Erreur: %s\n", mysql_error(conn));
        return 0;
    }
    res = mysql_store_result(conn);
    while ((row = mysql_fetch_row(res))) {
        int id = atoi(row[0]);
        catalogue[nb_livres].id_livre = id;
        strcpy(catalogue[nb_livres].isbn, row[1]);
        strcpy(catalogue[nb_livres].titre, row[2]);
        strcpy(catalogue[nb_livres].auteur, row[3]);
        strcpy(catalogue[nb_livres].categorie, row[4]);
        catalogue[nb_livres].prix = atof(row[5]);
        catalogue[nb_livres].qte_stock = atoi(row[6]);
        catalogue[nb_livres].id_four = row[7] ? atoi(row[7]) : 0;
        if (id > dernier_id_livre) dernier_id_livre = id;
        nb_livres++;
    }
    mysql_free_result(res);
    printf("  %d livres charges\n", nb_livres);

    return 1;
}

// Fonction pour mettre à jour le stock dans MySQL
int mettreAJourStockMySQL(int id_livre, int qte_stock) {
    char query[200];
    sprintf(query, "UPDATE Livre SET qte_stock = %d WHERE id_livre = %d", qte_stock, id_livre);
    if (mysql_query(conn, query)) {
        printf("Erreur MySQL: %s\n", mysql_error(conn));
        return 0;
    }
    return 1;
}

// Fonction pour insérer un livre dans MySQL
int insererLivreMySQL(int id_livre, const char* isbn, const char* titre, const char* auteur,
                      const char* categorie, float prix, int qte_stock, int id_four) {
    char query[500];
    char isbn_esc[50], titre_esc[200], auteur_esc[100], categorie_esc[60];
    char* isbn_escaped = isbn_esc;
    char* titre_escaped = titre_esc;
    char* auteur_escaped = auteur_esc;
    char* categorie_escaped = categorie_esc;

    // Échapper les chaînes pour éviter les injections SQL
    unsigned long isbn_len = strlen(isbn);
    unsigned long titre_len = strlen(titre);
    unsigned long auteur_len = strlen(auteur);
    unsigned long categorie_len = strlen(categorie);

    mysql_real_escape_string(conn, isbn_escaped, isbn, isbn_len);
    mysql_real_escape_string(conn, titre_escaped, titre, titre_len);
    mysql_real_escape_string(conn, auteur_escaped, auteur, auteur_len);
    mysql_real_escape_string(conn, categorie_escaped, categorie, categorie_len);

    if (id_four > 0) {
        sprintf(query, "INSERT INTO Livre (id_livre, isbn, titre, auteur, categorie, prix, qte_stock, id_four) "
                       "VALUES (%d, '%s', '%s', '%s', '%s', %.2f, %d, %d)",
                id_livre, isbn_escaped, titre_escaped, auteur_escaped, categorie_escaped, prix, qte_stock, id_four);
    } else {
        sprintf(query, "INSERT INTO Livre (id_livre, isbn, titre, auteur, categorie, prix, qte_stock, id_four) "
                       "VALUES (%d, '%s', '%s', '%s', '%s', %.2f, %d, NULL)",
                id_livre, isbn_escaped, titre_escaped, auteur_escaped, categorie_escaped, prix, qte_stock);
    }

    if (mysql_query(conn, query)) {
        printf("Erreur MySQL lors de l'insertion du livre: %s\n", mysql_error(conn));
        return 0;
    }
    printf("Livre insere dans la base de donnees avec succes!\n");
    return 1;
}

// Fonction pour insérer un fournisseur dans MySQL
int insererFournisseurMySQL(int id_four, const char* nom_four, const char* telephone, const char* adresse) {
    char query[500];
    char nom_esc[100], tel_esc[30], adresse_esc[400];
    char* nom_escaped = nom_esc;
    char* tel_escaped = tel_esc;
    char* adresse_escaped = adresse_esc;

    // Échapper les chaînes pour éviter les injections SQL
    unsigned long nom_len = strlen(nom_four);
    unsigned long tel_len = strlen(telephone);
    unsigned long adresse_len = strlen(adresse);

    mysql_real_escape_string(conn, nom_escaped, nom_four, nom_len);
    mysql_real_escape_string(conn, tel_escaped, telephone, tel_len);
    mysql_real_escape_string(conn, adresse_escaped, adresse, adresse_len);

    sprintf(query, "INSERT INTO Fournisseur (id_four, nom_four, telephone, adresse) "
                   "VALUES (%d, '%s', '%s', '%s')",
            id_four, nom_escaped, tel_escaped, adresse_escaped);

    if (mysql_query(conn, query)) {
        printf("Erreur MySQL lors de l'insertion du fournisseur: %s\n", mysql_error(conn));
        return 0;
    }
    printf("Fournisseur insere dans la base de donnees avec succes!\n");
    return 1;
}

// Fonction pour insérer une commande dans MySQL
int insererCommandeMySQL(int id_commande, int id_client, int id_livre, int qte_vente) {
    char query[300];
    sprintf(query, "INSERT INTO commande_client (id_commande, id_client, id_livre, date_vente, qte_vente) "
                   "VALUES (%d, %d, %d, NOW(), %d)",id_commande, id_client, id_livre, qte_vente);
    if (mysql_query(conn, query)) {
        printf("Erreur MySQL: %s\n", mysql_error(conn));
        return 0;
    }
    return 1;
}

// Fonctions d'affichage
void afficherCatalogue(void) {
    printf("\n=== CATALOGUE DE LIVRES ===\n");
    if (nb_livres == 0) {
        printf("Aucun livre.\n");
        return;
    }
    printf("%-5s %-20s %-40s %-25s %8s %6s\n",
           "ID", "ISBN", "Titre", "Auteur", "Prix", "Stock");
    printf("----------------------------------------------------------------------------------------\n");
    for (int i = 0; i < nb_livres; i++) {
        printf("%-5d %-20s %-40s %-25s %8.2f %6d\n",catalogue[i].id_livre, catalogue[i].isbn, catalogue[i].titre,catalogue[i].auteur, catalogue[i].prix, catalogue[i].qte_stock);
    }
}

void afficherClients(void) {
    printf("\n=== LISTE DES CLIENTS ===\n");
    if (nb_clients == 0) {
        printf("Aucun client.\n");
        return;
    }
    printf("%-5s %-30s %-15s %10s\n", "ID", "Nom", "Telephone", "Points");
    printf("--------------------------------------------------------------------\n");
    for (int i = 0; i < nb_clients; i++) {
        printf("%-5d %-30s %-15s %10d\n",clients[i].id_client, clients[i].nom_complet,clients[i].num_telephone, clients[i].points_fid);
    }
}

void afficherFournisseurs(void) {
    printf("\n=== LISTE DES FOURNISSEURS ===\n");
    if (nb_fournisseurs == 0) {
        printf("Aucun fournisseur.\n");
        return;
    }
    printf("%-5s %-30s %-15s %-40s\n", "ID", "Nom", "Telephone", "Adresse");
    printf("--------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < nb_fournisseurs; i++) {
        printf("%-5d %-30s %-15s %-40s\n",fournisseurs[i].id_four, fournisseurs[i].nom_four,fournisseurs[i].telephone, fournisseurs[i].adresse);
    }
}

void afficherCommandes(void) {
    printf("\n=== COMMANDES CLIENTS ===\n");
    if (nb_commandes == 0) {
        printf("Aucune commande.\n");
        return;
    }
    printf("%-5s %-5s %-5s %-20s %6s\n", "Cmd", "Client", "Livre", "Date", "Qte");
    printf("--------------------------------------------------------------------\n");
    for (int i = 0; i < nb_commandes; i++) {
        printf("%-5d %-5d %-5d %-20s %6d\n",commandes[i].id_commande, commandes[i].id_client,commandes[i].id_livre, commandes[i].date_vente, commandes[i].qte_vente);
    }
}

void afficherMenu(void) {
    printf("\n========================================\n");
    printf("   GESTION DE LIBRAIRIE\n");
    printf("========================================\n");
    printf("1.  Afficher le catalogue\n");
    printf("2.  Afficher les clients\n");
    printf("3.  Afficher les fournisseurs\n");
    printf("4.  Afficher les commandes\n");
    printf("5.  Chercher un livre\n");
    printf("6.  Ajouter un livre\n");
    printf("7.  Mettre a jour le stock\n");
    printf("8.  Creer une commande client\n");
    printf("9.  Ajouter un livre a la commande\n");
    printf("10. Calculer le total d'une commande\n");
    printf("11. Ajouter un fournisseur\n");
    printf("12. Passer une commande fournisseur\n");
    printf("13. Mettre a jour stock (reception fournisseur)\n");
    printf("14. Recharger les donnees depuis MySQL\n");
    printf("0.  Quitter\n");
    printf("========================================\n");
    printf("Votre choix: ");
}

int main(void) {
    int choix;
    char buffer[200];
    int id, id2, qte;
    float prix;
    Livre resultats[10];
    int nb_resultats;
    // Initialiser et connecter MySQL
    conn = mysql_init(NULL);
    if (conn == NULL) {
        printf("Erreur: Impossible d'initialiser MySQL\n");
        return 1;
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, NULL, 0)) {
        printf("Erreur de connexion: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }


    // Charger les données initiales
    if (!chargerDonneesMySQL()) {
        mysql_close(conn);
        return 1;
    }

    // Menu interactif
    do {
        afficherMenu();
        scanf("%d", &choix);
        getchar();

        switch (choix) {
            case 1:
                afficherCatalogue();
                break;

            case 2:
                afficherClients();
                break;

            case 3:
                afficherFournisseurs();
                break;

            case 4:
                afficherCommandes();
                break;

            case 5: {
                printf("\n--- Chercher un livre ---\n");
                printf("Terme de recherche: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                nb_resultats = chercherLivre(buffer, resultats, 10);
                if (nb_resultats > 0) {
                    printf("\n%d Livre(s) trouve(s):\n", nb_resultats);
                    for (int i = 0; i < nb_resultats; i++) {
                        printf("  ID: %d | %s | %s | %.2f DH | Stock: %d\n",resultats[i].id_livre, resultats[i].titre,resultats[i].auteur, resultats[i].prix, resultats[i].qte_stock);
                    }
                } else {
                    printf("Aucun resultat.\n");
                }
                break;
            }

            case 6: {
                printf("\n--- Ajouter un livre ---\n");
                printf("ISBN: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char isbn[20];
                strcpy(isbn, buffer);

                printf("Titre: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char titre[100];
                strcpy(titre, buffer);

                printf("Auteur: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char auteur[50];
                strcpy(auteur, buffer);

                printf("Categorie: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char categorie[30];
                strcpy(categorie, buffer);

                printf("Prix: ");
                scanf("%f", &prix);
                printf("Quantite stock: ");
                scanf("%d", &qte);
                printf("ID Fournisseur (0 si aucun): ");
                scanf("%d", &id);
                getchar(); // Nettoyer le buffer

                if (ajouterLivre(isbn, titre, auteur, categorie, prix, qte, id)) {
                    // Le livre vient d'être ajouté, il est à la fin du tableau
                    if (nb_livres > 0) {
                        Livre* livre_ajoute = &catalogue[nb_livres - 1];
                        insererLivreMySQL(livre_ajoute->id_livre, livre_ajoute->isbn,
                                         livre_ajoute->titre, livre_ajoute->auteur,
                                         livre_ajoute->categorie, livre_ajoute->prix,
                                         livre_ajoute->qte_stock, livre_ajoute->id_four);
                    }
                }
                break;
            }

            case 7: {
                printf("\n--- Mettre a jour le stock ---\n");
                printf("ID livre: ");
                scanf("%d", &id);
                printf("Quantite a ajouter (negatif pour decrementer): ");
                scanf("%d", &qte);
                if (mettreAJourStock(id, qte)) {
                    Livre* livre = trouverLivreParId(id);
                    if (livre != NULL) {
                        mettreAJourStockMySQL(id, livre->qte_stock);
                    }
                }
                break;
            }

            case 8: {
                printf("\n--- Creer une commande client ---\n");
                printf("ID client: ");
                scanf("%d", &id);
                int id_cmd = creerCommande(id);
                if (id_cmd > 0) {
                    printf("Commande %d creee avec succes!\n", id_cmd);
                    // Note: La commande sera insérée dans MySQL quand on ajoutera un livre à la commande
                }
                break;
            }

            case 9: {
                printf("\n--- Ajouter un livre a la commande ---\n");
                printf("ID commande: ");
                scanf("%d", &id);
                printf("ID client: ");
                scanf("%d", &id2);
                printf("ID livre: ");
                int id_livre;
                scanf("%d", &id_livre);
                printf("Quantite: ");
                scanf("%d", &qte);

                if (ajouterALaCommande(id, id2, id_livre, qte)) {
                    Livre* livre = trouverLivreParId(id_livre);
                    if (livre != NULL) {
                        mettreAJourStockMySQL(id_livre, livre->qte_stock);
                    }
                    insererCommandeMySQL(id, id2, id_livre, qte);
                }
                break;
            }

            case 10: {
                printf("\n--- Calculer le total d'une commande ---\n");
                printf("ID commande: ");
                scanf("%d", &id);
                float total = calculerTotal(id);
                if (total >= 0) {
                    printf("Total: %.2f DH\n", total);
                    // Appliquer les points de fidélité
                    for (int i = 0; i < nb_commandes; i++) {
                        if (commandes[i].id_commande == id) {
                            appliquerPointsFidelite(commandes[i].id_client, total);
                            break;
                        }
                    }
                }
                break;
            }

            case 11: {
                printf("\n--- Ajouter un fournisseur ---\n");
                printf("Nom: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char nom[50];
                strcpy(nom, buffer);

                printf("Telephone: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char tel[15];
                strcpy(tel, buffer);

                printf("Adresse: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                char adresse[200];
                strcpy(adresse, buffer);

                int id_four_ajoute = ajouterFournisseur(nom, tel, adresse);
                if (id_four_ajoute > 0) {
                    insererFournisseurMySQL(id_four_ajoute, nom, tel, adresse);
                }
                break;
            }

            case 12: {
                printf("\n--- Passer une commande fournisseur ---\n");
                printf("ID fournisseur: ");
                scanf("%d", &id);
                printf("ID livre: ");
                scanf("%d", &id2);
                printf("Quantite: ");
                scanf("%d", &qte);
                passerCommandeFournisseur(id, id2, qte);
                break;
            }

            case 13: {
                printf("\n--- Mettre a jour stock (reception fournisseur) ---\n");
                printf("ID livre: ");
                scanf("%d", &id);
                printf("Quantite recue: ");
                scanf("%d", &qte);
                if (mettreAJourStockFournisseur(id, qte)) {
                    Livre* livre = trouverLivreParId(id);
                    if (livre != NULL) {
                        mettreAJourStockMySQL(id, livre->qte_stock);
                    }
                }
                break;
            }

            case 14:
                chargerDonneesMySQL();
                break;

            case 0:
                printf("\nAu revoir!\n");
                break;

            default:
                printf("\nChoix invalide!\n");
                break;
        }

    } while (choix != 0);

    mysql_close(conn);
    return 0;
}
