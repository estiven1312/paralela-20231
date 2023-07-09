#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <assert.h>
#include "mpi.h"
#include <ctype.h>

#define MAX_PALABRAS_DOCUMENTO 1000
#define MAX_WORD_LENGTH 20
#define NUMERO_DOCUMENTOS 3
void removeSpecialChars(char *word)
{
    char *src = word;
    char *dst = word;

    while (*src)
    {
        if (isalpha(*src))
        {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

void removeNewLine(char *str)
{
    size_t length = strcspn(str, "\n");
    str[length] = '\0';
}

typedef struct
{
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

typedef struct
{
    char word[MAX_WORD_LENGTH];
    double idf;
    double matriz[NUMERO_DOCUMENTOS];
} WordCountPerDocument;
int compareWordCount(const void *a, const void *b)
{
    return strcmp(((WordCount *)a)->word, ((WordCount *)b)->word);
}
void reduce(int n, WordCount *wc)
{
    for (int i = 1; i < n; i++)
    {
        if (strcmp(wc[i].word, wc[i - 1].word) == 0)
        {
            wc[i].count += wc[i - 1].count;
            wc[i - 1].count = 0;
        }
    }
}
void calcularTFIDF(double **tfidf, double **tf, double *idf, int numeroFilas, int numColumnas)
{
    for (int i = 0; i < numeroFilas; i++)
    {
        for (int j = 0; j < numColumnas; j++)
        {
            tfidf[i][j] = tf[i][j] * idf[j];
        }
    }
}
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    char document[20];

    WordCount *wc1[MAX_PALABRAS_DOCUMENTO];
    WordCount *wc2[MAX_PALABRAS_DOCUMENTO];
    WordCount *wc3[MAX_PALABRAS_DOCUMENTO];

    int numPalabrasDoc1;
    int numPalabrasDoc2;
    int numPalabrasDoc3;

    char texto[MAX_PALABRAS_DOCUMENTO * MAX_WORD_LENGTH];
    char palabras[MAX_PALABRAS_DOCUMENTO][MAX_WORD_LENGTH];
    // PArte del MAP
    if (rank != 0)
    {
        // En esta parte se hace la lectura de los docs
        sprintf(document, "sample%d.txt", rank);
        WordCount *wc[MAX_PALABRAS_DOCUMENTO];
        for (int i = 0; i < MAX_PALABRAS_DOCUMENTO; i++)
        {
            wc[i] = NULL;
        }

        FILE *file = fopen(document, "r");
        char textoCompleto[10000] = "";
        char line[1000];
        while (fgets(line, sizeof(line), file))
        {
            removeNewLine(line);
            strcat(textoCompleto, line);
        }
        // fgets(textoDocumentos[i], 10000, file);
        int length = strlen(textoCompleto);
        for (int i = 0; i < length; i++)
        {
            if (!isalpha(textoCompleto[i]))
            {
                textoCompleto[i] = ' ';
            }
        }
        strcpy(texto, textoCompleto);
        texto[strcspn(texto, "\n")] = 0;
        int n = 0;
        char *word = strtok(texto, " ");
        while (word != NULL)
        {
            n++;

            *wc = realloc(*wc, (n) * sizeof(WordCount));
            strcpy((*wc)[n - 1].word, word);
            (*wc)[n - 1].count = 1;
            // printf("N: %d palabra, %s, count %d \n", rank,(*wc)[n - 1].word, (*wc)[n - 1].count );
            word = strtok(NULL, " ");
        }
        fclose(file);

        qsort(*wc, n, sizeof(WordCount), compareWordCount);
        printf("Numero de palabras en el documento %d es %d", rank, n);
        MPI_Send(*wc, n * sizeof(WordCount), MPI_BYTE, 0, 0, MPI_COMM_WORLD);

        MPI_Send(&n, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
    // En el Proceso 0 se realiza el reduce
    if (rank == 0)
    {
        MPI_Recv(&numPalabrasDoc1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&numPalabrasDoc2, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&numPalabrasDoc3, 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("N: %d %d %d \n", numPalabrasDoc1, numPalabrasDoc2, numPalabrasDoc3);

        *wc1 = malloc(numPalabrasDoc1 * sizeof(WordCount));
        *wc2 = malloc(numPalabrasDoc2 * sizeof(WordCount));
        *wc3 = malloc(numPalabrasDoc3 * sizeof(WordCount));

        MPI_Recv(*wc1, numPalabrasDoc1 * sizeof(WordCount), MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(*wc2, numPalabrasDoc2 * sizeof(WordCount), MPI_BYTE, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(*wc3, numPalabrasDoc3 * sizeof(WordCount), MPI_BYTE, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        qsort(*wc1, numPalabrasDoc1, sizeof(WordCount), compareWordCount);
        qsort(*wc2, numPalabrasDoc2, sizeof(WordCount), compareWordCount);
        qsort(*wc3, numPalabrasDoc3, sizeof(WordCount), compareWordCount);

        reduce(numPalabrasDoc1, *wc1);
        reduce(numPalabrasDoc2, *wc2);
        reduce(numPalabrasDoc3, *wc3);

        // Luego de obtener el arreglo de palabras y su contador, se procederá a eliminar duplicados

        WordCount *wc1SinDuplicados[3 * MAX_PALABRAS_DOCUMENTO];
        for (int i = 0; i < MAX_PALABRAS_DOCUMENTO; i++)
        {
            wc1SinDuplicados[i] = NULL;
        }
        int wc1SinDuplicadosLength = 0;
        for (int j = 0; j < numPalabrasDoc1; j++)
        {

            if ((*wc1)[j].count > 0)
            {
                wc1SinDuplicadosLength++;
                *wc1SinDuplicados = realloc(*wc1SinDuplicados, (wc1SinDuplicadosLength) * sizeof(WordCount));
                strcpy((*wc1SinDuplicados)[wc1SinDuplicadosLength - 1].word, (*wc1)[j].word);
                (*wc1SinDuplicados)[wc1SinDuplicadosLength - 1].count = (*wc1)[j].count;
            }
        }

        WordCount *wc2SinDuplicados[3 * MAX_PALABRAS_DOCUMENTO];
        for (int i = 0; i < MAX_PALABRAS_DOCUMENTO; i++)
        {
            wc2SinDuplicados[i] = NULL;
        }
        int wc2SinDuplicadosLength = 0;
        for (int j = 0; j < numPalabrasDoc2; j++)
        {

            if ((*wc2)[j].count > 0)
            {
                wc2SinDuplicadosLength++;
                *wc2SinDuplicados = realloc(*wc2SinDuplicados, (wc2SinDuplicadosLength) * sizeof(WordCount));
                strcpy((*wc2SinDuplicados)[wc2SinDuplicadosLength - 1].word, (*wc2)[j].word);
                (*wc2SinDuplicados)[wc2SinDuplicadosLength - 1].count = (*wc2)[j].count;
            }
        }

        WordCount *wc3SinDuplicados[3 * MAX_PALABRAS_DOCUMENTO];
        for (int i = 0; i < MAX_PALABRAS_DOCUMENTO; i++)
        {
            wc3SinDuplicados[i] = NULL;
        }
        int wc3SinDuplicadosLength = 0;
        for (int j = 0; j < numPalabrasDoc3; j++)
        {

            if ((*wc3)[j].count > 0)
            {
                wc3SinDuplicadosLength++;
                *wc3SinDuplicados = realloc(*wc3SinDuplicados, (wc3SinDuplicadosLength) * sizeof(WordCount));
                strcpy((*wc3SinDuplicados)[wc3SinDuplicadosLength - 1].word, (*wc3)[j].word);
                (*wc3SinDuplicados)[wc3SinDuplicadosLength - 1].count = (*wc3)[j].count;
            }
        }

        WordCount *allWC[3 * MAX_PALABRAS_DOCUMENTO];
        for (int i = 0; i < MAX_PALABRAS_DOCUMENTO; i++)
        {
            allWC[i] = NULL;
        }

        int k = 0;

        for (int j = 0; j < numPalabrasDoc1; j++)
        {

            if ((*wc1)[j].count > 0)
            {
                k++;
                *allWC = realloc(*allWC, (k) * sizeof(WordCount));
                strcpy((*allWC)[k - 1].word, (*wc1)[j].word);
                (*allWC)[k - 1].count = (*wc1)[j].count;
            }
        }
        for (int j = 0; j < numPalabrasDoc2; j++)
        {
            if ((*wc2)[j].count > 0)
            {
                k++;

                *allWC = realloc(*allWC, (k) * sizeof(WordCount));
                strcpy((*allWC)[k - 1].word, (*wc2)[j].word);
                (*allWC)[k - 1].count = (*wc2)[j].count;
            }
        }
        for (int j = 0; j < numPalabrasDoc3; j++)
        {
            if ((*wc3)[j].count > 0)
            {
                k++;
                *allWC = realloc(*allWC, (k) * sizeof(WordCount));
                strcpy((*allWC)[k - 1].word, (*wc3)[j].word);
                (*allWC)[k - 1].count = (*wc3)[j].count;
            }
        }
        qsort(*allWC, k, sizeof(WordCount), compareWordCount);
        reduce(k, *allWC);

        WordCount *allWCSinDuplicados[3 * MAX_PALABRAS_DOCUMENTO];
        for (int i = 0; i < MAX_PALABRAS_DOCUMENTO; i++)
        {
            allWCSinDuplicados[i] = NULL;
        }
        int numeroSinDuplicados = 0;
        for (int j = 0; j < k; j++)
        {
            if ((*allWC)[j].count > 0)
            {
                numeroSinDuplicados++;

                *allWCSinDuplicados = realloc(*allWCSinDuplicados, (numeroSinDuplicados) * sizeof(WordCount));
                strcpy((*allWCSinDuplicados)[numeroSinDuplicados - 1].word, (*allWC)[j].word);
                (*allWCSinDuplicados)[numeroSinDuplicados - 1].count = (*allWC)[j].count;
            }
        }
        for (int i = 0; i < numeroSinDuplicados; i++)
        {
            printf("N: %d palabra, %s, count %d \n", rank, (*allWCSinDuplicados)[i].word, (*allWCSinDuplicados)[i].count);
        }
        printf("\n");

        // Algoritmo TFIDF

        int rows = NUMERO_DOCUMENTOS;   // número de filas
        int cols = numeroSinDuplicados; // número de columnas

        double **tf; // declaración del puntero a puntero
        double **tfidf;
        double *idf = malloc(cols * sizeof(double));

        // Asignación de memoria para las filas
        tf = (double **)malloc(rows * sizeof(double *));
        tfidf = (double **)malloc(rows * sizeof(double *));

        // Asignación de memoria para las columnas de cada fila
        for (int i = 0; i < rows; i++)
        {
            tf[i] = (double *)malloc(cols * sizeof(double));
            tfidf[i] = (double *)malloc(cols * sizeof(double));
        }

        tf = (double **)malloc(rows * sizeof(double *));
        // Asignación de memoria para las columnas de cada fila
        for (int i = 0; i < rows; i++)
        {
            tf[i] = (double *)malloc(cols * sizeof(double));
        }

        for (int i = 0; i < cols; i++)
        {
            int numDocsAparicion = 0;
            for (int k = 0; k < wc1SinDuplicadosLength; k++)
            {
                int count = 0;

                if (strcmp((*allWCSinDuplicados)[i].word, (*wc1SinDuplicados)[k].word) == 0)
                {
                    count = (*wc1SinDuplicados)[k].count;
                    numDocsAparicion++;
                    tf[0][i] = count / (numPalabrasDoc1 * 1.000);
                }
            }
            for (int k = 0; k < wc2SinDuplicadosLength; k++)
            {
                int count = 0;

                if (strcmp((*allWCSinDuplicados)[i].word, (*wc2SinDuplicados)[k].word) == 0)
                {
                    count = (*wc2SinDuplicados)[k].count;
                    numDocsAparicion++;
                    tf[1][i] = count / (numPalabrasDoc2 * 1.000);
                }
            }
            for (int k = 0; k < wc3SinDuplicadosLength; k++)
            {
                int count = 0;

                if (strcmp((*allWCSinDuplicados)[i].word, (*wc3SinDuplicados)[k].word) == 0)
                {
                    count = (*wc3SinDuplicados)[k].count;
                    numDocsAparicion++;
                    tf[2][i] = count / (numPalabrasDoc3 * 1.000);
                }
            }
            idf[i] = log10(NUMERO_DOCUMENTOS / (numDocsAparicion * 1.00));
        }

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {

                printf("%f \t", tf[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        for (int i = 0; i < cols; i++)
        {
            printf("%f \t", idf[i]);
        }
        printf("\n");

        calcularTFIDF(tfidf, tf, idf, NUMERO_DOCUMENTOS, cols);
        printf("==============\n");
        printf("TFIDF MATRIX\n");
        // Liberación de la memoria asignada
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {

                printf("%f \t", tfidf[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        for (int i = 0; i < rows; i++)
        {
            free(tf[i]);
        }
        free(tf);

        for (int i = 0; i < rows; i++)
        {
            free(tfidf[i]);
        }
        free(tfidf);
        free(idf);
        //
    }
    MPI_Finalize();
}
