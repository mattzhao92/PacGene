//
//  main.c
//  Pacwar
//
//  Created by Matt Zhao on 9/10/14.
//  Copyright (c) 2014 Matt Zhao. All rights reserved.
//

#include "PacWar.h"
#include "sort.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#define NUM_THREADS 8
#define POPULATION_SIZE 30


char trace_file_name[] = "trace";
char initial_population[] = "/Users/mattzhao/Desktop/Github/PacGene/linux/init_population.txt";

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


void rand_str(char *dest, size_t length) {
    char charset[] = "0123";
    while (length-- > 0) {
        int index = (int) (((double) rand() / RAND_MAX) * 4);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void copy_population(GeneWrapper ** destination_population, GeneWrapper * source_population, int population_size) {
    GeneWrapper * new_population = *destination_population;
    if (*destination_population == NULL) {
        new_population = (GeneWrapper *) malloc(sizeof(GeneWrapper) * population_size);
    }
    
    int i;
    char buffer[51];
    for (i = 0; i < population_size; i++) {
        new_population[i].gene = malloc(sizeof(PacGene));
        NewStringFromGene(source_population[i].gene, buffer);
        SetGeneFromString(buffer, new_population[i].gene);
    }
    *destination_population = new_population;
}

void free_population(GeneWrapper *population, int population_size) {
    int i;
    for (i = 0; i < population_size; i++) {
        free(population[i].gene);
    }
    free(population);
}



void compete(PacGenePtr p1, PacGenePtr p2, CompetionResult* result) {
    int count[2] = {1,1};
    int rounds = 500;
    FastDuel(p1, p2, &rounds, &count[0], &count[1]);
    result->count1 = count[0];
    result->count2 = count[1];
    result->rounds = rounds;
}



void result2score(int *score1, int *score2, const CompetionResult *result) {
    int score = 0;
    if (result->count1 > result->count2) {
        score = 10;
        if (result->count2 == 0) {
            if (result->rounds <= 100) {
                score = 20;
            } else if (result->rounds >= 100 && result->rounds <= 199) {
                score = 19;
            } else if (result->rounds >= 200 && result->rounds <= 299) {
                score = 18;
            } else if (result->rounds >= 300 && result->rounds <= 500) {
                score = 17;
            }
        } else {
            double ratio = 1.0 * result->count1 / result->count2;
            if (ratio >= 10.0) {
                score = 13;
            } else if (ratio >= 3.0 && ratio < 10.0) {
                score = 12;
            } else if (ratio >= 1.5 && ratio < 3.0) {
                score = 11;
            }
        }
    } else if (result->count1 < result->count2) {
        score = 0;
        if (result->count1 == 0) {
            if (result->rounds <= 100) {
                score = 0;
            } else if (result->rounds >= 100 && result->rounds <= 199) {
                score = 1;
            } else if (result->rounds >= 200 && result->rounds <= 299) {
                score = 2;
            } else if (result->rounds >= 300 && result->rounds <= 500) {
                score = 3;
            }
        } else {
            double ratio = 1.0 * result->count2 / result->count1;
            if (ratio >= 10.0) {
                score = 7;
            } else if (ratio >= 3.0 && ratio < 10.0) {
                score = 8;
            } else if (ratio >= 1.5 && ratio < 3.0) {
                score = 9;
            }
        }
    }
    if (NULL != score1) {
        *score1 = score;
    }
    if (NULL != score2) {
        *score2 = 20 - score;
    }
    
}



int compute_score(GeneWrapper * wrapper, GeneWrapper* population, int population_size) {
    CompetionResult result;
    int total_score = 0;
    int i;
    for (i = 0; i < population_size; i++) {
        PacGenePtr opponent = population[i].gene;
        compete(wrapper->gene, opponent, &result);
        int score;
        result2score(&score, NULL, &result);
        total_score += score;
    }
    return total_score;
}

int gene_score_comparator (const void * elem1, const void * elem2)
{
    GeneWrapper * f = (GeneWrapper*)elem1;
    GeneWrapper * s = (GeneWrapper*)elem2;
    if (f->score > s->score) return  1;
    if (f->score < s->score) return -1;
    return 0;
}

void mutual_compete(int set_size, GeneWrapper * gene_set) {
    int g1_idx, g2_idx;
    int score1, score2;
    CompetionResult result;
    for (g1_idx = 0; g1_idx < set_size; g1_idx++) {
        gene_set[g1_idx].score = 0;
    }
    for (g1_idx = 0; g1_idx < set_size; g1_idx++) {
        for (g2_idx = g1_idx + 1; g2_idx < set_size; g2_idx++){
            compete(gene_set[g1_idx].gene, gene_set[g2_idx].gene, &result);
            result2score(&score1, &score2, &result);
            gene_set[g1_idx].score += score1;
            gene_set[g2_idx].score += score2;
        }
    }
    MergeSort(gene_set,
              set_size,
              sizeof(gene_set[0]), gene_score_comparator);
}


void trace_population (GeneWrapper * wrappers, int population_size, int thread_id) {
    FILE *fp;
    
    time_t timer;
    char buffer[25];
    char string_gene[51];
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 25, "%m-%d %H:%M:%S", tm_info);
    int i;
    
    char trace_file_name_by_thread[256];
    bzero(trace_file_name_by_thread, 256);
    if (thread_id == -1) {
        snprintf(trace_file_name_by_thread, sizeof(trace_file_name_by_thread), "%s", trace_file_name);
    } else {
        snprintf(trace_file_name_by_thread, sizeof(trace_file_name_by_thread), "%s-%d", trace_file_name, thread_id);
    }
    
    fp = fopen(trace_file_name_by_thread, "a");
    for (i = 0; i < population_size; i++) {
        string_gene[50] = '\0';
        NewStringFromGene(wrappers[i].gene, string_gene);
        printf("[Thead:%d] %s : ", thread_id, buffer);
        printf(" #%d : [%s]\n", i, string_gene);
        fprintf(fp, "[Thead:%d] %s : ", thread_id, buffer);
        fprintf(fp, " #%d : [%s]\n", i, string_gene);
    }
    fclose(fp);
}

void trace_string(int thread_id, char *str_trace){
    FILE *fp;
    
    time_t timer;
    char buffer[25];
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 25, "%m-%d %H:%M:%S", tm_info);
    
    printf("[Thead:%d] %s : ", thread_id, buffer);
    printf(" %s\n", str_trace);
    
    char trace_file_name_by_thread[256];
    bzero(trace_file_name_by_thread, 256);
    if (thread_id == -1) {
        snprintf(trace_file_name_by_thread, sizeof(trace_file_name_by_thread), "%s", trace_file_name);
    } else {
        snprintf(trace_file_name_by_thread, sizeof(trace_file_name_by_thread), "%s-%d", trace_file_name, thread_id);
    }
    //puts(buffer);
    printf("find a strong species for new population. %s \n", str_trace);
    
    fp = fopen(trace_file_name_by_thread, "a");
    fprintf(fp, "[Thead:%d] %s : ", thread_id, buffer);
    fprintf(fp, " %s\n", str_trace);
    fclose(fp);
}

void clear_trace_file(void){
    FILE *fp;
    fp = fopen(trace_file_name, "w");
    fclose(fp);
}

GeneWrapper * initialize_population(int * population_size) {
    FILE *file;
    if ((file = fopen(initial_population, "r")) == NULL) {
        if (errno == ENOENT) {
            *population_size = POPULATION_SIZE;
            printf("initial population cannot be found, randomly initialize one \n");
            GeneWrapper * initial_population = (GeneWrapper *) malloc(sizeof(GeneWrapper) * (*population_size));
            int i;
            char random_gene_string[51];
            for (i = 0; i < * population_size; i++) {
                rand_str(random_gene_string, 50);
                initial_population[i].gene = malloc(sizeof(PacGene));
                SetGeneFromString(random_gene_string, initial_population[i].gene);
            }
            return initial_population;
        } else {
            printf("some other error occured");
            exit(2);
        }
    } else {
        *population_size  = 0;
        char line [1000];
        while (fgets ( line, sizeof line, file ) != NULL )
        {
            *population_size = *population_size + 1;
        }
        
        printf("initial population is read from file, size is %d \n", *population_size);
        
        rewind(file);
        GeneWrapper * initial_population = (GeneWrapper *) malloc(sizeof(GeneWrapper) * (*population_size));
        
        int i = 0;
        char gene_string[51];
        
        while (fgets ( line, sizeof line, file ) != NULL )
        {
            strncpy(gene_string, line, 50);
            gene_string[50] = '\0';
            initial_population[i].gene = malloc(sizeof(PacGene));
            SetGeneFromString(gene_string, initial_population[i].gene);
            i ++;
        }
        
        fclose(file);
        return initial_population;
    }
}

void reduce_population_through_competition(GeneWrapper * initial_population, GeneWrapper * new_population,
                                           int initial_population_size, int new_population_size) {
    
    char buffer[51];
    int i = initial_population_size - 1;
    int j = new_population_size - 1;
    
    GeneWrapper * new_population_copy = NULL;
    copy_population(&new_population_copy, new_population, new_population_size);
    
    mutual_compete(new_population_size, new_population_copy);
    
    while (i >= 0) {
        NewStringFromGene(new_population_copy[j].gene, buffer);
        SetGeneFromString(buffer, initial_population[i].gene);
        i--;
        j--;
    }
    
    free_population(new_population_copy, new_population_size);
}



void reduce_population_through_evolution_helper(bool * flags, int index, int flag_array_size,
                                                GeneWrapper * unitialize_genes, GeneWrapper* p1, GeneWrapper* p2,
                                                int * first_uninitialized_gene) {
    
    int i;
    if (index == flag_array_size) {
        char buffer[51];
        char p1buffer[51];
        char p2buffer[52];
        int offsets[6]  = {0, 4, 20, 23, 26, 38};
        int sizes[6]  = {4, 16, 3, 3, 12, 12};
        
        GeneWrapper * unitialized_gene = &unitialize_genes[*first_uninitialized_gene];
        
        NewStringFromGene(p2->gene, buffer);
        NewStringFromGene(p1->gene, p1buffer);
        NewStringFromGene(p2->gene, p2buffer);
        for (i = 0; i < flag_array_size; i++) {
            if (flags[i]) {
                memcpy(buffer+offsets[i], p1buffer+offsets[i], sizes[i]);
            }
        }
        unitialized_gene->gene = malloc(sizeof(PacGene));
        unitialized_gene->score = 0;
        SetGeneFromString(buffer, unitialized_gene->gene);
        *first_uninitialized_gene = * first_uninitialized_gene + 1;
        return;
    }
    
    bool options[2] = {true, false};
    for (i = 0; i < 2; i++) {
        flags[index] = options[i];
        reduce_population_through_evolution_helper(flags, index+1, flag_array_size, unitialize_genes, p1, p2, first_uninitialized_gene);
    }
}


void reduce_population_through_evolution(GeneWrapper * initial_population, GeneWrapper * new_population,
                                         int population_size,
                                         double elite_rate,
                                         double mutation_rate) {
    
    assert(population_size * elite_rate * 64 >= population_size);
    int size_to_consider = (int) (population_size * elite_rate);
    if (size_to_consider <= 0) {
        size_to_consider = population_size;
    }
    GeneWrapper combined_populaton[size_to_consider * 64];
    int i,j;
    bool flags[6];
    int first_unitialized_gene = 0;
    for (i = population_size - size_to_consider; i < population_size; i++) {
        for (j = i+1; i < population_size; i++) {
            reduce_population_through_evolution_helper(flags, 0, 6, combined_populaton, &new_population[i], &new_population[j], &first_unitialized_gene);
        }
    }
    
    assert(first_unitialized_gene == size_to_consider * 64);
    reduce_population_through_competition(initial_population, combined_populaton, population_size, size_to_consider * 64);
    for (i = 0; i < size_to_consider * 64; i++) {
        free(combined_populaton[i].gene);
    }
}

void * generate_new_generation(void *arg)
{
    struct thread_arguments_info *info = arg;
    int thread_id = info->thread_id;
    int population_size = info->population_size;
    GeneWrapper * population = info->population;
    GeneWrapper * next_population = info->next_population;
    char random_gene_string[51];
    int i;
    
    for (i = 0; i < population_size; i++) {
        rand_str(random_gene_string, 50);
        random_gene_string[50] = '\0';
        SetGeneFromString(random_gene_string, next_population[i].gene);
        
        GeneWrapper current_gene = next_population[i];
        int current_score = compute_score(&current_gene, population, population_size);
        bool found_local_minimum = false;
        while (!found_local_minimum) {
            bool better_neighbor_found = false;
            int start_index = (int) (((double) rand() / RAND_MAX) * 50);
            int j = start_index;
            
            do {
                j++;
                if (j >= 50) {
                    j = 0;
                }
                char old_char = random_gene_string[j];
                int start_value_index = (int) (((double) rand() / RAND_MAX) * 4);
                int z = start_value_index;
                
                do {
                    z ++;
                    if (z >= 4) {
                        z = 0;
                    }
                    char value = '0' + z;
                    if (value != old_char) {
                        random_gene_string[j] = value;
                        SetGeneFromString(random_gene_string, current_gene.gene);
                        int neighbor_score = compute_score(&current_gene, population, population_size);
                        if (neighbor_score > current_score) {
                            printf("[Thread:%d] better neighbor found score : %d \n", thread_id, neighbor_score);
                            better_neighbor_found = true;
                            current_score = neighbor_score;
                            break;
                        } else {
                            random_gene_string[j] = old_char;
                        }
                    }
                } while (start_value_index != z && !better_neighbor_found);
                
            } while (j != start_index && !better_neighbor_found);
            
            if (!better_neighbor_found) {
                SetGeneFromString(random_gene_string, current_gene.gene);
                printf("[Thread:%d] found a local minimum \n", thread_id);
                found_local_minimum = true;
            }
        }
        
        
        // generate the next population by getting the 1000 strongest genes
        // from the set (population + next_population)
        GeneWrapper combined_population[population_size * 2];
        for (i = 0; i < population_size; i++) {
            combined_population[i].gene = population[i].gene;
            combined_population[i].score = 0;
            combined_population[population_size + i].gene = next_population[i].gene;
            combined_population[population_size + i].score = 0;
        }
        
        reduce_population_through_competition(next_population, combined_population, population_size, population_size * 2);
        trace_population(next_population, population_size, thread_id);
        
    }
    return NULL;
}



int main(int argc, const char * argv[]) {
    
    int i, j, population_size;
    // clear_trace_file();
    
    // initialize initial population
    GeneWrapper * population = initialize_population(&population_size);
    int number_threads = NUM_THREADS;
    pthread_t pthread_ids[number_threads];
    struct thread_arguments_info * args[number_threads];
    for (i = 0; i < number_threads; i++) {
        struct thread_arguments_info * info = malloc(sizeof(struct thread_arguments_info));
        info->population = NULL;
        args[i] = info;
    }
    
    while (1) {
        GeneWrapper next_population[number_threads * population_size];
        
        for (i = 0; i < number_threads; i++) {
            int start = i * population_size;
            int end = start + population_size;
            for (j = start; j < end; j++) {
                next_population[j].gene = malloc(sizeof(PacGene));
            }
            
            struct thread_arguments_info * info = args[i];
            copy_population(&info->population, population, population_size);
            info->next_population = next_population + start;
            info->population_size = population_size;
            info->thread_id = i;
            
            if (pthread_create(&pthread_ids[i], NULL, generate_new_generation, info)) {
                fprintf(stderr, "thread initialization failed \n");
            }
        }
        
        for (i = 0; i < number_threads; i++) {
            pthread_join(pthread_ids[i], NULL);
        }
        
        // store the elite guys into elite_population
        reduce_population_through_competition(population, next_population, population_size, population_size * number_threads);
        //reduce_population_through_evolution(population, population, population_size, 0.05, 0.02);
        
        // clean up malloced stuff
        for (i = 0; i < number_threads * population_size; i++) {
            free(next_population[i].gene);
        }
        
        // print out the current super elite population
        trace_population(population, population_size, -1);
        printf("a new generation population from threads have been merged! \n");
    }
    
    //free up memory
    for (i = 0; i < population_size; i++) {
        free(population[i].gene);
    }
    
    for (i = 0; i < number_threads; i++) {
        for (j = 0; j < population_size; j++) {
            free(args[i]->population[j].gene);
        }
        free(args[i]->population);
        free(args[i]);
    }
    free(population);
}
