/**
 * @file studentsseq.c
 * @brief Implementação sequencial para o trabalho 1 de Computação de Alto Desempenho.
 *
 * @author Bruno Rusca Janini (13674917)
 * @author Matheus Aguiar (14781512)
 * @author Renan Correia Monteiro Soares (14605661)
 * @author Vitor Alexandre Garcia Vaz (14611432)
 *
 * @attention O código exige a leitura de um arquivo de entrada, contendo os parâmetros de execução
 *            do programa, e o caminho para esse arquivo deve ser especificado na macro ARQ_DIR,
 *            presente no início do código (na seção de Defines).
 * @attention O código foi desenvolvido com uso da biblioteca de matemática math.h, pressupondo
 *            assim, para a execução do programa, o uso da flag -lm.
 */

/*============================ Inclusão de bibliotecas ===========================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <stdbool.h>

/**
 * @brief Lê e valida os parâmetros de entrada do programa.
 *
 * @param argc Número de argumentos passados no terminal.
 * @param argv Vetor de strings com os argumentos.
 * @param R Ponteiro para armazenar o número de regiões.
 * @param C Ponteiro para armazenar o número de turmas.
 * @param A Ponteiro para armazenar o número de atividades.
 * @param N Ponteiro para armazenar o número de avaliações por atividade.
 * @param T Ponteiro para armazenar o número de threads.
 * @param SEED Ponteiro para armazenar a semente do gerador aleatório.
 * @return 1 se os parâmetros forem válidos, 0 caso contrário.
 */
bool read_input_from_file(const char *filename, int *R, int *C, int *A, int *N, int *T, int *SEED)
{
    // Tenta abrir o arquivo no modo de leitura ("r")
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro: Não foi possível abrir o arquivo '%s'.\n", filename);
        return false;
    }

    // Lê os 6 valores inteiros.
    // fscanf retorna a quantidade de variáveis lidas com sucesso.
    int itens_lidos = fscanf(file, "%d %d %d %d %d %d", R, C, A, N, T, SEED);

    // Fecha o arquivo para liberar memória
    fclose(file);

    // Valida se o arquivo tinha exatamente a quantidade de parâmetros esperada
    if (itens_lidos != 6)
    {
        printf("Erro: Arquivo com formato inválido. Esperados 6 valores, mas foram lidos %d.\n", itens_lidos);
        return false;
    }

    return true;
}

/**
 * @brief Particiona um vetor em torno de um pivô para quickselect/quick sort.
 *
 * @param arr Vetor de valores float.
 * @param low Índice inicial do segmento.
 * @param high Índice final do segmento.
 * @return Índice final do pivô após a partição.
 */
int partition(float arr[], int low, int high)
{
    float pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++)
    {
        if (arr[j] <= pivot)
        {
            i++;
            float temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    float temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

// Quickselect recursivo para encontrar o k-ésimo menor elemento (0-based)
/**
 * @brief Seleciona o k-ésimo menor elemento em um vetor.
 *
 * @param arr Vetor de valores float.
 * @param low Índice inicial do segmento.
 * @param high Índice final do segmento.
 * @param k Posição desejada (0-based) do elemento.
 * @return O valor do k-ésimo menor elemento.
 */
float quickselect(float arr[], int low, int high, int k)
{
    if (low == high)
        return arr[low];

    int pivotIndex = partition(arr, low, high);

    if (k == pivotIndex)
        return arr[k];
    else if (k < pivotIndex)
        return quickselect(arr, low, pivotIndex - 1, k);
    else
        return quickselect(arr, pivotIndex + 1, high, k);
}

/**
 * @brief Gera a tabela de avaliações com valores aleatórios.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param T Número de threads (não utilizado no modo sequencial).
 * @param SEED Semente para geração pseudoaleatória.
 * @param avaliation_table Tabela de saída de avaliações.
 * @return 1 em caso de sucesso.
 */
int create_avaliation_table(int R, int C, int A, int N, int T, int SEED, float avaliation_table[R][C][A][N])
{
    srand(SEED);

    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            for (int k = 0; k < A; k++)
            {
                for (int l = 0; l < N; l++)
                {
                    avaliation_table[i][j][k][l] = (float)(rand() % 1001) / 10.0;
                }
            }
        }
    }
    return 1;
}

/**
 * @brief Calcula a média das avaliações por atividade.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param T Número de threads para paralelização.
 * @param avaliation_table Tabela de avaliações de entrada.
 * @param average_table Tabela de médias por atividade.
 */
void create_average_table(int R, int C, int A, int N, int T, float avaliation_table[restrict R][C][A][N], float average_table[restrict R][C][A])
{
    // para cada região, turma e atividade
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            for (int k = 0; k < A; k++)
            {
                // calcula a média das avaliações usando uma redução para somar os valores
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

/**
 * @brief Calcula estatísticas por cidade usando médias de atividades.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param T Número de threads (não utilizado diretamente).
 * @param average_table Tabela de médias de cada atividade.
 * @param city_stats Matriz de saída com min, max, mediana, média e desvio padrão.
 */
void create_city_stats(int R, int C, int A, int N, int T, float average_table[restrict R][C][A], float city_stats[restrict R][C][5],
                       int *best_city_i, int *best_city_j, float *best_city_avg)
{
    float *aux_array = malloc(A * sizeof(float));
    if (!aux_array)
    {
        fprintf(stderr, "Erro na alocação de memória\n");
        exit(1);
    }

    // Divide o trabalho do loop entre as threads
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            float min = average_table[i][j][0];
            float max = average_table[i][j][0];
            float median;
            double sum = 0.0f;
            double sum_sq = 0.0f;

            // Reaproveita a memória alocada fora do loop
            for (int k = 0; k < A; k++)
            {
                float val = average_table[i][j][k];
                aux_array[k] = val;
                sum += val;
                sum_sq += val * val;
                if (val < min)
                    min = val;
                if (val > max)
                    max = val;
            }

            // cálculo da mediana usando quickselect
            if (A % 2 == 0)
            {
                float m1 = quickselect(aux_array, 0, A - 1, A / 2);         // procura o A/2-1 menor elemento
                float m2 = quickselect(aux_array, 0, A / 2 - 1, A / 2 - 1); // procura o A/2 menor elemento
                median = (m1 + m2) / 2.0f;
            }
            else
            {
                median = quickselect(aux_array, 0, A - 1, A / 2); // procura o A/2 menor elemento
            }

            // calculo do desvio padrão e da média e armazenamento dos resultados
            double average = sum / A;
            float stddev = sqrt((sum_sq - (sum * sum) / A) / (A - 1));
            city_stats[i][j][0] = min;
            city_stats[i][j][1] = max;
            city_stats[i][j][2] = median;
            city_stats[i][j][3] = average;
            city_stats[i][j][4] = stddev;

            if (average > *best_city_avg)
            {
                *best_city_avg = average;
                *best_city_i = i;
                *best_city_j = j;
            }
        }
    }

    // 4. Libera a memória
    free(aux_array);
}

/**
 * @brief Calcula estatísticas por região agregando todas as turmas e atividades.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param T Número de threads (não utilizado diretamente).
 * @param average_table Tabela de médias de cada atividade.
 * @param region_stats Matriz de saída com min, max, mediana, média e desvio padrão.
 */
void create_region_stats(int R, int C, int A, int N, int T, float average_table[restrict R][C][A], float region_stats[restrict R][5],
                         int *best_region_i, float *best_region_avg)
{
    // 1. Abre a região paralela (cria as T threads, mas ainda não inicia o loop)

    // 2. Cada thread faz UMA única alocação no Heap para processar todas as regiões que receber
    float *aux_array = malloc((C * A) * sizeof(float));

    if (!aux_array)
    {
        fprintf(stderr, "Erro na alocação de memória auxiliar\n");
        exit(1);
    }

    // 3. Divide o loop das regiões (R) entre as threads criadas
    for (int i = 0; i < R; i++)
    {
        // inicialização das variáveis estatísticas
        float min = average_table[i][0][0];
        float max = average_table[i][0][0];
        float median;
        double sum = 0.0;
        double sum_sq = 0.0;

        float *ptr_start = &average_table[i][0][0];
        for (int j = 0; j < C * A; j++)
        {
            float val = *(ptr_start + j);
            aux_array[j] = val;
            sum += val;
            sum_sq += (val * val);
            if (val < min)
                min = val;
            if (val > max)
                max = val;
        }

        // mediana usando quickselect (reaproveitando a memória alocada fora do loop)
        if ((C * A) % 2 == 0)
        {
            float m1 = quickselect(aux_array, 0, C * A - 1, C * A / 2);
            float m2 = quickselect(aux_array, 0, C * A / 2 - 1, C * A / 2 - 1);
            median = (m1 + m2) / 2.0f;
        }
        else
        {
            median = quickselect(aux_array, 0, C * A - 1, C * A / 2);
        }

        // calculo do desvio padrão
        double average = sum / (C * A);
        double stddev = sqrt((sum_sq - (sum * sum) / (C * A)) / (C * A - 1));

        region_stats[i][0] = min;
        region_stats[i][1] = max;
        region_stats[i][2] = median;
        region_stats[i][3] = average;
        region_stats[i][4] = stddev;

        if (average > *best_region_avg)
        {
            *best_region_avg = average;
            *best_region_i = i;
        }
    }

    free(aux_array);
}

/**
 * @brief Calcula estatísticas agregadas para todo o Brasil.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param T Número de threads (não utilizado diretamente).
 * @param average_table Tabela de médias de cada atividade.
 * @param brasil_stats Vetor de saída com min, max, mediana, média e desvio padrão.
 */
void create_brasil_stats(int R, int C, int A, int N, int T, float average_table[restrict R][C][A], float brasil_stats[5])
{
    // inicialização das variáveis estatísticas
    float min = average_table[0][0][0];
    float max = average_table[0][0][0];
    float median;
    float sum = 0.0f;
    double sum_sq = 0.0f;
    int total = R * C * A;

    // alocação de memória para o array auxiliar usado no cálculo da mediana
    float *aux_array = malloc((R * C * A) * sizeof(float));
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
        sum_sq += (val * val);
        if (val < min)
            min = val;
        if (val > max)
            max = val;
    }

    if ((R * C * A) % 2 == 0)
    {
        float m1 = quickselect(aux_array, 0, R * C * A - 1, R * C * A / 2);
        float m2 = quickselect(aux_array, 0, R * C * A / 2 - 1, R * C * A / 2 - 1);
        median = (m1 + m2) / 2.0f;
    }
    else
    {
        median = quickselect(aux_array, 0, R * C * A - 1, R * C * A / 2);
    }

    // calculo do desvio padrão e da média
    double average = sum / (R * C * A);
    double stddev = sqrt((sum_sq - (sum * sum) / (R * C * A)) / (R * C * A - 1));

    // Armazenamento dos resultados na matriz de saída e liberação da memória auxiliar
    brasil_stats[0] = min;
    brasil_stats[1] = max;
    brasil_stats[2] = median;
    brasil_stats[3] = average;
    brasil_stats[4] = stddev;
    free(aux_array);
}

/**
 * @brief Exibe a tabela de avaliações completas.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param avaliation_table Tabela de avaliações de entrada.
 */
#include <stdio.h>

/**
 * @brief Exibe a tabela de avaliações completas no formato do Apêndice A.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades (alunos).
 * @param N Número de avaliações por atividade.
 * @param avaliation_table Tabela de avaliações de entrada.
 */
void show_table(int R, int C, int A, int N, float avaliation_table[R][C][A][N])
{
    printf("Notas individuais/parciais geradas pseudo-aleatoriamente por aluno, em ordem de regiões e cidades\n");
    for (int i = 0; i < R; i++)
    {
        printf("\nRegião %d\n", i);

        // Cabeçalho das notas
        printf("\t\t");
        for (int l = 0; l < N; l++)
        {
            printf("Nota %d\t\t", l);
        }
        printf("\n");

        for (int j = 0; j < C; j++)
        {
            for (int k = 0; k < A; k++)
            {
                printf("R=%d, C=%d, A=%d\t", i, j, k);
                for (int l = 0; l < N; l++)
                {
                    printf("%.1f\t\t", avaliation_table[i][j][k][l]);
                }
                printf("\n");
            }
        }
    }
}

/**
 * @brief Exibe a tabela de médias por aluno.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades (alunos).
 * @param average_table Tabela de médias de entrada.
 */
void show_average_table(int R, int C, int A, float average_table[R][C][A])
{
    printf("\nNotas médias dos alunos. Cálculos para determinar os dados por Brasil, regiões e cidades.\n\n");

    // Cabeçalho dos alunos
    printf("\t\t");
    for (int k = 0; k < A; k++)
    {
        printf("Aluno %d\t\t", k);
    }
    printf("\n");

    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            printf("R=%d, C=%d\t", i, j);
            for (int k = 0; k < A; k++)
            {
                printf("%.1f\t\t", average_table[i][j][k]);
            }
            printf("\n");
        }
    }
}

/**
 * @brief Exibe as estatísticas calculadas por cidade.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param city_stats Matriz de estatísticas por cidade.
 */
void show_city_stats(int R, int C, float city_stats[R][C][5])
{
    printf("\nTabelas para a saída: consideram as notas médias dos alunos.\n\n");
    printf("%-12s\tMin Nota\tMax Nota\tMediana\t\tMédia\t\tDsvPdr\n", "Cidades");

    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            printf("R=%d, C=%d\t", i, j);
            printf("%.1f\t\t%.1f\t\t%.1f\t\t%.1f\t\t%.1f\n",
                   city_stats[i][j][0], city_stats[i][j][1],
                   city_stats[i][j][2], city_stats[i][j][3],
                   city_stats[i][j][4]);
        }
    }
}

/**
 * @brief Exibe as estatísticas calculadas por região.
 *
 * @param R Número de regiões.
 * @param region_stats Matriz de estatísticas por região.
 */
void show_region_stats(int R, float region_stats[R][5])
{
    printf("\n%-12s\tMin Nota\tMax Nota\tMediana\t\tMédia\t\tDsvPdr\n", "Regiões");
    for (int i = 0; i < R; i++)
    {
        printf("R=%d\t\t", i);
        printf("%.1f\t\t%.1f\t\t%.1f\t\t%.1f\t\t%.1f\n",
               region_stats[i][0], region_stats[i][1],
               region_stats[i][2], region_stats[i][3],
               region_stats[i][4]);
    }
}

/**
 * @brief Exibe as estatísticas agregadas do Brasil.
 *
 * @param brasil_stats Vetor de estatísticas do Brasil.
 */
void show_brasil_stats(float brasil_stats[5])
{
    printf("\n%-12s\tMin Nota\tMax Nota\tMediana\t\tMédia\t\tDsvPdr\n", "Brasil");
    printf("\t\t%.1f\t\t%.1f\t\t%.1f\t\t%.1f\t\t%.1f\n",
           brasil_stats[0], brasil_stats[1],
           brasil_stats[2], brasil_stats[3],
           brasil_stats[4]);
}

/**
 * @brief Exibe na tela os resultados da premiação formatados.
 */
void printa_premiacao(int best_city_i, int best_city_j, float best_city_avg,
                      int best_region_i, float best_region_avg)
{
    printf("\n%-15s\tReg/Cid\t\tMedia Arit\n", "Premiação");
    printf("Melhor região:\tR%d\t\t%.1f\n", best_region_i, best_region_avg);
    printf("Melhor cidade:\tR%d-C%d\t\t%.1f\n\n", best_city_i, best_city_j, best_city_avg);
}

/**
 * @brief Ponto de entrada do programa.
 *
 * @param argc Número de argumentos passados no terminal.
 * @param argv Vetor de argumentos.
 * @return Código de saída do programa.
 */
int main(int argc, char *argv[])
{
    int R, C, A, N, T, SEED;
    double tempo_execucao;
    double start;
    double end;

    int best_city_i, best_city_j, best_region_i;
    float best_city_avg, best_region_avg;

    // 1. Verifica se o usuário passou o nome do arquivo na linha de comando
    if (argc < 2)
    {
        printf("Uso correto: %s <arquivo_de_entrada.txt>\n", argv[0]);
        return 1; // Retorna 1 para indicar erro de execução
    }

    // 2. Chama a nova função passando o arquivo (argv[1]) e o endereço das variáveis
    if (!read_input_from_file(argv[1], &R, &C, &A, &N, &T, &SEED))
    {
        printf("Erro na leitura dos dados do arquivo.\n");
        return 1;
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
    create_city_stats(R, C, A, N, T, average_table, city_stats,
                      &best_city_i, &best_city_j, &best_city_avg);
    create_region_stats(R, C, A, N, T, average_table, region_stats,
                        &best_region_i, &best_region_avg);
    create_brasil_stats(R, C, A, N, T, average_table, brasil_stats);
    end = omp_get_wtime();

    // show_table(R, C, A, N, avaliation_table);
    // show_average_table(R, C, A, average_table);
    show_city_stats(R, C, city_stats);
    show_region_stats(R, region_stats);
    show_brasil_stats(brasil_stats);
    printa_premiacao(best_city_i, best_city_j, best_city_avg,
                     best_region_i, best_region_avg);

    tempo_execucao = (end - start); // Calcula o tempo de execução
    printf("Tempo de execução: %.6f segundos\n", tempo_execucao);

    free(avaliation_table);
    free(average_table);
    free(city_stats);
    free(region_stats);
    free(brasil_stats);

    return 0;
}