#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "chrono.h" // Inclui o header para usar o cronômetro

#define MAX_TOTAL_ELEMENTS 16000000

#define MAX_THREADS 16

typedef struct
{
    long long *v;
    int l;
    int r;
} vetor;

typedef struct
{
    vetor input;
    vetor x;
    vetor *result;
    int nThreads;
} data_t;

void threaded_bsearch_lower_bound(data_t *d);

// Função para criar o vetor ordenado
long long *create_sorted_array(int n, int q)
{
    long long *array = (long long *)malloc(n * sizeof(long long));

    if (array == NULL)
    {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }

    srand(time(NULL));

    array[0] = q * (rand() % 100);
    for (int i = 1; i < n; i++)
    {
        array[i] = (long long)(rand() % 10) * q + array[i - 1] + 1;
    }

    return array;
}

void print_array(vetor *v)
{
    for (int i = v->l; i <= v->r; i++)
    {
        printf("%d: %lld ", i, v->v[i]);
    }
    printf("\n");
}

// Função para mesclar dois vetores ordenados em um terceiro vetor ordenado
vetor merge_sorted_vectors(vetor *vet1, vetor *vet2)
{
    int size1 = vet1->r - vet1->l + 1;
    int size2 = vet2->r - vet2->l + 1;

    vetor merged;
    merged.v = (long long *)malloc((size1 + size2) * sizeof(long long));
    if (!merged.v)
    {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }
    merged.l = 0;
    merged.r = size1 + size2 - 1;

    int i = vet1->l, j = vet2->l, k = 0;

    while (i <= vet1->r && j <= vet2->r)
    {
        if (vet1->v[i] <= vet2->v[j])
        {
            merged.v[k++] = vet1->v[i++];
        }
        else
        {
            merged.v[k++] = vet2->v[j++];
        }
    }

    while (i <= vet1->r)
    {
        merged.v[k++] = vet1->v[i++];
    }

    while (j <= vet2->r)
    {
        merged.v[k++] = vet2->v[j++];
    }

    return merged;
}

// Função recursiva para mesclar `n` vetores ordenados
vetor merge_n_sorted_vectors(vetor *arrays, int left, int right)
{
    // Caso base: apenas um vetor a ser mesclado
    if (left == right)
    {
        return arrays[left];
    }

    // Calcula o ponto médio para dividir o trabalho
    int mid = left + (right - left) / 2;

    // Mescla recursivamente as duas metades
    vetor left_merged = merge_n_sorted_vectors(arrays, left, mid);
    vetor right_merged = merge_n_sorted_vectors(arrays, mid + 1, right);

    // Mescla as duas metades mescladas e retorna o resultado
    vetor merged = merge_sorted_vectors(&left_merged, &right_merged);

    // Libera memória dos vetores temporários
    if (left_merged.v != arrays[left].v)
        free(left_merged.v);
    if (right_merged.v != arrays[mid + 1].v)
        free(right_merged.v);

    return merged;
}

void add_result(vetor *v, int n)
{
    v->v[++v->r] = n;
}

int cmp(const void *a, const void *b)
{
    long long int_a = *(const long long *)a;
    long long int_b = *(const long long *)b;

    if (int_a < int_b)
        return -1;
    if (int_a > int_b)
        return 1;
    return 0;
}

// Função para verificar se o vetor 'result' está correto
bool verify_result(vetor *input, vetor *x, vetor *result)
{
    int result_size = result->r - result->l + 1;
    int x_size = x->r - x->l + 1;

    // Verifica se o tamanho de 'result' é igual ao tamanho de 'x'
    if (result_size != x_size)
    {
        printf("Erro: O tamanho do vetor 'result' (%d) não corresponde ao tamanho de 'x' (%d).\n",
               result_size, x_size);
        return false;
    }

    // Verifica se cada valor de 'x' foi associado corretamente no vetor 'input'
    for (int i = 0; i < x_size; i++)
    {
        int idx_result = result->v[i]; // Índice calculado para o valor de 'x'
        long long value_x = x->v[i];   // Valor correspondente em 'x'

        // Verifica se o índice é válido (dentro dos limites de 'input')
        if (idx_result < input->l || idx_result >= input->r + 2)
        {
            printf("Erro: Índice fora do intervalo (%d - %d) válido (%d).\n", input->l, input->r, idx_result);
            return false;
        }

        // Se o índice estiver dentro de 'input', verifica se ele é a posição correta
        if (idx_result <= input->r && input->v[idx_result] < value_x)
        {
            printf("Erro: input[%d] (%lld) é menor que x[%d] (%lld).\n",
                   idx_result, input->v[idx_result], i, value_x);
            return false;
        }

        // Se não for o início do vetor, verifica se o valor anterior é menor que 'x'
        if (idx_result > input->l && input->v[idx_result - 1] >= value_x)
        {
            printf("Erro: input[%d] (%lld) não deveria ser >= x[%d] (%lld).\n",
                   idx_result - 1, input->v[idx_result - 1], i, value_x);
            return false;
        }
    }

    printf("O vetor 'result' está correto.\n");
    return true;
}

// Função de busca binária sequencial
int bsearch_s(vetor *input, long long x)
{
    int l = input->l;
    int r = input->r;

    while (l <= r)
    {
        int mid = l + (r - l) / 2;

        if (input->v[mid] == x)
            return mid;
        else if (input->v[mid] < x)
            l = mid + 1;
        else
            r = mid - 1;
    }

    // Se não encontrou, retorna a posição onde `x` poderia ser inserido
    return l;
}

void div_vetor(data_t *d, data_t *left_d, data_t *right_d)
{
    *left_d = *right_d = *d;
    int mid = d->input.l + (d->input.r - d->input.l) / 2; // Calcula o ponto médio do vetor de entrada

    // Busca o ponto médio no vetor x para dividir a carga de trabalho
    int mid_x = bsearch_s(&d->x, d->input.v[mid]);

    // Corrige `mid_x` para garantir que esteja dentro dos limites de `x`
    if (mid_x > d->x.r)
        mid_x = d->x.r;
    else if (d->x.v[mid_x] > d->input.v[mid])
        mid_x--;

    // Ajusta os limites das sub-regiões para a metade esquerda
    left_d->input.r = mid; // Região esquerda do vetor de entrada
    left_d->x.r = mid_x;   // Região correspondente no vetor x

    // Ajusta os limites das sub-regiões para a metade direita
    right_d->input.l = mid + 1; // Região direita do vetor de entrada
    right_d->x.l = mid_x + 1;   // Região correspondente no vetor
}

void bsearch_lower_bound(data_t *d)
{
    for (int i = d->x.l; i <= d->x.r; i++)
    {
        add_result(d->result, bsearch_s(&d->input, d->x.v[i]));
    }
    return;
}

// Função que será executada pelas threads
void *thread_bsearch_lower_bound(void *arg)
{
    data_t *d = (data_t *)arg;
    bsearch_lower_bound(d); // Chama a busca sequencial
    pthread_exit(NULL);
}

// Função que divide o trabalho em múltiplas threads
void threaded_bsearch_lower_bound(data_t *d)
{
    if (d->nThreads == 0)
    {
        bsearch_lower_bound(d);
        return;
    }

    int x_tam = (d->x.r - d->x.l) / d->nThreads;

    pthread_t threads[MAX_THREADS];
    data_t *nd = (data_t *)malloc(d->nThreads * sizeof(data_t));

    int l, r;
    l = 0;
    r = x_tam - 1;
    vetor *result;

    for (int i = 0; i < d->nThreads - 1; i++)
    {

        nd[i] = *d;
        nd[i].x.l = l;
        nd[i].x.r = r;
        result = (vetor *)malloc(sizeof(vetor));
        result->v = (long long *)malloc((r - l + 1) * sizeof(long long));
        result->l = 0;
        result->r = -1;
        nd[i].result = result;
        pthread_create(&threads[i], NULL, thread_bsearch_lower_bound, (void *)&nd[i]);

        l = r + 1;
        r += x_tam;
    }
    int i = d->nThreads - 1;
    nd[i] = *d;
    nd[i].x.l = l;
    result = (vetor *)malloc(sizeof(vetor));
    result->v = (long long *)malloc((r - l + 1) * sizeof(long long));
    result->l = 0;
    result->r = -1;
    nd[i].result = result;
    pthread_create(&threads[i], NULL, thread_bsearch_lower_bound, (void *)&nd[i]);

    result = (vetor *)malloc(sizeof(vetor));
    result->v = (long long *)malloc((r - l + 1) * sizeof(long long));
    result->l = 0;
    result->r = -1;
    for (int i = 0; i < d->nThreads; i++)
    {
        pthread_join(threads[i], NULL);
        *result = merge_sorted_vectors(result, nd[i].result);
        // print_array(result);
        free(nd[i].result);
    }

    *d->result = *result;

    // Ordena o vetor de resultados ao final da execução
    int result_size = d->result->r - d->result->l + 1;
    qsort(d->result->v, result_size, sizeof(long long), cmp);

    free(nd); // Libera a memória alocada para as divisões
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

    // Preparação para medir o tempo
    chronometer_t chrono;
    chrono_reset(&chrono);

    // Criação dos vetores de entrada e de pesquisa
    vetor input, x, *result = (vetor *)malloc(sizeof(vetor));
    if (!result)
    {
        printf("Erro ao alocar memória para result.\n");
        return 1;
    }

    input.v = create_sorted_array(n, 13);
    if (!input.v)
    {
        printf("Erro ao alocar memória para input.\n");
        free(result);
        return 1;
    }

    x.v = create_sorted_array(m, input.v[n - 1] / m);
    if (!x.v)
    {
        printf("Erro ao alocar memória para x.\n");
        free(input.v);
        free(result);
        return 1;
    }

    input.l = x.l = 0;
    input.r = n - 1;
    x.r = m - 1;

    data_t d;
    d.input = input;
    d.x = x;

    result->v = (long long *)malloc(m * sizeof(long long));
    if (!result->v)
    {
        printf("Erro ao alocar memória para result->v.\n");
        free(input.v);
        free(x.v);
        free(result);
        return 1;
    }

    result->l = 0;
    result->r = -1;
    d.result = result;
    d.nThreads = nThreads; // Define o número de threads com o valor do argumento

        
    
    // Inicia e para a medição de tempo para uma única execução
    chrono_start(&chrono); // Inicia a medição de tempo

    for (int i = 0; i < 10; i++){
        // Executa a busca binária paralela uma vez
        threaded_bsearch_lower_bound(&d);
    }

    chrono_stop(&chrono); // Finaliza a medição de tempo

    // Exibe o tempo total e a vazão
    chrono_reportTime(&chrono, "Tempo de execução para threaded_bsearch_lower_bound");
    double total_time_in_seconds = (double)chrono_gettotal(&chrono) / ((double)1000 * 1000 * 1000);
    printf("Tempo total em segundos:: %lf s\n", total_time_in_seconds);

    double OPS = ((double)m) / total_time_in_seconds;
    printf("Throughput (OPS): %lf OP/s\n", OPS);

    // Liberação de memória
    free(input.v);
    free(x.v);
    free(result->v);
    free(result);

    return 0;
}
