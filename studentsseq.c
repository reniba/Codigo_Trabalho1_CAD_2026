#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

int input_data(int *R, int *C, int *A, int *N, int *T, int *SEED)
{
    scanf("%d", R);
    scanf("%d", C);
    scanf("%d", A);
    scanf("%d", N);
    scanf("%d", T);
    scanf("%d", SEED);
    return 1;
}

static int float_compare(const void *a, const void *b)
{
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    if (fa < fb)
        return -1;
    if (fa > fb)
        return 1;
    return 0;
}

int create_avaliation_table(int R, int C, int A, int N, int T, int SEED, float avaliation_table[R][C][A][N])
{
    srand(SEED);
    {
        // Cada thread tem sua própria semente baseada no ID dela
        unsigned int seed = SEED + omp_get_thread_num();
        for (int i = 0; i < R; i++)
        {
            for (int j = 0; j < C; j++)
            {
                for (int k = 0; k < A; k++)
                {
                    for (int l = 0; l < N; l++)
                    {
                        avaliation_table[i][j][k][l] = ((float)rand_r(&seed) / (float)RAND_MAX) * 100.0f;
                    }
                }
            }
        }
    }
    return 1;
}

void create_average_table(int R, int C, int A, int N, int T, float avaliation_table[R][C][A][N], float average_table[R][C][A])
{
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            for (int k = 0; k < A; k++)
            {
                double sum = 0.0f;
                for (int l = 0; l < N; l++)
                {
                    sum += avaliation_table[i][j][k][l];
                }
                double average = sum / N;
                average_table[i][j][k] = average;
            }
        }
    }
}

void create_city_stats(int R, int C, int A, int N, int T, float average_table[R][C][A], float city_stats[R][C][5])
{
#pragma omp parallel for num_threads(T) collapse(2)
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {

            float min = average_table[i][j][0];
            float max = average_table[i][j][0];
            float median;
            float *aux_array = malloc(A * sizeof(float));
            double sum = 0.0f;

            if (!aux_array)
            {
                fprintf(stderr, "Erro na alocação de memória auxiliar\n");
                exit(1);
            }

// array auxiliar para calcular as demais estatísticas
            for (int k = 0; k < A; k++)
            {
                float val = average_table[i][j][k];
                aux_array[k] = val;
                sum += val;
            }

            qsort(aux_array, A, sizeof(float), float_compare);

            // minimo e maximo
            min = aux_array[0];
            max = aux_array[A - 1];

            // mediana
            if (A % 2 == 0)
                median = (aux_array[A / 2 - 1] + aux_array[A / 2]) / 2.0f;
            else
                median = aux_array[A / 2];

            // calculo da media
            double average = sum / A;

            // calculo do desvio padrão
            double stddev = 0.0f;
            for (int k = 0; k < A; k++)
            {
                double v = aux_array[k] - average;
                stddev += v * v;
            }
            stddev = sqrt(stddev / A);

            city_stats[i][j][0] = min;
            city_stats[i][j][1] = max;
            city_stats[i][j][2] = median;
            city_stats[i][j][3] = average;
            city_stats[i][j][4] = stddev;

            free(aux_array);
        }
    }
}

void create_region_stats(int R, int C, int A, int N, int T, float average_table[R][C][A], float region_stats[R][5])
{
    for (int i = 0; i < R; i++)
    {
        float min = average_table[i][0][0];
        float max = average_table[i][0][0];
        float median;
        float *aux_array = malloc((C * A) * sizeof(float));
        float sum = 0.0f;

        if (!aux_array)
        {
            fprintf(stderr, "Erro na alocação de memória auxiliar\n");
            exit(1);
        }

        // array auxiliar para calcular as demais estatísticas
        int index = 0;

        float *ptr_start = &average_table[i][0][0];
        for (int j = 0; j < C * A; j++)
        {
            float val = *(ptr_start + j);
            aux_array[index++] = val;
            sum += val;
        }

        qsort(aux_array, C * A, sizeof(float), float_compare);

        // minimo e maximo
        min = aux_array[0];
        max = aux_array[C * A - 1];

        // mediana
        if ((C * A) % 2 == 0)
            median = (aux_array[C * A / 2 - 1] + aux_array[C * A / 2]) / 2.0f;
        else
            median = aux_array[C * A / 2];

        // calculo da media
        float average = sum / (C * A);

        // calculo do desvio padrão
        float stddev = 0.0f;
        for (int j = 0; j < C * A; j++)
        {
            float v = aux_array[j] - average;
            stddev += v * v;
        }
        stddev = sqrt(stddev / (C * A));

        region_stats[i][0] = min;
        region_stats[i][1] = max;
        region_stats[i][2] = median;
        region_stats[i][3] = average;
        region_stats[i][4] = stddev;

        free(aux_array);
    }
}

void create_brasil_stats(int R, int C, int A, int N, int T, float average_table[R][C][A], float brasil_stats[5])
{
    float min = average_table[0][0][0];
    float max = average_table[0][0][0];
    float median;
    float *aux_array = malloc((R * C * A) * sizeof(float));
    float sum = 0.0f;

    if (!aux_array)
    {
        fprintf(stderr, "Erro na alocação de memória auxiliar\n");
        exit(1);
    }

    // array auxiliar para calcular as demais estatísticas

    float *src = &average_table[0][0][0]; // Ponteiro para o início da matriz
    for (int n = 0; n < R * C * A; n++)
    {
        float val = src[n];
        aux_array[n] = val;
        sum += val;
    }

    qsort(aux_array, R * C * A, sizeof(float), float_compare);

    // minimo e maximo
    min = aux_array[0];
    max = aux_array[R * C * A - 1];

    // mediana
    if ((R * C * A) % 2 == 0)
        median = (aux_array[R * C * A / 2 - 1] + aux_array[R * C * A / 2]) / 2.0f;
    else
        median = aux_array[R * C * A / 2];

    // calculo da media
    float average = sum / (R * C * A);

    // calculo do desvio padrão
    float stddev = 0.0f;

    for (int i = 0; i < R * C * A; i++)
    {
        float v = aux_array[i] - average;
        stddev += v * v;
    }
    stddev = sqrt(stddev / (R * C * A));

    brasil_stats[0] = min;
    brasil_stats[1] = max;
    brasil_stats[2] = median;
    brasil_stats[3] = average;
    brasil_stats[4] = stddev;

    free(aux_array);
}

void show_table(int R, int C, int A, int N, float avaliation_table[R][C][A][N])
{
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            for (int k = 0; k < A; k++)
            {
                printf("Avaliação da regiao %d da turma %d na atividade %d: ", i, j, k);
                for (int l = 0; l < N; l++)
                {
                    printf("%.2f ", avaliation_table[i][j][k][l]);
                }
                printf("\n");
            }
        }
    }
}

void show_average_table(int R, int C, int A, float average_table[R][C][A])
{
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            printf("Media da regiao %d da turma %d: ", i, j);
            for (int k = 0; k < A; k++)
            {
                printf("%.2f ", average_table[i][j][k]);
            }
            printf("\n");
        }
    }
}

void show_city_stats(int R, int C, float city_stats[R][C][5])
{
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            printf("Estatisticas da regiao %d da turma %d: ", i, j);
            printf("Min: %.2f ", city_stats[i][j][0]);
            printf("Max: %.2f ", city_stats[i][j][1]);
            printf("Median: %.2f ", city_stats[i][j][2]);
            printf("Average: %.2f ", city_stats[i][j][3]);
            printf("Stddev: %.2f ", city_stats[i][j][4]);
            printf("\n");
        }
    }
}

void show_region_stats(int R, float region_stats[R][5])
{
    for (int i = 0; i < R; i++)
    {
        printf("Estatisticas da regiao %d: ", i);
        printf("Min: %.2f ", region_stats[i][0]);
        printf("Max: %.2f ", region_stats[i][1]);
        printf("Median: %.2f ", region_stats[i][2]);
        printf("Average: %.2f ", region_stats[i][3]);
        printf("Stddev: %.2f ", region_stats[i][4]);
        printf("\n");
    }
}

void show_brasil_stats(float brasil_stats[5])
{
    printf("Estatisticas do Brasil: ");
    printf("Min: %.2f ", brasil_stats[0]);
    printf("Max: %.2f ", brasil_stats[1]);
    printf("Median: %.2f ", brasil_stats[2]);
    printf("Average: %.2f ", brasil_stats[3]);
    printf("Stddev: %.2f ", brasil_stats[4]);
    printf("\n");
}

void gera_premiacao(int R, int C, int A, int N, float average_table[R][C][A], float city_stats[R][C][5], float region_stats[R][5], float brasil_stats[5])
{
    // premia a cidade e a regiao com maior média de notas
    int best_city_i = 0, best_city_j = 0;
    float best_city_avg = city_stats[0][0][3];
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            if (city_stats[i][j][3] > best_city_avg)
            {
                best_city_avg = city_stats[i][j][3];
                best_city_i = i;
                best_city_j = j;
            }
        }
    }

    int best_region_i = 0;
    float best_region_avg = region_stats[0][3];
    for (int i = 0; i < R; i++)
    {
        if (region_stats[i][3] > best_region_avg)
        {
            best_region_avg = region_stats[i][3];
            best_region_i = i;
        }
    }

    printf("Premiação:\n");
    printf("Cidade premiada: Região %d, Turma %d com média %.2f\n", best_city_i, best_city_j, best_city_avg);
    printf("Região premiada: Região %d com média %.2f\n", best_region_i, best_region_avg);
}

int main()
{
    int R, C, A, N, T, SEED;
    double tempo_execucao;
    double start;
    double end;

    if (!input_data(&R, &C, &A, &N, &T, &SEED))
    {
        printf("Erro na leitura dos dados\n");
        return 0;
    }

    float (*avaliation_table)[C][A][N] = malloc(sizeof(float[R][C][A][N]));
    float (*average_table)[C][A] = malloc(sizeof(float[R][C][A]));
    float (*city_stats)[C][5] = malloc(sizeof(float[R][C][5])); // 0: min, 1: max, 2: median ,3: average, 4: stddev
    float (*region_stats)[5] = malloc(sizeof(float[R][5]));     // 0: min, 1: max, 2: median ,3: average, 4: stddev
    float *brasil_stats = malloc(5 * sizeof(float));            // 0: min, 1: max, 2: median ,3: average, 4: stddev

    if (!avaliation_table || !average_table || !city_stats || !region_stats || !brasil_stats)
    {
        printf("Erro na alocação de memória\n");
        free(avaliation_table);
        free(average_table);
        free(city_stats);
        free(region_stats);
        free(brasil_stats);
        return 1;
    }


    create_avaliation_table(R, C, A, N, T, SEED, avaliation_table);
    start = omp_get_wtime();
    create_average_table(R, C, A, N, T, avaliation_table, average_table);
    create_city_stats(R, C, A, N, T, average_table, city_stats);
    create_region_stats(R, C, A, N, T, average_table, region_stats);
    create_brasil_stats(R, C, A, N, T, average_table, brasil_stats);
    end = omp_get_wtime();

    // show_table(R, C, A, N, avaliation_table);
    // show_average_table(R, C, A, average_table);
    // show_city_stats(R, C, city_stats);
    // show_region_stats(R, region_stats);
    // show_brasil_stats(brasil_stats);
    gera_premiacao(R, C, A, N, average_table, city_stats, region_stats, brasil_stats);

    tempo_execucao = (end - start); // Calcula o tempo de execução
    printf("Tempo de execução: %.2f segundos\n", tempo_execucao);

    free(avaliation_table);
    free(average_table);
    free(city_stats);
    free(region_stats);
    free(brasil_stats);

    return 0;
}