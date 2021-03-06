#include "Utils.h"
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char * trace_file_name;
extern char * initial_population;

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv)
{
    bool population_mode = false;
    bool individual_mode = false;
    bool rank_mode = false;
    char * tokens = NULL;
    int c;
    
    opterr = 0;
    while ((c = getopt (argc, argv, "p:i:r:")) != -1)
        switch (c)
    {
        case 'p':
            population_mode = true;
            tokens = optarg;
            break;
        case 'i':
            individual_mode = true;
            tokens = optarg;
            break;
        case 'r':
            rank_mode = true;
            tokens = optarg;
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        default:
            abort ();
    }
    
    if (!population_mode && !individual_mode && !rank_mode) {
        printf("please use -i -p -r to specify a mode (individual), (population), (ranking)");
    }
    
    
    if ((population_mode && individual_mode) || (population_mode && rank_mode) ||
        (individual_mode && rank_mode)) {
        printf("please specify only one mode \n");
    }

    char * s1 = strtok(tokens, ":");
    if (s1 == NULL) {
        printf("input is in bad format: %s \n", tokens);
        return (-1);
    }
    
    char * s2 = strtok(NULL, ":");
    if (s2 == NULL) {
        printf("input is in bad format: %s \n", tokens);
        return (-1);
    }
    
    if (individual_mode) {
        PacGenePtr p1 = malloc(sizeof(PacGene));
        PacGenePtr p2 = malloc(sizeof(PacGene));
        SetGeneFromString(s1, p1);
        SetGeneFromString(s2, p2);
        printf("gene1 %s \n", s1);
        printf("gene2 %s \n", s2);
        CompetionResult result;
        compete(p1, p2, &result);
        printf("%d vs %d \n", result.count1, result.count2);
        free(p1);
        free(p2);
    } else {
        
        GeneWrapper * p1 = NULL;
        GeneWrapper * p2 = NULL;
        int p1_size, p2_size;
        printf("population1 file: %s \n", s1);
        printf("population2 file: %s \n", s2);
        if (initialize_population(&p1, s1, false, &p1_size) != 0) {
            printf("reading p1 from %s failed \n", s1);
            return -1;
        }
        if (initialize_population(&p2, s2, false, &p2_size) != 0) {
            printf("reading p2 from %s failed \n", s2);
            return -1;
        }
        
        printf("population 1 size: %d \n", p1_size);
        printf("population 2 size: %d \n", p2_size);
        
        if (population_mode){
            
            double p1_total_wins, p1_total_loses, p2_total_wins, p2_total_loses = 0.0;
            
            int i;
            CompetionResult result;
            for (i = 0; i < p1_size; i++) {
                compete_againt_population(p1[i].gene, p2, p2_size, &result);
                p1_total_wins += result.count1/p2_size;
                p1_total_loses += result.count2/p2_size;
            }
            
            for (i = 0; i < p2_size; i++) {
                compete_againt_population(p2[i].gene, p1, p1_size, &result);
                p2_total_wins += result.count1/p1_size;
                p2_total_loses += result.count2/p1_size;
            }
            
            printf("###################\nResult:\n");
            printf("p1 wins %f loses %f \n", p1_total_wins, p1_total_loses);
            printf("p2 wins %f loses %f \n", p2_total_wins, p2_total_loses);
        } else {
            int i = 0;
            char buffer[51];
            for (i = 0; i < p1_size; i++) {
                long score = compute_score(&p1[i], p2, p2_size);
                NewStringFromGene(p1[i].gene, buffer);
                printf("score: %ld %s\n", score, buffer);
            }
        }
    }

    return 0;
}
