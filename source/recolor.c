/*
	File : recolor.c
	Nom    : Devienne
	Prenom : Alexandre
	CAMIPRO: 246865
	Date   : 2014-11-24
	Version code: 2.00
	Version de la donnée: 1.03
	Description : projet recolor. lit une table de couleurs
	 et des seuils pour transformer une image en format ppm
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> // for sqrt function


#define MIN_SEUIL 0 // allowed interval for seuils (normalized value)
#define MAX_SEUIL 1
#define TOLERANCE_SEUIL 0.005f // différence min entre 2 seuils successifs

#define MIN_COLOR 0 // allowed interval for normalized "recoloriage" color
#define MAX_COLOR 1
#define MIN_RECOLOR_NB 2
#define MAX_RECOLOR_NB 255
#define COLOR_COMPONENTS 3 // number of components per color (3 for RGB format)

#define BORDER_COLOR 0 // RGB normalized format for the border color
#define DEFAULT_COLOR 0 // color id used for border and for filtrage

#define VOISIN_RADIUS 1 // radius of the circle in which to consider adjacent cells
#define MINIMUM_VOISIN 6 // number of voisin in order to take the corresponding color
// size ot the array to store voisin values
// (maximum of adjacent color before the result color is DEFAULT_COLOR)
//                    |        number of voisin         |
#define TAILLE_VOISIN (4*VOISIN_RADIUS*(VOISIN_RADIUS+1)) - MINIMUM_VOISIN + 1

#define NOT_ASSIGNED -1 // a default val until a correct val is assigned

#define FORMAT_SIZE 3 // expected numbers of characters for the format string

#define MAX_PIXEL_PER_LINE 5 // printing formating
#define CHAR_BY_SUBPIXEL 3


int verbose = -1;


// Read the various input and init corresponding arrays
static float** read_recoloriage(int *nbR_adr);
static float*  read_seuils(int size_seuils, int *nb_filtrage_adr);
static void    read_image_params(char format[], int *rows_adr, int *columns_adr,
                                 int *intensite_max_adr);
static int**   process_image(int rows, int columns, int size_seuils,
                             float seuils[size_seuils], int intensite_max);

// from a pixel's RGB values, return a single grayscale value between 0 and 1
static float grayscale(int rgb_values[], int max);

// params : sorted array, size of array, val to look for
// output : smallest x such as val < array[x]
static int seuillage(float array[], int size, float val);

// filtrage algorithm
static void filtrage(int rows, int columns, int *image[rows], int nb_filtrage);
// fct to manipulate voisin[][] which stores value of neighboring cells
static int update_voisin(int voisin[TAILLE_VOISIN][2], int color, int ammount);
static void reset_voisin(int voisin[TAILLE_VOISIN][2]);

// print the image out of filtrage fct
static void print_image_ppm(char format[], int rows, int columns, int *image[rows],
                            int nb_color, float *nor_color[nb_color], int max_color);


// Tab utility fct :
// copy
static void copy_2D_tab(int rows, int columns, int *source[rows], int *target[rows]);
// set border of 2D tab to a specified val
static void set_border(int rows, int columns, int *tab[rows],
                       int val, int border_size);

// Memory allocation function
// params : number of cell and bytes number per cell
// output : pointer
static void*  init_tab   (unsigned long int bytes, unsigned int rows);
static void** init_2D_tab(unsigned long int bytes, unsigned int rows,
                          unsigned int columns);
static void error_allocation(unsigned long int bytes);
// free a 2D tab (allocated by init_2D_tab)
static void free_2D_tab(void **ptr, unsigned int rows);

// fonctions prédéfinies pour indiquer si les données sont correctes
static void correct(void);
static void erreur_nbR(int nbR);
static void erreur_couleur(float couleur);
static void erreur_seuil(float seuil);
static void erreur_seuil_non_croissant(float s1, float s2);
static void erreur_seuil_non_distinct (float s1, float s2);


//-------------------------------------------------------------------
int main(void)
{
    int nbR = 0; // nombre de couleurs de recoloriage
    int nbF = 0; // nombre de filtrages
    int rows = 0; // refers to the image
    int columns = 0; // refers to the image
    int intensite_max = 0;

    int   **image = NULL; // 2D array of the image's pixel
    float *seuils    = NULL; // seuils utilisés
    float **couleurs = NULL; // tableau des couleurs de recoloriage

    char format[FORMAT_SIZE] = {0};

    scanf("%d", &verbose); // verbose value is assumed correct

    couleurs = read_recoloriage(&nbR);
    seuils   = read_seuils(nbR-1, &nbF);
    read_image_params(format, &rows, &columns, &intensite_max);
    image    = process_image(rows, columns, nbR-1, seuils, intensite_max);

    //* Rendu final
    filtrage(rows, columns, image, nbF);
    print_image_ppm(format, rows, columns, image, nbR+1, couleurs, intensite_max);
    // */

    /* Rendu inter
    correct();
    // */

    free(seuils);
    free_2D_tab((void**) image, rows);
    free_2D_tab((void**) couleurs, nbR+1);
	return EXIT_SUCCESS;
}


// read the colors of the "recoloriage"
// return pointer to 2D array of colors (needs to be freed)
static float** read_recoloriage(int *nbR_adr)
{
    int i=0, j=0;
    int nbR = 0;
    float **couleurs = NULL;

    if (verbose) printf("Entrez le nombre de couleurs de recoloriage :\n");
    scanf("%d", &nbR);
    if (nbR<MIN_RECOLOR_NB || nbR>MAX_RECOLOR_NB)
        erreur_nbR(nbR);

    couleurs = (float**) init_2D_tab(sizeof(float), nbR+1, COLOR_COMPONENTS);

    for (i=0 ; i<COLOR_COMPONENTS ; i++) // init border color
        couleurs[DEFAULT_COLOR][i] = BORDER_COLOR;

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

    *nbR_adr = nbR;
    return couleurs;
}


// read the various seuils and check if they are correct
// also reads the number of filtrage
// return pointer to array seuils (needs to be freed)
static float* read_seuils(int size_seuils, int *nb_filtrage_adr)
{
    int i=0;
    float *seuils = (float*) init_tab(sizeof(float), size_seuils);

    if (verbose) printf("Entrez les %d seuils de recoloriage :\n", size_seuils);
    for (i=0 ; i<size_seuils ; i++) // scan seuils
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
    scanf("%d", nb_filtrage_adr); // filtrage nb is assumed correct

    return seuils;
}


// read the image basic parameters
static void read_image_params(char format[], int *rows_adr, int *columns_adr,
                              int *intensite_max_adr)
{
    // all values are assumed correct
    if (verbose) printf("Entrez le code du format de l'image :\n");
    scanf("%s", format);
    if (verbose) printf("Entrez les dimensions de l'image :\n");
    scanf("%d", columns_adr);
    scanf("%d", rows_adr);
    if (verbose) printf("Entrez l'intensité max pour la couleur : \n");
    scanf("%d", intensite_max_adr);

}


// read the image's pixels and already to a "seuillage"
// return the pointer to image (needs to be freed)
static int** process_image(int rows, int columns, int size_seuils,
                           float seuils[size_seuils], int intensite_max)
{
    int i=0, j=0, k=0;
    int rgb_values[COLOR_COMPONENTS] = {0};
    int **image = (int**) init_2D_tab(sizeof(int), rows, columns);

    if (verbose) printf("Entrez les valeurs des couleurs des pixels :\n");
    for (i=0 ; i<rows ; i++)
    {
        for (j=0 ; j<columns ; j++)
        {
            for (k=0 ; k<COLOR_COMPONENTS ; k++)
            {
                // pixel values are assumed correct
                scanf("%d", &rgb_values[k]);
            }
            // seuillage : add 1 because MIN_SEUIL is not in array seuils
            image[i][j] = 1 + seuillage(seuils, size_seuils,
                                        grayscale(rgb_values, intensite_max));
        }
    }

    return image;
}


// From a pixel's RGB values, return a single grayscale value between 0 and 1
static float grayscale(int rgb_values[], int max)
{
    int i;
    float result = 0;
 
    for (i=0 ; i<COLOR_COMPONENTS ; i++)
        result += rgb_values[i] * rgb_values[i];

    return sqrt(result) / (sqrt(COLOR_COMPONENTS) * max);
}


// Return in which interval a val is inside a tab
// output : smallest x such as val < array[x]
// if val > array[size-1], return size
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
static void print_image_ppm(char format[], int rows, int columns, int *image[rows],
                            int nb_color, float *nor_color[nb_color], int max_color)
{
    int i=0, j=0, k=0;
    int pixel_count = 0; // to add line-break after MAX_PIXEL_PER_LINE lines
    // to store the scaled value of the color
    int **color = (int**) init_2D_tab(sizeof(int), nb_color, COLOR_COMPONENTS);

    // to scale nor_color
    for (i=0 ; i<nb_color ; i++)
        for (j=0 ; j<COLOR_COMPONENTS ; j++)
        {
            color[i][j] = nor_color[i][j] * max_color;
        }

    // general info about the image
    printf("%s\n", format);
    printf("%d %d\n", columns, rows);
    printf("%d\n", max_color);

    // the actual pixels
    for (i=0 ; i<rows ; i++)
    {
        for (j=0 ; j<columns ; j++)
        {
            if (pixel_count == MAX_PIXEL_PER_LINE)
            {
                printf("\n");
                pixel_count = 0;
            }

            pixel_count++;

            for (k=0 ; k<COLOR_COMPONENTS; k++)
            {
                printf("%*d ", CHAR_BY_SUBPIXEL, color[image[i][j]][k]);
            }
            printf(" "); // some more spacing between pixels
        }
        pixel_count = 0;
        printf("\n");
    }

    free_2D_tab((void**) color, nb_color);
}


// filtrage main fct
static void filtrage(int rows, int columns, int *image[rows], int nb_filtrage)
{
    int i, j, k, l;
    int countF = 0; // count the number of filtrage done

    // store in each row a neighboring color and the ammount of it
    int voisin[TAILLE_VOISIN][2] = {{0}};
    int **temp_image = (int**) init_2D_tab(sizeof(int), rows, columns); // buffer image

    if (nb_filtrage>=1) // set border to DEFAULT_COLOR
        set_border(rows, columns, temp_image, DEFAULT_COLOR, VOISIN_RADIUS);
    for (countF=0; countF<nb_filtrage ; countF++)
    {
        // cycle through all pixels (border excepted)
        for (i=VOISIN_RADIUS; i<rows-VOISIN_RADIUS ; i++)
        {
            for (j=VOISIN_RADIUS; j<columns-VOISIN_RADIUS ; j++)
            {
                temp_image[i][j] = NOT_ASSIGNED;
                reset_voisin(voisin);

                // cycle through all neighbors
                for (k=i-VOISIN_RADIUS ; k<=i+VOISIN_RADIUS ; k++)
                {
                    for (l=j-VOISIN_RADIUS ;
                         l<=j+VOISIN_RADIUS && temp_image[i][j]==NOT_ASSIGNED ;
                         l++)
                    {
                        if (k!=i || l!=j) // if not equal to center pixel
                        {
                            // check if this new data allows to make any descision
                            temp_image[i][j] = update_voisin(voisin, image[k][l], 1);
                        }
                    }
                }
                // if not decided yet
                if (temp_image[i][j] == NOT_ASSIGNED)
                {
                    temp_image[i][j] = DEFAULT_COLOR;
                }
            }
        }

        // update image by temp_image for further filtrage
        copy_2D_tab(rows, columns, temp_image, image);
    }

    free_2D_tab((void**) temp_image, rows);
}


// update voisin[][] with given params :
// params : increase the color by ammount
// return : color whci should be the result pixel given the data in voisin
// e.g : 1 - if a color >= MAXIMUM_VOISIN, return this color
//       2 - if all color slots are already assigned, return the DEFAULT_COLOR
//            (none can reach MINIMUM_VOISIN given the definition of TAILLE_VOISIN)
static int update_voisin(int voisin[TAILLE_VOISIN][2], int color, int ammount)
{
    int i;
    int found = 0; // if a place in array as been found

    if (ammount == 0)
        return NOT_ASSIGNED;

    for (i=0 ; i<TAILLE_VOISIN && !found ; i++)
    {
        if (voisin[i][0] == color || voisin[i][0]==NOT_ASSIGNED)
        {
            found = 1;
            voisin[i][0] = color;
            voisin[i][1] += ammount;

            if (voisin[i][1] >= MINIMUM_VOISIN)
                return color;
        }
    }
    if (!found) // no place left in array, thus no color can be >= MINIMUM_VOISIN
        return DEFAULT_COLOR;
    else // not enough data to decide yet
        return NOT_ASSIGNED;
}


// clear voisin[][] for further anylisis
static void reset_voisin(int voisin[TAILLE_VOISIN][2])
{
    int i;

    for (i=0; i<TAILLE_VOISIN ; i++)
    {
        voisin[i][0] = NOT_ASSIGNED; // set all color to default (-1)
        voisin[i][1] = 0; // reset all ammounts to 0
    }
}


// set the border of an array to a val
static void set_border(int rows, int columns, int *tab[rows], int val, int border_size)
{
    int i, j;

    if (rows<=border_size || columns<=border_size)
    {
        border_size = (rows<columns ? rows : columns);
    }

    for (i=0 ; i<border_size ; i++)
    {
        // set top and bottom rows
        for (j=i ; j<columns-i ; j++)
        {
            tab[i][j] = val;
            tab[rows-i-1][j] = val;

        }
        // set left and right columns
        for (j=i ; j<rows-i-1 ; j++)
        {
            tab[j][i] = val;
            tab[j][columns-i-1] = val;
        }
    }
}


// copy a 2D tab into another 2D tab (of specified size)
static void copy_2D_tab(int rows, int columns, int *source[rows], int *target[rows])
{
    int i, j;

    for (i=0 ; i<rows ; i++)
        for (j=0 ; j<columns ; j++)
            target[i][j] = source[i][j];
}


// Allocate memory for a 1D tab
static void* init_tab(unsigned long int bytes, unsigned int rows)
{
    void *ptr = NULL;
    ptr = malloc(rows*bytes);
    if (ptr == NULL)
        error_allocation(rows*bytes);
    return ptr;
}

// Allocate memory for a 2D tab
static void** init_2D_tab(unsigned long int bytes, unsigned int rows,
                          unsigned int columns)
{
    int i;
    void **ptr = NULL;

    ptr = malloc(rows*sizeof(void*));
    if (ptr == NULL)
    {
        error_allocation(rows*sizeof(void*));
    }

    for (i=0 ; i<rows ; i++)
    {
        ptr[i] = init_tab(bytes, columns);
    }

    return ptr;
}

// Free 2D tab initialized by init_2D_tab
static void free_2D_tab(void **ptr, unsigned int rows)
{
    int i;
    for (i=0 ; i<rows ; i++)
    {
        free(ptr[i]);
    }

    free(ptr);
}

// In case malloc fails
static void error_allocation(unsigned long int bytes)
{
        printf("ERROR : Memory allocation failed (%lu bytes)\n", bytes);
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
