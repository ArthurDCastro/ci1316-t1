// chrono.h
//
// A small library to measure time in programs
//
// by W.Zola (2017)

#ifndef CHRONO_H
#define CHRONO_H

#include <time.h>
#include <stdio.h>

// Estrutura para armazenar os dados do cronômetro
typedef struct {
    struct timespec xadd_time1, xadd_time2;
    long long xtotal_ns;
    long xn_events;
} chronometer_t;

// Funções do cronômetro

// Reseta o cronômetro (zera tempo total e eventos)
void chrono_reset(chronometer_t *chrono);

// Inicia a contagem do tempo
inline void chrono_start(chronometer_t *chrono);

// Retorna o tempo total acumulado em nanosegundos
inline long long chrono_gettotal(chronometer_t *chrono);

// Retorna o número total de eventos registrados
inline long long chrono_getcount(chronometer_t *chrono);

// Para a contagem de tempo e atualiza o tempo total acumulado
inline void chrono_stop(chronometer_t *chrono);

// Relatório de tempo médio por operação
void chrono_reportTime(chronometer_t *chrono, char *s);

// Relatório de tempo médio considerando um loop de operações
void chrono_report_TimeInLoop(chronometer_t *chrono, char *s, int loop_count);

#endif // CHRONO_H
