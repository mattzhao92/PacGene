#ifndef PACWAR_UTILS_H
#define PACWAR_UTILS_H
#include "PacWar.h"
#include "sort.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#define NUM_THREADS 4
#define POPULATION_SIZE 20
#define NUM_ITERATIONS 200


typedef struct competion_result {
    int count1;
    int count2;
    int rounds;
} CompetionResult;


typedef struct gene_score {
    int score;
    PacGenePtr gene;
} GeneWrapper;


struct thread_arguments_info {
    int population_size;
    int thread_id;
    int number_iterations;
    GeneWrapper * population;
    GeneWrapper * next_population;
};

void copy_population(GeneWrapper ** destination_population, GeneWrapper * source_population, int population_size);

void free_population(GeneWrapper *population, int population_size);


void compete(PacGenePtr p1, PacGenePtr p2, CompetionResult* result);
void compete_againt_population(PacGenePtr p, GeneWrapper * population, int population_size, CompetionResult* result);

void result2score(int *score1, int *score2, const CompetionResult *result);


long compute_score(GeneWrapper * wrapper, GeneWrapper* population, int population_size);
int gene_score_comparator (const void * elem1, const void * elem2);

void mutual_compete(int set_size, GeneWrapper * gene_set);

void trace_population (GeneWrapper * wrappers, int population_size, int thread_id);
void trace_string(int thread_id, char *str_trace);
void clear_trace_file(void);
int initialize_population(GeneWrapper ** population, char * population_trace, bool random_init, int * population_size);

void reduce_population_through_competition(GeneWrapper * initial_population, GeneWrapper * new_population,
                                           int initial_population_size, int new_population_size);



void reduce_population_through_evolution(GeneWrapper * initial_population, GeneWrapper * new_population,
                                         int population_size,
                                         double elite_rate,
                                         double mutation_rate);

void generate_new_generation(void *arg);
#endif
