//
//  main.c
//  Pacwar
//
//  Created by Matt Zhao on 9/10/14.
//  Copyright (c) 2014 Matt Zhao. All rights reserved.
//

#include "Utils.h"
#include <pthread.h>
#include <stdio.h>


extern char * trace_file_name;
char initial_population_trace[] = "/Users/mattzhao/Desktop/Github/PacGene/linux/init_population.txt";

int main(int argc, const char * argv[]) {
    
    int i, j, population_size;
    // clear_trace_file();
    
    // initialize initial population
    GeneWrapper * population = NULL;
    initialize_population(&population, initial_population_trace, true, &population_size);
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
