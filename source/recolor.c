/*
	File : recolor.c
	Nom    : Devienne
	Prenom : Alexandre
	CAMIPRO: 246865
	Date   : 2014-10-11 
	Version code: 1.00
	Version de la donnée: 1.03
	Description : projet recolor. lit une table de couleurs
	 et des seuils pour transformer une image en format ppm
*/

#include <stdio.h>
#include <stdlib.h>
//#include <stdbool.h>
#include <math.h>

// différence minimum entre 2 seuils successifs
#define TOLERANCE_SEUIL 0.005f

// fonctions prédéfinies pour indiquer si les données sont correctes
static void correct(void);
static void erreur_nbR(int nbR);
static void erreur_couleur(float couleur);
static void erreur_seuil(float seuil);
static void erreur_seuil_non_croissant(float s1, float s2);
static void erreur_seuil_non_distinct(float s1, float s2);


float seuillage(int pixel_RGB[], int max);

//TODO, maybe, change input method to return a value, and not a void
void scan_string(char string[]); // scan a number and exit on failure
void scan_int(int *nb);
void scan_float(float *nb);
//TODO not consistent : no sizeof in the 2D version
float* init_tab(int size); // allocate memory for a table
float** init_2D_tab(int x, int y); // allocate memory for a 2D table


//-------------------------------------------------------------------
int main(void)
{
    int i=0, j=0, k=0;

    int verbose = -1;

    int nbR = 0; // nombre de couleurs de recoloriage
    int nbF = 0; // nombre de filtrages
    float **couleurs = NULL; // tablau des couleur de recoloriage
    float *seuils = NULL; // seuils utilisés

    char format[2] = {0}; //TODO change name and check method
    int nbC = 0; // nombre de colonne
    int nbL = 0; // nombre de ligne
    int couleur_max = 0; //TODO change variable name
    int pixel_RGB[3] = {0};
    float **image = NULL;

    scan_int(&verbose);

    if (verbose) printf("Nombre de couleurs de recoloriage : ");
    scan_int(&nbR);
    if (nbR<2 || nbR>255)
        erreur_nbR(nbR);

    couleurs = init_2D_tab(nbR+1, 3);
    for (i=0 ; i<3 ; i++) // met la premiere couleur a NOIR
        couleurs[0][i] = 0;

    if (verbose) printf("Entrer celles-ci (format RGB variant de 0 à 1) :\n");
    for (i=1 ; i<nbR+1 ; i++) // scan color
    {
        for (j=0 ; j<3 ; j++) // scan RBG components
        {
            scan_float(&couleurs[i][j]);
            if (couleurs[i][j]<0 || couleurs[i][j]>1)
                erreur_couleur(couleurs[i][j]);
        }
    }

    seuils = init_float_tab(nbR-1);

    if (verbose) printf("Entrer la valeur des seuils à utilisés :\n");
    for (i=0 ; i<nbR-1 ; i++)
    {
        scan_float(&seuils[i]);
        if (seuils[i]<0 || seuils[i]>1)
            erreur_seuil(seuils[i]);
        else if (seuils[i]<TOLERANCE_SEUIL) // different from seuil 0 // TODO check if those 2 test are needed
            erreur_seuil_non_distinct(0, seuils[i]);
        else if (seuils[i]>1-TOLERANCE_SEUIL) // different from seuil 1
            erreur_seuil_non_distinct(seuils[i], 1);
        else if (i>=1)
        {
            if (seuils[i] < seuils[i-1])
                erreur_seuil_non_croissant(seuils[i-1], seuils[i]);
            else if (seuils[i]-seuils[i-1] < TOLERANCE_SEUIL)
                erreur_seuil_non_distinct(seuils[i-1], seuils[i]);
        }
    }

    if (verbose) printf("Nombre de filtrage : ");
    scan_int(&nbF);
    //TODO add check for nbF (nbF<1)

    if (verbose) printf("Entrer les informations de l'image en format PPM :\n");
    scan_string(format); // TODO check value "P3"

    scan_int(&nbC); //TODO check value
    scan_int(&nbL); //TODO check value
    scan_int(&couleur_max); //TODO check value
    
    image = init_2D_tab(nbC, nbL);

    for (i=0 ; i<nbC ; i++)
    {
        for (j=0 ; j<nbL ; j++)
        {
            for (k=0 ; k<3 ; k++)
            {
                scan_int(&pixel_RGB[k]);
            }
            image[i][j] = seuillage(pixel_RGB, couleur_max);
        }
    }

    correct();

    free(image);
    free(couleurs);
    free(seuils);
	return EXIT_SUCCESS;
}


float seuillage(int pixel_RGB[], int max)
{
    int i;
    float result = 0;
    
    for(i=0 ; i<3 ; i++)
        result += pow(pixel_RGB[i], 2);
    result = sqrt(result) / (sqrt(3) * max);

    return result; 
}

// Scan a char from the default input and exit on failure
void scan_string(char string[])
{
    if (scanf("%s", string) != 1)
    {
        printf("Input failed\n");
        exit(EXIT_FAILURE);
    }
}

// Scan an integer from the default input and exit on failure
void scan_int(int *nb)
{
    if (scanf("%d", nb) != 1)
    {
        printf("Input failed\n");
        exit(EXIT_FAILURE);
    }
}

// Scan a float form the default input and exit on failure
void scan_float(float *nb)
{
    if (scanf("%f", nb) != 1)
    {
        printf("Input failed\n");
        exit(EXIT_FAILURE);
    }
}

// allocate memory to a pointer
float* init_tab(int size)
{
    float *pointer = NULL;
    pointer = (float *) malloc(size);
    if (pointer == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return pointer;
}

// allocate memory for a 2 dimension tab
float** init_2D_tab(int x, int y)
{
    int i;
    float **pointer = NULL;

    pointer = (float**) malloc(x*sizeof(float*));
    if (pointer == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (i=0 ; i<x ; i++)
    {
        pointer[i] = init_tab(y*sizeof(float));
    }

    return pointer;
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
