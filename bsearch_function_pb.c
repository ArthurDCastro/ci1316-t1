#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define MAX_THREADS 8

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
        array[i] = (long long)i * (rand() % 10) * q + array[i - 1] + 1;
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

// Função de busca binária sequencial
int bsearch_s(vetor *input, long long x)
{
    int l, r;
    l = input->l;
    r = input->r;
    while (l < r)
    {
        int mid = l + (r - l) / 2;
        if (input->v[mid] < x)
        {
            l = mid + 1;
        }
        else
        {
            r = mid;
        }
    }

    if (input->v[l] < x)
        return l + 1;
    return l;
}

// Função que será executada pelas threads
void *thread_bsearch_lower_bound(void *arg)
{
    data_t *d = (data_t *)arg;
    threaded_bsearch_lower_bound(d); // Chama a busca sequencial
    pthread_exit(NULL);
}

// Função que divide o trabalho em múltiplas threads
void threaded_bsearch_lower_bound(data_t *d)
{
    // Caso base: se a região do vetor x é inválida (início maior que o fim), termina a execução
    if (d->x.l > d->x.r)
        return;

    // Caso base: se a região do vetor x contém apenas um elemento
    if (d->x.l == d->x.r)
    {
        // Adiciona ao resultado o índice da busca binária para o único elemento de x
        add_result(d->result, bsearch_s(&d->input, d->x.v[d->x.l]));
        return;
    }

    // Caso base: se o vetor de entrada tem apenas um elemento
    if (d->input.l == d->input.r)
    {
        // Para cada elemento em x, adiciona a posição desse único elemento ao resultado
        for (int i = d->x.l; i <= d->x.r; i++)
            add_result(d->result, d->input.l);
        return;
    }

    // Criação de duas estruturas `data_t` para representar as metades esquerda e direita
    data_t ld = *d, rd = *d;                              // Cópia dos dados para subdividir
    int mid = d->input.l + (d->input.r - d->input.l) / 2; // Calcula o ponto médio do vetor de entrada

    // Busca o ponto médio no vetor x para dividir a carga de trabalho
    int mid_x = bsearch_s(&d->x, d->input.v[mid]) - 1;

    // Ajusta os limites das sub-regiões para a metade esquerda
    ld.input.r = mid; // Região esquerda do vetor de entrada
    ld.x.r = mid_x;   // Região correspondente no vetor x

    // Ajusta os limites das sub-regiões para a metade direita
    rd.input.l = mid + 1; // Região direita do vetor de entrada
    rd.x.l = mid_x + 1;   // Região correspondente no vetor x

    // Se não há threads disponíveis, realiza a busca binária sequencialmente nas duas metades
    if (d->nThreads < 2)
    {
        threaded_bsearch_lower_bound(&ld); // Processa a metade esquerda
        threaded_bsearch_lower_bound(&rd); // Processa a metade direita
        return;
    }

    // Verifica se a região da metade esquerda é válida
    if (ld.x.r - ld.x.l < 0)
    {
        // Se não for válida, processa apenas a metade direita
        threaded_bsearch_lower_bound(&rd);
        return;
    }

    // Verifica se a região da metade direita é válida
    if (rd.x.r - rd.x.l < 0)
    {
        // Se não for válida, processa apenas a metade esquerda
        threaded_bsearch_lower_bound(&ld);
        return;
    }

    // Reduz o número de threads disponíveis
    d->nThreads--;

    // Calcula a proporção do trabalho para distribuir as threads entre as duas metades
    double c = (double)(ld.x.r - ld.x.l) / (d->x.r - d->x.l);

    // Calcula o número de threads para a metade esquerda
    int t = d->nThreads * c + 1;

    // Exibe informações para depuração
    printf("t: %d\n l = %d, d = %d, c: %f\n", t, (ld.x.r - ld.x.l), (d->x.r - d->x.l), c);

    // Distribui as threads entre as duas metades
    ld.nThreads = t;
    rd.nThreads = d->nThreads - t;

    // Declaração das threads
    pthread_t threads[2];
    printf("2 threads criadas\n");

    // Cria threads para processar as metades esquerda e direita
    pthread_create(&threads[0], NULL, thread_bsearch_lower_bound, (void *)&ld);
    pthread_create(&threads[1], NULL, thread_bsearch_lower_bound, (void *)&rd);

    // Espera as threads terminarem
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    // Ordena o vetor de resultados ao final da execução
    int result_size = d->result->r - d->result->l + 1;
    qsort(d->result->v, result_size, sizeof(long long), cmp);
}

#include <stdbool.h>

#include <stdbool.h>

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
        if (idx_result < input->l || idx_result > input->r + 1)
        {
            printf("Erro: Índice fora do intervalo válido (%d).\n", idx_result);
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

int main()
{
    int n = 100000;
    int m = 60;
    vetor input, x, *result = (vetor *)malloc(sizeof(vetor));
    input.v = create_sorted_array(n, 13);
    x.v = create_sorted_array(m, 997);
    input.l = x.l = 0;
    input.r = n - 1;
    x.r = m - 1;
    data_t d;
    d.input = input;
    d.x = x;

    result->v = (long long *)malloc(3 * m * sizeof(long long));
    result->l = 0;
    result->r = -1;
    d.result = result;

    d.nThreads = MAX_THREADS;

    printf("Input:\n");
    print_array(&input);
    printf("x: %d\n", bsearch_s(&x, (long long)76189));
    print_array(&x);

    // Testes com diferentes valores de x

    threaded_bsearch_lower_bound(&d);
    printf("Result:\n");
    print_array(d.result);

    if (verify_result(&input, &x, result))
    {
        printf("Teste bem-sucedido: O vetor 'result' está correto.\n");
    }
    else
    {
        printf("Teste falhou: O vetor 'result' contém erros.\n");
    }

    free(input.v);

    return 0;
}
