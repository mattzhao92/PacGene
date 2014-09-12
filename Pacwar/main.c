//
//  main.c
//  Pacwar
//
//  Created by Matt Zhao on 9/10/14.
//  Copyright (c) 2014 Matt Zhao. All rights reserved.
//

#include "PacWar.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define POPULATION_SIZE 10

typedef struct competion_result {
    int count1;
    int count2;
    int rounds;
} CompetionResult;


typedef struct gene_score {
    int score;
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

void compete(PacGenePtr p1, PacGenePtr p2, CompetionResult* result) {
    int count[2] = {1,1};
    int rounds = 500;
    FastDuel(p1, p2, &rounds, &count[0], &count[1]);
    result->count1 = count[0];
    result->count2 = count[1];
    result->rounds = rounds;
}



int compute_score(PacGenePtr gene, PacGenePtr* population) {
    CompetionResult result;
    int total_score = 0;
    for (int i = 0; i < POPULATION_SIZE; i++) {
        PacGenePtr opponent = population[i];
        compete(gene, opponent, &result);
        
        int score = 0;
        if (result.count1 > result.count2) {
            score = 10;
            if (result.count2 == 0) {
                if (result.rounds <= 100) {
                    score = 20;
                } else if (result.rounds >= 100 && result.rounds <= 199) {
                    score = 19;
                } else if (result.rounds >= 200 && result.rounds <= 299) {
                    score = 18;
                } else if (result.rounds >= 300 && result.rounds <= 500) {
                    score = 17;
                }
            } else {
                double ratio = 1.0 * result.count1 / result.count2;
                if (ratio >= 10.0) {
                    score = 13;
                } else if (ratio >= 3.0 && ratio < 10.0) {
                    score = 12;
                } else if (ratio >= 1.5 && ratio < 3.0) {
                    score = 11;
                }
            }
        } else if (result.count1 < result.count2) {
            
            score = 0;
            if (result.count1 == 0) {
                if (result.rounds <= 100) {
                    score = 0;
                } else if (result.rounds >= 100 && result.rounds <= 199) {
                    score = 1;
                } else if (result.rounds >= 200 && result.rounds <= 299) {
                    score = 2;
                } else if (result.rounds >= 300 && result.rounds <= 500) {
                    score = 3;
                }
            } else {
                double ratio = 1.0 * result.count2 / result.count1;
                if (ratio >= 10.0) {
                    score = 7;
                } else if (ratio >= 3.0 && ratio < 10.0) {
                    score = 8;
                } else if (ratio >= 1.5 && ratio < 3.0) {
                    score = 9;
                }
            }
        }
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



int main(int argc, const char * argv[]) {
    
    // generate 1000 random genes
    PacGenePtr population[POPULATION_SIZE];
    PacGenePtr next_population[POPULATION_SIZE];
    char random_gene_string[51];
    char best_gene_string[51];
    
    
    for (int i = 0; i < POPULATION_SIZE; i++) {
        rand_str(random_gene_string, 50);
        population[i] = malloc(sizeof(PacGene));
        next_population[i] = malloc(sizeof(PacGene));
        SetGeneFromString(random_gene_string, population[i]);
        
    }
    
    while (1) {
        
        for (int i = 0; i < POPULATION_SIZE; i++) {
            rand_str(random_gene_string, 50);
            SetGeneFromString(random_gene_string, next_population[i]);
            PacGenePtr current_gene = next_population[i];
            
            // finding local minimum
            while (1) {
                int current_score = compute_score(current_gene, population);
                int max_score = 0;
                
                strncpy(best_gene_string, random_gene_string, 51);
                // for each neighbor
                for (int j = 0; j < 50; j++) {
                    char old_char = random_gene_string[j];
                    PacGenePtr neigbhor_gene = next_population[i];
                    
                    for (char value = '0'; value < '4'; value++) {
                        if (value != old_char) {
                            random_gene_string[j] = value;
                            SetGeneFromString(random_gene_string, neigbhor_gene);
                            int neighbor_score = compute_score(neigbhor_gene, population);
                            if (neighbor_score > max_score) {
                                max_score = neighbor_score;
                                strncpy(best_gene_string, random_gene_string, 51);
                            }
                        }
                    }
                    
                    random_gene_string[i] = old_char;
                }
                
                if (current_score < max_score) {
                    strncpy(random_gene_string, best_gene_string, 51);
                    SetGeneFromString(random_gene_string, current_gene);
                } else {
                    printf("%d %d \n", current_score, max_score);
                    break;
                }
            }
            
            SetGeneFromString(random_gene_string, next_population[i]);
        }
        // generate the next population by getting the 1000 strongest genes
        // from the set (population + next_population)
        GeneScore * combined_population = (GeneScore *) malloc(sizeof(GeneScore *) * (POPULATION_SIZE * 2));
        
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
    }
}





