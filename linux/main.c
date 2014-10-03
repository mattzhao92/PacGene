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
#include <pthread.h>


char trace_file_name[] = "trace.txt";
char initial_population[] = "/Users/mattzhao/Desktop/PacGene/linux/init_population.txt";

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
    snprintf(trace_file_name_by_thread, sizeof(trace_file_name_by_thread), "%s-%d", trace_file_name, thread_id);
    
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
    snprintf(trace_file_name_by_thread, sizeof(trace_file_name_by_thread), "%s-%d", trace_file_name, thread_id);
    
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
            *population_size = 50;
            printf("Initial Population cannot be found, randomly initialize one");
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
            printf("Some other error occured");
            exit(2);
        }
    } else {
        *population_size  = 0;
        char line [1000];
        while (fgets ( line, sizeof line, file ) != NULL )
        {
            *population_size = *population_size + 1;
        }
        
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

void update_initial_population(GeneWrapper * initial_population, GeneWrapper * new_population,
                               int initial_population_size, int new_population_size) {
    
    char buffer[51];
    int i = initial_population_size - 1;
    int j = new_population_size - 1;
    
    mutual_compete(new_population_size, new_population);
    
    while (i >= 0) {
        NewStringFromGene(new_population[j].gene, buffer);
        SetGeneFromString(buffer, initial_population[i].gene);
        i--;
        j--;
    }
}

void * generate_new_generation(void *arg)
{
    struct thread_arguments_info *info = arg;
    int thread_id = info->thread_id;
    int population_size = info->population_size;
    int number_iterations = info->number_iterations;
    GeneWrapper * population = info->population;
    GeneWrapper * next_population = info->next_population;
    char random_gene_string[51];
    char neighbor_gene_string[51];
    int i;
    
    while (number_iterations -- > 0) {
        for (i = 0; i < population_size; i++) {
            
            rand_str(random_gene_string, 50);
            random_gene_string[50] = '\0';
            neighbor_gene_string[50] = '\0';
            SetGeneFromString(random_gene_string, next_population[i].gene);
            SetGeneFromString(neighbor_gene_string, next_population[i].gene);
            
            GeneWrapper current_gene = next_population[i];
            int current_score = compute_score(&current_gene, population, population_size);
            
            // start impatient climbing
            while (1) {
                bool found_local_minimum = false;
                bool better_neighbor_found = false;
                
                while (!found_local_minimum) {
                    int start_index = (int) (((double) rand() / RAND_MAX) * 50);
                    int j = start_index-1;
                    //  for each neighbor
                    do {
                        j++;
                        if (j >= 50) {
                            j = 0;
                        }
                        char old_char = random_gene_string[j];
                        int start_value_index = (int) (((double) rand() / RAND_MAX) * 4);
                        int z = start_value_index-1;
                        
                        do {
                            z ++;
                            if (z >= 4) {
                                z = 0;
                            }
                            char value = '0' + z;
                            if (value != old_char) {
                                neighbor_gene_string[j] = value;
                                SetGeneFromString(neighbor_gene_string, current_gene.gene);
                                int neighbor_score = compute_score(&current_gene, population, population_size);
                                if (neighbor_score > current_score) {
                                    better_neighbor_found = true;
                                    break;
                                }
                            }
                        } while (start_value_index != z);
                        
                        neighbor_gene_string[j] = old_char;
                    } while (j != start_index);
                    
                    if (!better_neighbor_found) {
                        SetGeneFromString(random_gene_string, current_gene.gene);
                        found_local_minimum = true;
                    }
                }
                
                if (found_local_minimum) {
                    break;
                }
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
        
        update_initial_population(population, combined_population, population_size, population_size * 2);
    }
    trace_population(population, population_size, thread_id);
	return NULL;
}


GeneWrapper * copy_population(GeneWrapper * population, int population_size) {
    GeneWrapper * new_population = (GeneWrapper *) malloc(sizeof(GeneWrapper) * population_size);
    int i;
    char buffer[51];
    for (i = 0; i < population_size; i++) {
        new_population[i].gene = malloc(sizeof(PacGene));
        NewStringFromGene(population[i].gene, buffer);
        SetGeneFromString(buffer, new_population[i].gene);
    }
    return new_population;
}

int main(int argc, const char * argv[]) {
    
    int i, j, population_size;
    // clear_trace_file();
    
    // initialize initial population
    GeneWrapper * population = initialize_population(&population_size);
    int number_threads = 4;
    pthread_t pthread_ids[number_threads];
    struct thread_arguments_info * args[number_threads];
    while (1) {
        GeneWrapper next_population[number_threads * population_size];
        
        for (i = 0; i < number_threads; i++) {
            int start = i * population_size;
            int end = start + population_size;
            for (j = start; j < end; j++) {
                next_population[j].gene = malloc(sizeof(PacGene));
            }
            
            struct thread_arguments_info * info = malloc(sizeof(struct thread_arguments_info));
            args[i] = info;
            info->population = copy_population(population, population_size);
            info->next_population = next_population + start;
            info->population_size = population_size;
            info->thread_id = i;
            info->number_iterations = 2;
            
            if (pthread_create(&pthread_ids[i], NULL, generate_new_generation, info)) {
                fprintf(stderr, "thread initialization failed \n");
            }
            
        }
        
        for (i = 0; i < number_threads; i++) {
            pthread_join(pthread_ids[i], NULL);
        }
        
        update_initial_population(population, next_population, population_size, population_size * number_threads);
        
        for (i = 0; i < number_threads; i++) {
            for (j = 0; j < population_size; j++) {
                free(args[i]->population[j].gene);
            }
            free(args[i]->population);
            free(args[i]);
        }
        
        for (i = 0; i < number_threads * population_size; i++) {
            free(next_population[i].gene);
        }
        
        printf("Merge population from threads finished! \n");
        
    }
    
    //free up memory
    for (i = 0; i < population_size; i++) {
        free(population[i].gene);
    }
    free(population);
}