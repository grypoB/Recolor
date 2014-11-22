/*
	File : recolor.c
	Nom    : Devienne
	Prenom : Alexandre
	CAMIPRO: 246865
	Date   : 2014-10-30
	Version code: 1.00
	Version de la donnée: 1.03
	Description : projet recolor. lit une table de couleurs
	 et des seuils pour transformer une image en format ppm
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> // for sqrt, pow function


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

#define MAX_PIXEL_PER_LINE 6 // for printing the image (add line break)


// fonctions prédéfinies pour indiquer si les données sont correctes
static void correct(void);
static void erreur_nbR(int nbR);
static void erreur_couleur(float couleur);
static void erreur_seuil(float seuil);
static void erreur_seuil_non_croissant(float s1, float s2);
static void erreur_seuil_non_distinct(float s1, float s2);

// from a pixel's RGB values, return a single normalized value
static float normalize(int rgb_values[], int max);

// seuillage function in logarithm complexity
// params : sorted array, size of array, val to look for
// output : first x such as val > array[x]
static int seuillage(float array[], int size, float val);


// input function, act as scanf, but checks if the return value is correct 
// params : pointer to var
static void scan_int(int *nb_adress);
static void scan_float(float *nb_adress);
static void scan_string(char string[]);

// memory allocation function
// params : number of cell
// output : pointer
static int*  init_int_tab(int size);
static int** init_2D_int_tab(int x, int y);
static float*  init_float_tab(int size);
static float** init_2D_float_tab(int x, int y);

void error_allocation(long unsigned int byte);

// Free a 2D tab (allocated by init_2D_float_tab)
static void free_2D_int_tab(int **pointer, int x);
static void free_2D_float_tab(float **pointer, int x);

//-------------------------------------------------------------------
int main(void)
{
    int i=0, j=0, k=0;

    int verbose = -1;

    int nbR = 0; // nombre de couleurs de recoloriage
    int nbF = 0; // nombre de filtrages
    float *seuils    = NULL; // seuils utilisés
    float **couleurs = NULL; // tableau des couleurs de recoloriage

    char format[FORMAT_SIZE] = {0};
    int nbC = 0; // nombre de colonne
    int nbL = 0; // nombre de ligne
    int intensite_max = 0;
    int rgb_values[COLOR_COMPONENTS] = {0};
    int **image = NULL; // 2D array of the image's pixel

    scan_int(&verbose); // verbose value is assumed correct

    if (verbose) printf("Entrez le nombre de couleurs de recoloriage :\n");
    scan_int(&nbR);
    if (nbR<MIN_RECOLOR_NB || nbR>MAX_RECOLOR_NB)
        erreur_nbR(nbR);

    couleurs = init_2D_float_tab(nbR+1, COLOR_COMPONENTS);
    for (i=0 ; i<COLOR_COMPONENTS ; i++) // init border color
        couleurs[0][i] = BORDER_COLOR;

    if (verbose) printf("Entrez les %d couleurs de recoloriage "
                        "(format RGB normalisé) :\n", nbR);
    for (i=1 ; i<nbR+1 ; i++) // scan color
    {
        for (j=0 ; j<COLOR_COMPONENTS ; j++) // scan RBG components
        {
            scan_float(&couleurs[i][j]);
            if (couleurs[i][j]<MIN_COLOR || couleurs[i][j]>MAX_COLOR)
                erreur_couleur(couleurs[i][j]);
        }
    }

    seuils = init_float_tab(nbR-1);

    if (verbose) printf("Entrez les %d seuils de recoloriage :\n", nbR-1);
    for (i=0 ; i<nbR-1 ; i++) // scan seuils
    {
        scan_float(&seuils[i]);
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
    scan_int(&nbF); // nbF is assumed correct

    // all values are assumed correct from this point
    if (verbose) printf("Entrez le code du format de l'image :\n");
    scan_string(format);
    if (verbose) printf("Entrez les dimensions de l'image :\n");
    scan_int(&nbC);
    scan_int(&nbL);
    if (verbose) printf("Entrez l'intensité max pour la couleur : \n");
    scan_int(&intensite_max);

    image = init_2D_int_tab(nbC, nbL);

    if (verbose) printf("Entrez les valeurs des couleurs des pixels :\n");
    for (i=0 ; i<nbC ; i++)
    {
        for (j=0 ; j<nbL ; j++)
        {
            for (k=0 ; k<COLOR_COMPONENTS ; k++)
            {
                scan_int(&rgb_values[k]);
            }
            // seuillage : add 1 because MIN_SEUIL is not in array seuils
            image[i][j] = 1 + seuillage(seuils, nbR+1, normalize(rgb_values, intensite_max));
        }
    }

    correct();

    free(seuils);
    free_2D_int_tab(image, nbC);
    free_2D_float_tab(couleurs, nbR+1);
	return EXIT_SUCCESS;
}


// From a pixel's RGB values, return a single normalized value
static float normalize(int rgb_values[], int max)
{
    int i;
    float result = 0;
 
    for (i=0 ; i<COLOR_COMPONENTS ; i++)
        result += pow(rgb_values[i], 2);

    return sqrt(result) / (sqrt(COLOR_COMPONENTS) * max);
}

// Return in which interval a val is inside a tab
// Counts from 0 and interval of type [x, y[
static int seuillage(float array[], int size, float val)
{
    
    if (val < array[size/2])
    {
        if (size/2==0)
            return 0; // before the first value
        else
            return seuillage(array, size/2, val);
    }
    else
    {
        if (size==1)
            return 1; // after the last value
        else // search in the "right" array (so shift the start of the array)
            return size/2 + seuillage(array+size/2, size-size/2, val);
    }
    
}


// print image in ppm format
// nor_color[X] contain the normalized values of the sub-pixels of color X 
// image[][] store the id of the color of the corresponding pixel
// max_color is used to un-normalized nor_color[]
static void print_image_ppm(char format[], int x, int y, int image[x][y],  
                            int nb_color, float nor_color[nb_color][COLOR_COMPONENTS], 
                            int max_color)
{
    int i, j, k;
    int pixel_count = 0; // to add line-break after MAX_PIXEL_PER_LINE lines
    int **color = init_2D_int_tab(x, y); // un-normalized value of the color 
    
    // un-normalized nor_color
    for (i=0 ; i<nb_color ; i++)
        for (j=0 ; j<COLOR_COMPONENTS ; j++)
        {
            color[i][j] = nor_color[i][j] * max_color;
        }

    // print the image
    // general info about the image
    printf("%s\n", format);
    printf("%d\n", x);
    printf("%d\n", y);
    printf("%d\n", max_color);

    // the actual image
    for (j=0 ; j<y ; j++)
        for (i=0 ; i<x ; i++)
        {
            pixel_count++;

            for (k=0 ; k<COLOR_COMPONENTS; k++)
            {
                printf("%d ", color[image[i][j]][k]);
            }

            if (pixel_count%MAX_PIXEL_PER_LINE == 0) // add line break 
                printf("\n");
        }

}


// Scan an integer from the default input and exit on failure
static void scan_int(int *nb_adress)
{
    if (scanf("%d", nb_adress) != 1)
    {
        printf("ERROR : Input failed (expected [%%d])\n");
        exit(EXIT_FAILURE);
    }
}


// Scan a float from the default input and exit on failure
static void scan_float(float *nb_adress)
{
    if (scanf("%f", nb_adress) != 1)
    {
        printf("ERROR : Input failed (expected [%%f])\n");
        exit(EXIT_FAILURE);
    }
}


// Scan a string from the default input and exit on failure
/* RISKY
 * If the string is longer than the memory allowed
 * it will override values in the memory
 */
static void scan_string(char string[])
{
    if (scanf("%s", string) != 1)
    {
        printf("ERROR : Input failed (expected a string)\n");
        exit(EXIT_FAILURE);
    }
}


// Allocate memory for a 1D tab
static float* init_float_tab(int size)
{
    float *pointer = NULL;

    pointer = (float *) malloc(size*sizeof(float));

    if (pointer == NULL)
    {
        error_allocation(size*sizeof(float));
    }

    return pointer;
}


// Allocate memory for a 2D tab
static float** init_2D_float_tab(int x, int y)
{
    int i;
    float **pointer = NULL;

    pointer = (float**) malloc(x*sizeof(float*));
    if (pointer == NULL)
    {
        error_allocation(x*sizeof(float*));
    }

    for (i=0 ; i<x ; i++)
    {
        pointer[i] = init_float_tab(y);
    }

    return pointer;
}


// Allocate memory for a 1D tab
static int* init_int_tab(int size)
{
    int *pointer = NULL;

    pointer = (int *) malloc(size*sizeof(int));

    if (pointer == NULL)
    {
        error_allocation(size*sizeof(int));
    }

    return pointer;
}


// Allocate memory for a 2D tab
static int** init_2D_int_tab(int x, int y)
{
    int i;
    int **pointer = NULL;

    pointer = (int**) malloc(x*sizeof(int*));
    if (pointer == NULL)
    {
        error_allocation(x*sizeof(int*));
    }

    for (i=0 ; i<x ; i++)
    {
        pointer[i] = init_int_tab(y);
    }

    return pointer;
}


// Free the memory from a 2D tab (initialized by init_2D_tab())
static void free_2D_float_tab(float **pointer, int x)
{
    int i;

    for (i=0 ; i<x ; i++)
    {
        free(pointer[i]);
    }

    free(pointer);
}


// Free the memory from a 2D tab (initialized by init_2D_tab())
static void free_2D_int_tab(int **pointer, int x)
{
    int i;

    for (i=0 ; i<x ; i++)
    {
        free(pointer[i]);
    }

    free(pointer);
}


void error_allocation(long unsigned int byte)
{
        printf("ERROR : Memory allocation failed (%lu bytes)\n", byte);
        exit(EXIT_FAILURE);
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
