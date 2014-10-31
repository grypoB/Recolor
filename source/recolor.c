/*
	File : recolor.c
	Nom    : Devienne
	Prenom : Alexandre Andre Steven
	CAMIPRO: 246865
	Date   : 2014-10-31
	Version code: 1.00
	Version de la donnée: 1.03
	Description : projet recolor. lit une table de couleurs
	 et des seuils pour transformer une image en format ppm
*/

#include <stdio.h>
#include <stdlib.h>

#define MIN_SEUIL 0
#define MAX_SEUIL 1
#define TOLERANCE_SEUIL 0.005f // différence min entre 2 seuils successifs

#define MIN_COLOR 0 // min/max value of a color (0 and 1 for normalized value)
#define MAX_COLOR 1
#define MIN_RECOLOR_NB 2
#define MAX_RECOLOR_NB 255
#define COLOR_COMPONENTS 3 // number of components per color (RGB format)

#define BORDER_COLOR 0 // RGB normalized format for the border color

#define FORMAT_SIZE 3 // expected numbers of characters for the format string


// fonctions prédéfinies pour indiquer si les données sont correctes
static void correct(void);
static void erreur_nbR(int nbR);
static void erreur_couleur(float couleur);
static void erreur_seuil(float seuil);
static void erreur_seuil_non_croissant(float s1, float s2);
static void erreur_seuil_non_distinct(float s1, float s2);


//-------------------------------------------------------------------
int main(void)
{
    int i=0, j=0, k=0;

    int verbose = -1;

    int nbR = 0; // nombre de couleurs de recoloriage
    int nbF = 0; // nombre de filtrages

    char format[FORMAT_SIZE] = {0};
    int nbC = 0; // nombre de colonne
    int nbL = 0; // nombre de ligne
    int intensite_max = 0;
    int rgb_values[COLOR_COMPONENTS] = {0};

    /* Variable lenght arrays
    float seuils[];
    float couleurs[][];
    float image[][];
    */


    scanf("%d", &verbose); // verbose value is assumed correct

    if (verbose) printf("Entrez le nombre de couleurs de recoloriage :\n");
    scanf("%d", &nbR);
    if (nbR<MIN_RECOLOR_NB || nbR>MAX_RECOLOR_NB)
        erreur_nbR(nbR);

    float couleurs[nbR+1][COLOR_COMPONENTS];
    for (i=0 ; i<COLOR_COMPONENTS ; i++) // init border color
        couleurs[0][i] = BORDER_COLOR;

    if (verbose) printf("Entrez les %d couleurs de recoloriage "
                        "(format RGB normalisé) :\n", nbR);
    for (i=1 ; i<nbR+1 ; i++) // scan color
    {
        for (j=0 ; j<COLOR_COMPONENTS ; j++) // scan RBG components
        {
            scanf("%f", &couleurs[i][j]);
            if (couleurs[i][j]<MIN_COLOR || couleurs[i][j]>MAX_COLOR)
                erreur_couleur(couleurs[i][j]);
        }
    }

    float seuils[nbR-1];

    if (verbose) printf("Entrez les %d seuils de recoloriage :\n", nbR-1);
    for (i=0 ; i<nbR-1 ; i++) // scan seuils
    {
        scanf("%f", &seuils[i]);
        if (seuils[i]<=MIN_SEUIL || seuils[i]>=MAX_SEUIL)
            erreur_seuil(seuils[i]);
        else if (seuils[i]<MIN_SEUIL+TOLERANCE_SEUIL)
            erreur_seuil_non_distinct(MIN_SEUIL, seuils[i]);
        else if (seuils[i]>MAX_SEUIL-TOLERANCE_SEUIL)
            erreur_seuil_non_distinct(seuils[i], MAX_SEUIL);
        else if (i>=1)
        {
            if (seuils[i] < seuils[i-1])
                erreur_seuil_non_croissant(seuils[i-1], seuils[i]);
            else if (seuils[i]-seuils[i-1] < TOLERANCE_SEUIL)
                erreur_seuil_non_distinct(seuils[i-1], seuils[i]);
        }
    }

    if (verbose) printf("Entrez le nombre de filtrage :\n");
    scanf("%d", &nbF); // nbF is assumed correct

    // all values are assumed correct from this point
    if (verbose) printf("Entrez le code du format de l'image :\n");
    scanf("%s", format);
    if (verbose) printf("Entrez les dimensions de l'image :\n");
    scanf("%d", &nbC);
    scanf("%d", &nbL);
    if (verbose) printf("Entrez l'intensité max pour la couleur : \n");
    scanf("%d", &intensite_max);
    
    //float image[nbC][nbL];

    if (verbose) printf("Entrez les valeurs des couleurs des pixels :\n");
    for (i=0 ; i<nbC ; i++)
    {
        for (j=0 ; j<nbL ; j++)
        {
            for (k=0 ; k<COLOR_COMPONENTS ; k++)
            {
                scanf("%d", &rgb_values[k]);
            }
            //image[i][j] = normalize(rgb_values, intensite_max);
        }
    }

    correct();

    return EXIT_SUCCESS;
}


//---------------------------------------------------------------------
// Fonctions prédéfinies pour indiquer si les données sont correctes
// Les fonctions signalant une erreur provoquent la fin du programme
// en appelant exit(). Leur message d'erreur est toujours affiché.
//
//                 NE PAS MODIFIER CES FONCTIONS
//---------------------------------------------------------------------
// A appeler si toutes les données sont correctes (rendu intermédiaire)
static void correct(void)
{
	printf("Les données sont correctes ! \n");
}

//----------------------------------------------------
static void erreur_nbR(int nbR)
{
	printf("Nombre de couleurs incorrect [2,255]: %d\n",nbR);
	exit(EXIT_FAILURE);
}

//----------------------------------------------------
static void erreur_couleur(float couleur)
{
	printf("Valeur R-V-B incorrecte [0., 1.]: %6.3f\n",couleur);
	exit(EXIT_FAILURE);
}

//----------------------------------------------------
static void erreur_seuil(float seuil)
{
	printf("Valeur de seuil incorrecte ]0., 1.[: %6.3f\n",seuil);
	exit(EXIT_FAILURE);
}


//-------------------------------------------------------------
// A appeler si s1 > s2, où s1 et s2 sont 2 seuils consécutifs
static void erreur_seuil_non_croissant(float s1, float s2)
{
	printf("Valeurs de seuils non croissantes: %5.3f > %5.3f\n", s1, s2);
	exit(EXIT_FAILURE);
}

//---------------------------------------------------------------
// A appeler si l'écart entre 2 seuils consécutifs est strictement 
// inférieur à la valeur TOLERANCE_SEUIL
static void erreur_seuil_non_distinct(float s1, float s2)
{
	printf("Valeur de seuil trop proche du précédent [%5.3f]:"
	       " %5.6f et %5.6f\n",TOLERANCE_SEUIL, s1, s2);
	exit(EXIT_FAILURE);
}
