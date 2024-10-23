#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>

#define MAX_THREADS 8

typedef struct {
    long long *input;
    long long x;
    int l;
    int r;
    int result;
    int nThreads;
} data_t;

// Função de busca binária sequencial
void bsearch_lower_bound_sequential(data_t *d) {
    while (d->l < d->r) {
        int mid = d->l + (d->r - d->l) / 2;

        if (d->input[mid] < d->x) {
            d->l = mid + 1;
        } else {
            d->r = mid;
        }
    }
    d->result = d->l;
}

// Função que será executada pelas threads
void* thread_bsearch_lower_bound(void* arg) {
    data_t *d = (data_t*)arg;
    bsearch_lower_bound_sequential(d);  // Chama a busca sequencial
    pthread_exit(NULL);
}

// Função que divide o trabalho em múltiplas threads
void threaded_bsearch_lower_bound(data_t *d) {
    if (d->nThreads <= 1) {
        bsearch_lower_bound_sequential(d);  // Se não há threads suficientes, faz sequencialmente
        return;
    }

    // Criação de duas estruturas para as sub-regiões da busca
    data_t ld = *d, rd = *d;
    int mid = d->l + (d->r - d->l) / 2;

    ld.r = mid;        // Metade esquerda
    rd.l = mid + 1;    // Metade direita

    int t = d->nThreads / 2;

    ld.nThreads = t;
    rd.nThreads = d->nThreads - t;

    pthread_t threads[2];

    // Cria threads para ambas as metades
    pthread_create(&threads[0], NULL, thread_bsearch_lower_bound, (void*)&ld);
    pthread_create(&threads[1], NULL, thread_bsearch_lower_bound, (void*)&rd);

    // Aguarda as threads finalizarem
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    // Combina o resultado
    d->result = (d->input[ld.result] >= d->x) ? ld.result : rd.result;
}

// Função para criar o vetor ordenado
long long* create_sorted_array(int n) {
    long long* array = (long long*)malloc(n * sizeof(long long));

    if (array == NULL) {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        array[i] = (long long)i * 13;
    }

    return array;
}

void print_array(long long* array, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d: %lld ", i, array[i]);
    }
    printf("\n");
}


int main() {
    int n = 100;
    data_t d;
    d.input = create_sorted_array(n);
    d.l = 0;
    d.r = n;
    d.nThreads = MAX_THREADS;

    print_array(d.input, n);

    // Testes com diferentes valores de x
    d.x = 0;
    threaded_bsearch_lower_bound(&d);
    printf("Posição de %lld: %d\n", d.x, d.result);

    d.x = 1;
    threaded_bsearch_lower_bound(&d);
    printf("Posição de %lld: %d\n", d.x, d.result);

    d.x = 200;
    threaded_bsearch_lower_bound(&d);
    printf("Posição de %lld: %d\n", d.x, d.result);

    d.x = 300;
    threaded_bsearch_lower_bound(&d);
    printf("Posição de %lld: %d\n", d.x, d.result);

    d.x = 800;
    threaded_bsearch_lower_bound(&d);
    printf("Posição de %lld: %d\n", d.x, d.result);

    d.x = 900;
    threaded_bsearch_lower_bound(&d);
    printf("Posição de %lld: %d\n", d.x, d.result);

    free(d.input);

    return 0;
}
