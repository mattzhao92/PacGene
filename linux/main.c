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
#include <stdlib.h>
#include <time.h>


#define POPULATION_SIZE 50
char trace_file_name[] = "trace.txt";
char *str_population;
char *str_next_population;

typedef struct competion_result {
    int count1;
    int count2;
    int rounds;
} CompetionResult;


typedef struct gene_score {
    int score;
    int idx;
    PacGenePtr gene;
} GeneScore;


void rand_str(char *dest, size_t length) {
    char charset[] = "0123";
    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void neighbor_by_idx (char *neighbor, char *current, int idx) {
    strncpy(neighbor, current, 51);
    int position = idx / 4;
    int offset = idx % 4;
    neighbor[position] = (neighbor[position] - '0' + offset) % 4 + '0';
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

int compute_score(PacGenePtr gene, PacGenePtr* population) {
    CompetionResult result;
    int total_score = 0;
    int i;
    for (i = 0; i < POPULATION_SIZE; i++) {
        PacGenePtr opponent = population[i];
        compete(gene, opponent, &result);
        int score;
        result2score(&score, NULL, &result);
        total_score += score;
    }
    return total_score;
}

int comp (const void * elem1, const void * elem2)
{
    GeneScore * f = (GeneScore*)elem1;
    GeneScore * s = (GeneScore*)elem2;
    if (f->score > s->score) return  1;
    if (f->score < s->score) return -1;
    return 0;
}

void mutual_compete(int set_size, GeneScore * gene_set) {
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
	      sizeof(gene_set[0]), comp);
}



void trace_population (void) {
    FILE *fp;
    
    time_t timer;
    char buffer[25];
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 25, "%m-%d %H:%M:%S", tm_info);
    int i;
    for (i = 0; i < POPULATION_SIZE; i++) {
        printf("%s : ", buffer);
        printf(" #%d : [%s]\n", i, str_population + 51 * i);
    }
    //puts(buffer);
    
    fp = fopen(trace_file_name, "a");
    for (i = 0; i < POPULATION_SIZE; i++) {
        fprintf(fp, "%s : ", buffer);
        fprintf(fp, " #%d : [%s]\n", i, str_population + 51 * i);
    }
    fclose(fp);
}

void trace_string(char *str_trace){
    FILE *fp;
    
    time_t timer;
    char buffer[25];
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 25, "%m-%d %H:%M:%S", tm_info);
    
        printf("%s : ", buffer);
        printf("%s\n", str_trace);
    //puts(buffer);
    
    fp = fopen(trace_file_name, "a");
    fprintf(fp, "%s : ", buffer);
    fprintf(fp, "%s\n", str_trace);
    fclose(fp);
}

void clear_trace_file(void){
    FILE *fp;
    fp = fopen(trace_file_name, "w");
    fclose(fp);
}






int main(int argc, const char * argv[]) {
    
    int i;
    
    clear_trace_file();
    // generate 1000 random genes
    PacGenePtr population[POPULATION_SIZE];
    PacGenePtr next_population[POPULATION_SIZE];
    char random_gene_string[51];
    //char best_gene_string[51];
    
    str_population = (char *)malloc(POPULATION_SIZE * 51 * sizeof(char));
    str_next_population = (char *)malloc(POPULATION_SIZE * 51 * sizeof(char));
    
    for (i = 0; i < POPULATION_SIZE; i++) {
        rand_str(random_gene_string, 50);
        population[i] = malloc(sizeof(PacGene));
        next_population[i] = malloc(sizeof(PacGene));
        SetGeneFromString(random_gene_string, population[i]);
        strncpy((str_population + i * 51), random_gene_string, 51);
    }
    
    
    while (1) {
        
        for (i = 0; i < POPULATION_SIZE; i++) {
            rand_str(random_gene_string, 50);
            SetGeneFromString(random_gene_string, next_population[i]);
            PacGenePtr current_gene = next_population[i];
            int current_score = compute_score(current_gene, population);
            // finding local minimum
            while (1) {
                //impacient mount climbing
                // find a random neighbor with better score than current
                int neighbor_score = 0;
                int neighbor_visted[200] = {0};
                neighbor_visted[0] = 1;
                int neighbor_visted_count = 1;
                char neighbor_gene_str[51];
                int local_max_flag = 0;
                while (1) {
                    // keep generating new neighbors
                    while (neighbor_visted_count < 200) {
                        //generate a random unvisited neighbor
                        int neighbor_idx = 1 + (double) rand() / RAND_MAX * 199;
                        if (neighbor_visted[neighbor_idx] == 0) {
                            neighbor_visted[neighbor_idx] = 1;
                            neighbor_visted_count++;
                            neighbor_by_idx(neighbor_gene_str, random_gene_string, neighbor_idx);
                            break;
                            // find a new neighbor
                        }
                    }
                    
                    PacGenePtr neigbhor_gene = next_population[i];
                    SetGeneFromString(neighbor_gene_str, neigbhor_gene);
                    neighbor_score = compute_score(neigbhor_gene, population);
                    
                    if (neighbor_score > current_score) {
                        printf("find a better neighbor %d %d \n", current_score, neighbor_score);
                        strncpy(random_gene_string, neighbor_gene_str, 51);
                        SetGeneFromString(random_gene_string, current_gene);
                        current_score = neighbor_score;
                        break;
                    } else if (neighbor_visted_count >= 200) {
                        printf("local maximum\n");
                        local_max_flag = 1;
                        break;
                    }
                }
                if (local_max_flag == 1) {
                    break;
                }
            }
            
            SetGeneFromString(random_gene_string, next_population[i]);
            strncpy((str_next_population + i * 51), random_gene_string, 51);
            trace_string((str_next_population + i * 51));
            printf("find a strong species for new population. %s \n", (str_next_population + i * 51));
        }
        // generate the next population by getting the 1000 strongest genes
        // from the set (population + next_population)
        GeneScore * combined_population = (GeneScore *) malloc(sizeof(GeneScore) * (POPULATION_SIZE * 2));
        for (i = 0; i < POPULATION_SIZE; i++) {
            combined_population[i].gene = population[i];
            combined_population[i].idx = i;
	    combined_population[i].score = 0;
            combined_population[POPULATION_SIZE + i].gene = next_population[i];
            combined_population[POPULATION_SIZE + i].idx = POPULATION_SIZE + i;
	    combined_population[POPULATION_SIZE + i].score = 0;
        }
        mutual_compete(POPULATION_SIZE * 2, combined_population);
        
        char *tmp_str_strongest_gene = (char *) malloc( 51 * POPULATION_SIZE * sizeof(char));
        
        for (i = POPULATION_SIZE * 2 - 1; i >= 0; i--) {
            GeneScore * gene_score = &(combined_population[i]);
            if (i >= POPULATION_SIZE) {
                population[i-POPULATION_SIZE] = gene_score->gene;
                if (gene_score->idx < POPULATION_SIZE) {
                    // gene originally in population
                    strncpy(tmp_str_strongest_gene + 51 * (i-POPULATION_SIZE),
                            str_population + 51 * gene_score->idx,
                            51);
                } else {
                    // gene originally in next_population
                    strncpy(tmp_str_strongest_gene + 51 * (i-POPULATION_SIZE),
                            str_next_population + 51 * (gene_score->idx - POPULATION_SIZE),
                            51);
                }
            } else {
                //next_population[i-2*POPULATION_SIZE] = gene_score->gene;
                next_population[i] = gene_score->gene;
            }
        }
        
        memcpy(str_population, tmp_str_strongest_gene, 51 * POPULATION_SIZE);
        
        trace_population();
        
        /*
        for (int i = 0; i < POPULATION_SIZE; i++) {
            GeneScore * gene_score = &(combined_population[i]);
            gene_score->gene = population[i];
            int score = compute_score(population[i], next_population);
            gene_score->score = score;
        }
        
        
        for (int i = 0; i < POPULATION_SIZE; i++) {
            GeneScore * gene_score = &(combined_population[POPULATION_SIZE+i]);
            gene_score->gene  = next_population[i];
            int score = compute_score(next_population[i], population);
            gene_score->score = score;
        }
        
        mergesort(combined_population,
                  sizeof(combined_population)/sizeof(combined_population[0]),
                  sizeof(combined_population[0]), comp);
        
        for (int i = POPULATION_SIZE * 2 - 1; i >= 0; i--) {
            GeneScore * gene_score = &(combined_population[i]);
            if (i >= POPULATION_SIZE) {
                
                population[i-POPULATION_SIZE] = gene_score->gene;
            } else {
                next_population[i-2*POPULATION_SIZE] = gene_score->gene;
            }
        }
        */
    }
    
    //free up memory
    for (i = 0; i < POPULATION_SIZE; i++) {
        free(population[i]);
        free(next_population[i]);
    }
    free(str_population);
    free(str_next_population);
}





