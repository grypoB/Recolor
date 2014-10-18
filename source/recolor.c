/*
	File : recolor.c
	Nom    : Devienne
	Prenom : Alexandre
	CAMIPRO: 246865
	Date   : 2014-10-18 
	Version code: 1.00
	Version de la donnée: 1.03
	Description : projet recolor. lit une table de couleurs
	 et des seuils pour transformer une image en format ppm
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// différence minimum entre 2 seuils successifs
#define TOLERANCE_SEUIL 0.005f
//TODO add MIN=0 and MAX_SEUIL=1 if i'm using them in th code

// fonctions prédéfinies pour indiquer si les données sont correctes
static void correct(void);
static void erreur_nbR(int nbR);
static void erreur_couleur(float couleur);
static void erreur_seuil(float seuil);
static void erreur_seuil_non_croissant(float s1, float s2);
static void erreur_seuil_non_distinct(float s1, float s2);


float normalize(int RGB_values[], int max);
//TODO : static functions ?

// input function, act as scanf, but checks if the return value is correct //TODO check if this is authorized
// params : pointer to var
void scan_string(char string[]);
void scan_int(int *nb_adress);
void scan_float(float *nb_adress);


// memory allocation function
// params : number of cell
// output : pointer
float* init_float_tab(int size);
float** init_2D_float_tab(int x, int y);


//-------------------------------------------------------------------
int main(void)
{
    int i=0, j=0, k=0;

    int verbose = -1;

    int nbR = 0; // nombre de couleurs de recoloriage
    int nbF = 0; // nombre de filtrages
    float **couleurs = NULL; // tablau des couleur de recoloriage
    float *seuils = NULL; // seuils utilisés

    char format[3] = {0}; 
    int nbC = 0; // nombre de colonne
    int nbL = 0; // nombre de ligne
    int couleur_max = 0; //TODO change variable name
    int RGB_values[3] = {0}; //TODO check if var name respects convention
    float **image = NULL;

    scan_int(&verbose);

    if (verbose) printf("Nombre de couleurs de recoloriage : ");
    scan_int(&nbR);
    if (nbR<2 || nbR>255)
        erreur_nbR(nbR);

    couleurs = init_2D_float_tab(nbR+1, 3);
    for (i=0 ; i<3 ; i++) // init black color
        couleurs[0][i] = 0;

    if (verbose) printf("Entrer celles-ci (format RGB normalisé) :\n");
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
        if (seuils[i]<=0 || seuils[i]>=1)
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
    scan_int(&nbF); // nbF is assumed correct

    if (verbose) printf("Entrer les informations de l'image en format PPM :\n");
    scan_string(format); // all values are assumed correct from this point
    scan_int(&nbC);
    scan_int(&nbL);
    scan_int(&couleur_max);
    
    image = init_2D_float_tab(nbC, nbL);

    for (i=0 ; i<nbC ; i++)
    {
        for (j=0 ; j<nbL ; j++)
        {
            for (k=0 ; k<3 ; k++)
            {
                scan_int(&RGB_values[k]);
            }
            image[i][j] = normalize(RGB_values, couleur_max);
        }
    }

    correct();

    free(image);
    free(couleurs);
    free(seuils);
	return EXIT_SUCCESS;
}


float normalize(int RGB_values[], int max)
{
    int i;
    float result = 0;
 
    for(i=0 ; i<3 ; i++)
        result += pow(RGB_values[i], 2);

    result = sqrt(result) / (sqrt(3) * max);

    return result;
}


// Scan a string from the default input and exit on failure
void scan_string(char string[])
{
    if (scanf("%s", string) != 1)
    {
        printf("Input failed\n");
        exit(EXIT_FAILURE);
    }
}


// Scan an integer from the default input and exit on failure
void scan_int(int *nb_adress)
{
    if (scanf("%d", nb_adress) != 1)
    {
        printf("Input failed\n");
        exit(EXIT_FAILURE);
    }
}


// Scan a float form the default input and exit on failure
void scan_float(float *nb_adress)
{
    if (scanf("%f", nb_adress) != 1)
    {
        printf("Input failed\n");
        exit(EXIT_FAILURE);
    }
}


// Allocate memory for a 1D tab
float* init_float_tab(int size)
{
    float *pointer = NULL;
    pointer = (float *) malloc(size*sizeof(float));
    if (pointer == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return pointer;
}


// Allocate memory for a 2D tab
float** init_2D_float_tab(int x, int y)
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
        pointer[i] = init_float_tab(y);
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
