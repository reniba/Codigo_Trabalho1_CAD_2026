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


// Definições de constantes para o benchmark
#define MAX_LINHAS_CSV 100
#define NUM_EXECUCOES 10
#define NUM_PROCESSOS_ANALISE 5
#define ARQ_PARAMETROS_TABELAS "parametros.csv"
#define ARQ_SAIDA_TABELA "resultados_desempenho.csv"
#define DEBUG 1

// Funções utilitárias para estatística
double calcularMedia(double tempos[], int n) {
    double soma = 0.0;
    for(int i = 0; i < n; i++) soma += tempos[i];
    return soma / n;
}

double calcularDesvioPadrao(double tempos[], int n, double media) {
    double soma_quadrados = 0.0;
    for(int i = 0; i < n; i++) {
        soma_quadrados += (tempos[i] - media) * (tempos[i] - media);
    }
    return n > 1 ? sqrt(soma_quadrados / (n - 1)) : 0.0;
}

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
int input_data(int argc, char *argv[], int *R, int *C, int *A, int *N, int *T, int *SEED)
{
    if (argc != 7)
    {
        printf("Uso: %s <R> <C> <A> <N> <T> <SEED>\n", argv[0]);
        printf("Exemplo: %s 5 3 4 10 4 12345\n", argv[0]);
        return 0;
    }

    *R = atoi(argv[1]);
    *C = atoi(argv[2]);
    *A = atoi(argv[3]);
    *N = atoi(argv[4]);
    *T = atoi(argv[5]);
    *SEED = atoi(argv[6]);

    if (*R <= 0 || *C <= 0 || *A <= 0 || *N <= 0 || *T <= 0)
    {
        printf("Erro: R, C, A, N e T devem ser maiores que zero\n");
        return 0;
    }

    return 1;
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
#pragma omp parallel for num_threads(T) collapse(3)
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            for (int k = 0; k < A; k++)
            {
                // calcula a média das avaliações usando uma redução para somar os valores
                double sum = 0.0f;
#pragma omp simd reduction(+ : sum)
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
void create_city_stats(int R, int C, int A, int N, int T, float average_table[restrict R][C][A], float city_stats[restrict R][C][5])
{
// Abre a região paralela sem iniciar o loop 'for' ainda
#pragma omp parallel num_threads(T)
    {
        // Cada thread aloca seu espaço de trabalho UMA ÚNICA VEZ
        float *aux_array = malloc(A * sizeof(float));
        float *temp_array = malloc(A * sizeof(float));

        if (!aux_array || !temp_array)
        {
            fprintf(stderr, "Erro na alocação de memória\n");
            exit(1);
        }

// Divide o trabalho do loop entre as threads
#pragma omp for collapse(2)
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
#pragma omp simd reduction(+ : sum, sum_sq)
                for (int k = 0; k < A; k++)
                {
                    float val = average_table[i][j][k];
                    aux_array[k] = val;
                    sum += val;
                    sum_sq += (val * val);
                    if (val < min)
                        min = val;
                    if (val > max)
                        max = val;
                }

                // cálculo da mediana usando quickselect
                // memcpy(temp_array, aux_array, A * sizeof(float));
                if (A % 2 == 0)
                {
                    float m1 = quickselect(aux_array, 0, A - 1, A / 2); // procura o A/2-1 menor elemento
                    // memcpy(temp_array, aux_array, A * sizeof(float));        //  precisa copiar novamente porque o quickselect modifica o array
                    float m2 = quickselect(aux_array, 0, A / 2 - 1, A / 2 - 1); // procura o A/2 menor elemento
                    median = (m1 + m2) / 2.0f;
                }
                else
                {
                    median = quickselect(aux_array, 0, A - 1, A / 2); // procura o A/2 menor elemento
                }

                // calculo do desvio padrão e da média e armazenamento dos resultados
                double average = sum / A;
                float stddev = sqrt(sum_sq - (sum * sum) / A);
                city_stats[i][j][0] = min;
                city_stats[i][j][1] = max;
                city_stats[i][j][2] = median;
                city_stats[i][j][3] = average;
                city_stats[i][j][4] = stddev;
            }
        }

        // 4. Libera a memória no final da região paralela (apenas T vezes)
        free(aux_array);
        free(temp_array);
    }
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
void create_region_stats(int R, int C, int A, int N, int T, float average_table[restrict R][C][A], float region_stats[restrict R][5])
{
// 1. Abre a região paralela (cria as T threads, mas ainda não inicia o loop)
#pragma omp parallel num_threads(T)
    {
        // 2. Cada thread faz UMA única alocação no Heap para processar todas as regiões que receber
        float *aux_array = malloc((C * A) * sizeof(float));
        float *temp_array_region = malloc((C * A) * sizeof(float));

        if (!aux_array || !temp_array_region)
        {
            fprintf(stderr, "Erro na alocação de memória auxiliar\n");
            exit(1);
        }

// 3. Divide o loop das regiões (R) entre as threads criadas
#pragma omp for
        for (int i = 0; i < R; i++)
        {
            // inicialização das variáveis estatísticas
            float min = average_table[i][0][0];
            float max = average_table[i][0][0];
            float median;

            // Otimização: Mudei 'sum' para double. Como C * A pode ser um número grande,
            // usar float para o somatório causaria perda de precisão (catastrophic cancellation).
            double sum = 0.0;
            double sum_sq = 0.0;

            float *ptr_start = &average_table[i][0][0];

#pragma omp simd reduction(+ : sum, sum_sq)
            for (int j = 0; j < C * A; j++)
            {
                float val = *(ptr_start + j);

                // Otimização: Substituí o 'index++' por 'j'.
                // Incrementar variáveis fora do escopo do 'for' quebra a vetorização SIMD estrita.
                aux_array[j] = val;

                sum += val;
                sum_sq += (val * val);
                if (val < min)
                    min = val;
                if (val > max)
                    max = val;
            }

            // mediana usando quickselect (reaproveitando a memória alocada fora do loop)
            // memcpy(temp_array_region, aux_array, (C * A) * sizeof(float));
            if ((C * A) % 2 == 0)
            {
                float m1 = quickselect(aux_array, 0, C * A - 1, C * A / 2);
                // memcpy(temp_array_region, aux_array, (C * A) * sizeof(float));
                float m2 = quickselect(aux_array, 0, C * A / 2 - 1, C * A / 2 - 1);
                median = (m1 + m2) / 2.0f;
            }
            else
            {
                median = quickselect(aux_array, 0, C * A - 1, C * A / 2);
            }

            // calculo do desvio padrão
            double average = sum / (C * A);
            double stddev = sqrt(sum_sq - (sum * sum) / (C * A));

            region_stats[i][0] = min;
            region_stats[i][1] = max;
            region_stats[i][2] = median;
            region_stats[i][3] = average;
            region_stats[i][4] = stddev;
        }

        // 4. Cada thread libera sua memória ao finalizar toda a sua parte do trabalho
        free(aux_array);
        free(temp_array_region);
    }
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
#pragma omp parallel for simd num_threads(T) reduction(+ : sum, sum_sq)
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

    //float *temp_array_brasil = malloc((R * C * A) * sizeof(float));
    // memcpy(temp_array_brasil, aux_array, (R * C * A) * sizeof(float));
    if ((R * C * A) % 2 == 0)
    {
        float m1 = quickselect(aux_array, 0, R * C * A - 1, R * C * A / 2);
        // memcpy(temp_array_brasil, aux_array, (R * C * A) * sizeof(float));
        float m2 = quickselect(aux_array, 0, R * C * A / 2 - 1, R * C * A / 2 - 1);
        median = (m1 + m2) / 2.0f;
    }
    else
    {
        median = quickselect(aux_array, 0, R * C * A - 1, R * C * A / 2);
    }
    //free(temp_array_brasil);
    median = aux_array[R * C * A / 2];

    // calculo do desvio padrão e da média
    double average = sum / (R * C * A);
    double stddev = sqrt(sum_sq - (sum * sum) / (R * C * A));

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

/**
 * @brief Exibe a tabela de médias por atividade.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param average_table Tabela de médias de entrada.
 */
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

/**
 * @brief Exibe as estatísticas calculadas por cidade.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param city_stats Matriz de estatísticas por cidade.
 */
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

/**
 * @brief Exibe as estatísticas calculadas por região.
 *
 * @param R Número de regiões.
 * @param region_stats Matriz de estatísticas por região.
 */
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

/**
 * @brief Exibe as estatísticas agregadas do Brasil.
 *
 * @param brasil_stats Vetor de estatísticas do Brasil.
 */
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

/**
 * @brief Identifica e exibe as melhores cidades e regiões para premiação.
 *
 * @param R Número de regiões.
 * @param C Número de turmas.
 * @param A Número de atividades.
 * @param N Número de avaliações por atividade.
 * @param T Número de threads.
 * @param average_table Tabela de médias por atividade.
 * @param city_stats Estatísticas por cidade.
 * @param region_stats Estatísticas por região.
 * @param brasil_stats Estatísticas do Brasil.
 */
void gera_premiacao(int R, int C, int A, int N, int T, float average_table[R][C][A], float city_stats[R][C][5], float region_stats[R][5], float brasil_stats[5])
{
    int best_city_i = 0, best_city_j = 0;
    float best_city_avg = city_stats[0][0][3];

    // Busca sequencial da melhor cidade
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

    // Busca sequencial da melhor região
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

/**
 * @brief Ponto de entrada do programa.
 *
 * @param argc Número de argumentos passados no terminal.
 * @param argv Vetor de argumentos.
 * @return Código de saída do programa.
 */
int main() {
    const int NUM_THREADS_MIN = 1;
    const int NUM_THREADS_MAX = 12;
    
    FILE* arquivoParametros = NULL;
    FILE* arquivoSaidaTabela = NULL;
    
    long parametrosR[MAX_LINHAS_CSV], parametrosC[MAX_LINHAS_CSV];
    long parametrosA[MAX_LINHAS_CSV], parametrosN[MAX_LINHAS_CSV];
    int parametrosSeed[MAX_LINHAS_CSV];
    int numConfiguracoes = 0;
    
    printf("=== Leitura de Configurações do CSV ===\n\n");
    
    arquivoParametros = fopen(ARQ_PARAMETROS_TABELAS, "r");
    if(arquivoParametros == NULL){
        printf("Erro ao abrir arquivo de configurações: %s\n", ARQ_PARAMETROS_TABELAS);
        return 1;
    }
    
    char linha[256];
    if(fgets(linha, sizeof(linha), arquivoParametros) == NULL){
        fclose(arquivoParametros);
        return 1;
    }
    
    while(fgets(linha, sizeof(linha), arquivoParametros) != NULL && numConfiguracoes < MAX_LINHAS_CSV){
        if(sscanf(linha, "%ld,%ld,%ld,%ld,%d", 
                  &parametrosR[numConfiguracoes], &parametrosC[numConfiguracoes], 
                  &parametrosA[numConfiguracoes], &parametrosN[numConfiguracoes], 
                  &parametrosSeed[numConfiguracoes]) == 5){
            numConfiguracoes++;
        }
    }
    fclose(arquivoParametros);
    
    printf("Total de configurações carregadas: %d\n", numConfiguracoes);
    printf("Iniciando testes (Threads de %d a %d)...\n\n", NUM_THREADS_MIN, NUM_THREADS_MAX);
    
    arquivoSaidaTabela = fopen(ARQ_SAIDA_TABELA, "w");
    if(arquivoSaidaTabela == NULL) return 1;
    
    // Cabeçalho com a nova coluna "Threads"
    fprintf(arquivoSaidaTabela, "R,C,A,N,Threads");
    const char* nomesProcessos[] = {"Analise0", "Analise1", "Analise2", "Analise3", "Analise4", "Analise5"};
    for(int proc = 0; proc < NUM_PROCESSOS_ANALISE; proc++){
        fprintf(arquivoSaidaTabela, ",%s_Media(us),%s_DevPad(us)", nomesProcessos[proc], nomesProcessos[proc]);
    }
    fprintf(arquivoSaidaTabela, ",Total_Media(us),Total_DevPad(us)\n");
    fclose(arquivoSaidaTabela);
    
    for(int config = 0; config < numConfiguracoes; config++){
        long R = parametrosR[config], C = parametrosC[config], A = parametrosA[config], N = parametrosN[config];
        int seed = parametrosSeed[config];

        printf("Processando Configuração %d (R=%ld, C=%ld, A=%ld, N=%ld)...\n", config + 1, R, C, A, N);
        
        // Alocação dinâmica única para a configuração
        float (*avaliation_table)[C][A][N] = malloc(sizeof(float[R][C][A][N]));
        float (*average_table)[C][A] = malloc(sizeof(float[R][C][A]));
        float (*city_stats)[C][5] = malloc(sizeof(float[R][C][5])); 
        float (*region_stats)[5] = malloc(sizeof(float[R][5]));     
        float *brasil_stats = malloc(5 * sizeof(float));            

        if (!avaliation_table || !average_table) {
            printf("Erro de memória. Pulando...\n");
            continue;
        }

        // Geração da tabela base (com T=1 pois é apenas a geração dos dados)
        create_avaliation_table(R, C, A, N, 1, seed, avaliation_table);
        
        // Matriz para guardar os resultados de cada número de threads para esta configuração
        // Estrutura: [Threads][Execucao][Processo]
        // Como varia de 1 a 12, usaremos threads-1 como índice
        double temposAnalises[NUM_THREADS_MAX][NUM_EXECUCOES][NUM_PROCESSOS_ANALISE];

        for(int t = NUM_THREADS_MIN; t <= NUM_THREADS_MAX; t++){
            if(DEBUG) printf("  -> Testando com %d threads...\n", t);
            
            for(int i = 0; i < NUM_EXECUCOES; i++){
                double start, end;

                // Analise 0: Average Table
                start = omp_get_wtime();
                create_average_table(R, C, A, N, t, avaliation_table, average_table);
                end = omp_get_wtime();
                temposAnalises[t-1][i][0] = (end - start) * 1000000.0;

                // Analise 1: City Stats
                start = omp_get_wtime();
                create_city_stats(R, C, A, N, t, average_table, city_stats);
                end = omp_get_wtime();
                temposAnalises[t-1][i][1] = (end - start) * 1000000.0;

                // Analise 2: Region Stats
                start = omp_get_wtime();
                create_region_stats(R, C, A, N, t, average_table, region_stats);
                end = omp_get_wtime();
                temposAnalises[t-1][i][2] = (end - start) * 1000000.0;

                // Analise 3: Brasil Stats
                start = omp_get_wtime();
                create_brasil_stats(R, C, A, N, t, average_table, brasil_stats);
                end = omp_get_wtime();
                temposAnalises[t-1][i][3] = (end - start) * 1000000.0;

                // Analise 4: Premiação
                start = omp_get_wtime();
                gera_premiacao(R, C, A, N, t, average_table, city_stats, region_stats, brasil_stats);
                end = omp_get_wtime();
                temposAnalises[t-1][i][4] = (end - start) * 1000000.0;

                // Analise 5: Extra/Livre
                temposAnalises[t-1][i][5] = 0.0;
            }
        }

        // Escrita dos resultados desta configuração para todas as threads
        arquivoSaidaTabela = fopen(ARQ_SAIDA_TABELA, "a");
        for(int t = NUM_THREADS_MIN; t <= NUM_THREADS_MAX; t++){
            fprintf(arquivoSaidaTabela, "%ld,%ld,%ld,%ld,%d", R, C, A, N, t);
            
            double temposTotais[NUM_EXECUCOES];
            
            for(int proc = 0; proc < NUM_PROCESSOS_ANALISE; proc++){
                double amostrasProc[NUM_EXECUCOES];
                for(int exec = 0; exec < NUM_EXECUCOES; exec++){
                    amostrasProc[exec] = temposAnalises[t-1][exec][proc];
                }
                double m = calcularMedia(amostrasProc, NUM_EXECUCOES);
                double d = calcularDesvioPadrao(amostrasProc, NUM_EXECUCOES, m);
                fprintf(arquivoSaidaTabela, ",%.2f,%.2f", m, d);
            }

            // Cálculo do Total
            for(int exec = 0; exec < NUM_EXECUCOES; exec++){
                double somaTotal = 0;
                for(int proc = 0; proc < NUM_PROCESSOS_ANALISE; proc++) somaTotal += temposAnalises[t-1][exec][proc];
                temposTotais[exec] = somaTotal;
            }
            double mt = calcularMedia(temposTotais, NUM_EXECUCOES);
            double dt = calcularDesvioPadrao(temposTotais, NUM_EXECUCOES, mt);
            fprintf(arquivoSaidaTabela, ",%.2f,%.2f\n", mt, dt);
        }
        fclose(arquivoSaidaTabela);
        
        // Liberação de memória antes da próxima configuração de R,C,A,N
        free(avaliation_table); free(average_table); free(city_stats); free(region_stats); free(brasil_stats);
    }

    printf("\nBenchmark concluído com sucesso. Resultados em %s\n", ARQ_SAIDA_TABELA);
    return 0;
}