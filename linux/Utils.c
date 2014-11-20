#include "Utils.h"
#include <math.h>

#define max(a,b) (((a) > (b)) ? (a) : (b))


char trace_file_name[] = "trace";


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

void compete_againt_population(PacGenePtr p, GeneWrapper * population, int population_size, CompetionResult* result) {
    result->count1 = 0;
    result->count2 = 0;
    int wins = 0;
    int loses = 0;
    int i;
    for (i = 0; i < population_size; i++) {
        compete(p, population[i].gene, result);
        wins += result->count1;
        loses += result->count2;
    }
    result->count1 = wins;
    result->count2 = loses;
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


long compute_score(GeneWrapper * wrapper, GeneWrapper* population, int population_size) {
    CompetionResult result;
    long total_score = 0;
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

int gene_string_comparator (const void * elem1, const void * elem2)
{
    GeneWrapper * f = (GeneWrapper*)elem1;
    GeneWrapper * s = (GeneWrapper*)elem2;
    char g1[51];
    char g2[51];
    NewStringFromGene(f->gene, g1);
    NewStringFromGene(s->gene, g2);
    return strncmp(g1, g2, 50);
}



void remove_duplicates(GeneWrapper * population, int * population_size)
{
    MergeSort(population,
              * population_size,
              sizeof(population[0]), gene_string_comparator);
    char buffer[51];
    int i = -1, j = 0;
    while (j < * population_size) {
        if (i == -1 || gene_string_comparator(&population[i], &population[j]) != 0) {
            i += 1;
            NewStringFromGene(population[j].gene, buffer);
            SetGeneFromString(buffer, population[i].gene);
        } else {
            j += 1;
        }
    }
    *population_size = i + 1;
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


void rank_compete(GeneWrapper * population_to_rank, int population_to_rank_size,
                  GeneWrapper * base_population,    int base_population_size)
{
    int i, j;
    int score1, score2;
    for (i = 0; i < population_to_rank_size; i++) {
        population_to_rank[i].score = 0;
    }
    
    CompetionResult result;
    for (i = 0; i < population_to_rank_size; i++) {
        for (j = 0; j < base_population_size; j++) {
            compete(population_to_rank[i].gene, base_population[j].gene, &result);
            result2score(&score1, &score2, &result);
            population_to_rank[i].score += score1;
        }
    }
    
    MergeSort(population_to_rank,
              population_to_rank_size,
              sizeof(population_to_rank[0]), gene_score_comparator);
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

int initialize_population(GeneWrapper ** population, char * population_trace, bool random_initialize, int * population_size) {
    FILE *file;
    if ((file = fopen(population_trace, "r")) == NULL) {
        if (errno == ENOENT) {
            *population_size = POPULATION_SIZE;
            if (!random_initialize) {
                return -1;
            }
            printf("initial population cannot be found, randomly initialize one \n");
            GeneWrapper * initial_population = (GeneWrapper *) malloc(sizeof(GeneWrapper) * (*population_size));
            int i;
            char random_gene_string[51];
            for (i = 0; i < * population_size; i++) {
                rand_str(random_gene_string, 50);
                initial_population[i].gene = malloc(sizeof(PacGene));
                SetGeneFromString(random_gene_string, initial_population[i].gene);
            }
            * population = initial_population;
            return 0;
        } else {
            printf("some other error occured");
            return -1;
        }
    }else {
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
        *population = initial_population;
        return 0;
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


void reduce_population_through_ranking(GeneWrapper * population_after,  int population_size_after,
                                       GeneWrapper * population_before, int population_size_before,
                                       GeneWrapper * base_population,   int base_population_size)

{
    char buffer[51];
    int i = population_size_after - 1;
    int j = population_size_before - 1;
    GeneWrapper * population_before_copy = NULL;
    copy_population(&population_before_copy, population_before, population_size_before);
    
    rank_compete(population_before_copy, population_size_before, base_population, base_population_size);
    
    while (i >= 0) {
        NewStringFromGene(population_before_copy[j].gene, buffer);
        SetGeneFromString(buffer, population_after[i].gene);
        i--;
        j--;
    }
    free_population(population_before_copy, population_size_before);
}



void mutate_population(GeneWrapper * population, size_t population_size, double mutation_rate)
{
    int i,j;
    char buffer[51];
    bzero(buffer, 51);
    for (i = 0; i < population_size; i++) {
        GeneWrapper * p = &population[i];
        NewStringFromGene(p->gene, buffer);
        for (j = 0; j < 50; j++) {
            if (i <= 3) {
                continue;
            }
            printf("f %f \n" ,(double)rand() / RAND_MAX);
            if ((double)rand() / RAND_MAX < mutation_rate) {
                buffer[j] = '0' + (rand() % 4);
            }
        }
        SetGeneFromString(buffer, p->gene);
    }
}


# define NUM_CORSS_OVERS 7
void crossover_population_helper(bool * flags, int index, int flag_array_size,
                                 GeneWrapper * unitialize_genes, GeneWrapper* p1, GeneWrapper* p2,
                                 int * first_uninitialized_gene) {
    // printf("%d %d \n", index, flag_array_size);
    int i;
    if (index == flag_array_size) {
        char buffer[51];
        char p1buffer[51];
        char p2buffer[51];
        int offsets[NUM_CORSS_OVERS] =   {0, 6, 14, 23, 30, 39, 45};
        int sizes[NUM_CORSS_OVERS]   =      {6, 8,  9,  7,  9,  5, 5};
        bzero(buffer, 51);
        bzero(p1buffer, 51);
        bzero(p2buffer, 51);
        GeneWrapper * unitialized_gene = &(unitialize_genes[*first_uninitialized_gene]);
        
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
        mutate_population(unitialized_gene, 1, 0.02);
        *first_uninitialized_gene = (*first_uninitialized_gene) + 1;
    } else {
        bool options[2] = {true, false};
        for (i = 0; i < 2; i++) {
            flags[index] = options[i];
            crossover_population_helper(flags, index+1, flag_array_size, unitialize_genes, p1, p2, first_uninitialized_gene);
        }
    }
}

void crossover_population(GeneWrapper * population, int population_size,
                          GeneWrapper * base_population, int base_population_size)
{
    int i,j;
    int first_unitialized_gene = 0;
    int cross_over_size = population_size * (population_size-1) /2 * (int) (pow(2.0, 1.0 * NUM_CORSS_OVERS))  ;
    GeneWrapper * combined_populaton = (GeneWrapper *) malloc(sizeof(GeneWrapper) * cross_over_size);
    for (i = 0; i < population_size; i++) {
        for (j = i+1; j < population_size; j++) {
            GeneWrapper * p1 = &population[i];
            GeneWrapper * p2 = &population[j];
            bool flags[NUM_CORSS_OVERS];
            crossover_population_helper(flags, 0, NUM_CORSS_OVERS, combined_populaton, p1, p2, &first_unitialized_gene);
        }
    }
    
    int reduced_size = cross_over_size;
    remove_duplicates(combined_populaton, &reduced_size);
    reduce_population_through_ranking(population, population_size, combined_populaton, max(reduced_size,population_size), base_population, base_population_size);
    for (i = 0; i < cross_over_size; i++) {
        free(combined_populaton[i].gene);
    }
    free(combined_populaton);
}

void generate_new_generation(void *arg)
{
    struct thread_arguments_info *info = arg;
    int population_size = info->population_size;
    GeneWrapper * population = info->population;
    GeneWrapper * next_population = info->next_population;
    int i, num_iterations = NUM_ITERATIONS;

    GeneWrapper combined_populaton[population_size * 2];

    for (i = 0; i < population_size * 2; i++) {
        GeneWrapper * unitialized_gene = &combined_populaton[i];
        unitialized_gene->gene = malloc(sizeof(PacGene));
        unitialized_gene->score = 0;
    }

    char buffer[51];
    bzero(buffer, 51);
    while (num_iterations--  > 0) {

        for (i = 0; i < population_size * 2; i++) {
            NewStringFromGene(population[i % population_size].gene, buffer);
            SetGeneFromString(buffer, combined_populaton[i].gene);
        }
        printf("crossover_population \n");
        crossover_population(combined_populaton + population_size, population_size, population, population_size);
        mutate_population(combined_populaton, population_size, 0.06); // high mutation

        printf("reduce_population_through_ranking \n");
        int reduced_size = population_size * 2;
        remove_duplicates(combined_populaton, &reduced_size);
        reduce_population_through_ranking(combined_populaton, population_size, combined_populaton, max(reduced_size, population_size),
                                          population, population_size);
        for (i = 0; i < population_size; i++) {
            NewStringFromGene(combined_populaton[i].gene, buffer);
            SetGeneFromString(buffer, population[i].gene);
        }
    }
    
    for (i = 0; i < population_size * 2; i++) {
        free(combined_populaton[i].gene);
    }
    
    for (i = 0; i < population_size; i++) {
        NewStringFromGene(population[i].gene, buffer);
        SetGeneFromString(buffer, next_population[i].gene);
    }
}
