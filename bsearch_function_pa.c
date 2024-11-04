#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "chrono.h" // Inclui a biblioteca de cronometragem

#define MAX_THREADS 8

typedef struct
{
    long long *input;
    long long x;
    int l;
    int r;
    int result;
    int nThreads;
} data_t;

// Função de busca binária sequencial
void bsearch_lower_bound_sequential(data_t *d)
{
    while (d->l < d->r)
    {
        int mid = d->l + (d->r - d->l) / 2;

        if (d->input[mid] < d->x)
        {
            d->l = mid + 1;
        }
        else
        {
            d->r = mid;
        }
    }
    d->result = d->l;
}

// Função que será executada pelas threads
void *thread_bsearch_lower_bound(void *arg)
{
    data_t *d = (data_t *)arg;
    bsearch_lower_bound_sequential(d); // Chama a busca sequencial
    pthread_exit(NULL);
}

// Função que divide o trabalho em múltiplas threads
void threaded_bsearch_lower_bound(data_t *d)
{
    if (d->nThreads <= 1)
    {
        bsearch_lower_bound_sequential(d); // Se não há threads suficientes, faz sequencialmente
        return;
    }

    // Criação de duas estruturas para as sub-regiões da busca
    data_t ld = *d, rd = *d;
    int mid = d->l + (d->r - d->l) / 2;

    ld.r = mid;     // Metade esquerda
    rd.l = mid + 1; // Metade direita

    int t = d->nThreads / 2;

    ld.nThreads = t;
    rd.nThreads = d->nThreads - t;

    pthread_t threads[2];

    // Cria threads para ambas as metades
    pthread_create(&threads[0], NULL, thread_bsearch_lower_bound, (void *)&ld); // ta errado desse jeito
    pthread_create(&threads[1], NULL, thread_bsearch_lower_bound, (void *)&rd);

    // Aguarda as threads finalizarem
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    // Combina o resultado
    d->result = (d->input[ld.result] >= d->x) ? ld.result : rd.result;
}

// Função para criar o vetor ordenado
long long *create_sorted_array(int n)
{
    long long *array = (long long *)malloc(n * sizeof(long long));

    if (array == NULL)
    {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }

    for (int i = 0; i < n; i++)
    {
        array[i] = (long long)i * 13;
    }

    return array;
}

void print_array(long long *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d: %lld ", i, array[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Uso: %s <tamanho de input n> <número de threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);         // Lê o tamanho de input `n` dos argumentos
    int m = 100000;         // Lê o tamanho de x `m` dos argumentos
    int nThreads = atoi(argv[2]);  // Lê o número de threads dos argumentos

    if (nThreads <= 0)
    {
        printf("Erro: O número de threads deve ser positivo.\n");
        return 1;
    }

    if (nThreads > MAX_THREADS)
    {
        printf("Erro: O número de threads deve ser menor que %d.\n", MAX_THREADS);
        return 1;
    }

    // Inicializa o cronômetro
    chronometer_t chrono;
    chrono_reset(&chrono);

    // Prepara o vetor de entrada e configura os dados da busca
    data_t d;
    d.input = create_sorted_array(n);  // Cria o vetor de entrada ordenado
    d.l = 0;
    d.r = n - 1;
    d.nThreads = nThreads;

    long long *x = create_sorted_array(m);  // Cria o vetor de valores a serem buscados

    // Inicia a medição de tempo
    chrono_start(&chrono);

    for (int i = 0; i < 10; i++){
        // Executa a busca binária paralela para cada valor em `x`
        for (int i = 0; i < m; i++)
        {
            d.x = x[i]; // Define o valor a ser buscado
            threaded_bsearch_lower_bound(&d);
        }
    }

    // Para a medição de tempo
    chrono_stop(&chrono);

        // Exibe o tempo total e a vazão
    chrono_reportTime(&chrono, "Tempo de execução para threaded_bsearch_lower_bound");
    double total_time_in_seconds = (double)chrono_gettotal(&chrono) / ((double)1000 * 1000 * 1000);
    printf("Tempo total em segundos: %lf s\n", total_time_in_seconds);

    double OPS = ((double)m) / total_time_in_seconds;
    printf("Throughput (OPS): %lf OP/s\n", OPS);

    // Liberação de memória
    free(d.input);
    free(x);

    return 0;
}