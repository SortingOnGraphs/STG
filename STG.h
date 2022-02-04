/*
 * This source code is a simple C++ implementation of GAs for Young tableaux (or Young tabloid).
 * Copyright (C) Dohan Kim <dkim@airesearch.kr>
 * Author : Dohan Kim < dkim@airesearch.kr >
 *
 * File Name : STG.h
 * Description : Header file of STG
 *
 */

#ifndef _STG_H
#define _STG_H

#include <vector>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <string>
#include <iostream>

using namespace std;

typedef struct
{
	int *tableau_shape;
	bool young_tableau;
	int num_of_partitions;
} YoungTableau;

typedef struct
{
	float crossover_rate;
	float mutation_rate;
	float fitness_weight;
} GAParams;

typedef struct
{
	char *job_name;
	int num_of_tasks;
	int num_of_hosts;
	int *precedence_constraints_indices;
	int **precedence_constraints_list;
	float *processing_power_list;
	float *task_length_list;
	vector<string> hosts_list;
	char *parameter_input_file;
	GAParams gaParams;
} STGIO;

typedef struct
{
	int *rank;
	float *fitness_array;
	float *exp_array;
} Fitness_Rank;

class STG
{
  private:
	STGIO *stgIO;	
	YoungTableau *tableau;
	Fitness_Rank *fitness_rank;
	int **population;
	int **current_population;
	int *best_fitness_individual;
	int *candidate_solution_individual;
	float candidate_solution_fitness_value;
	float *fitness_array;
	float *current_fitness_array;
	float total_exp_values;
	float total_fitness_values;
	float current_total_fitness_values;
	int num_of_population;
	int num_of_generations;
	int generation_cnt;
	int best_individual_index;
	int current_best_individual_index;
	int current_marked_individual_index;
  public:
	STG( STGIO *stgIO, YoungTableau *tableau );
	~STG();
	int stg_next_generation();
	int stg_select_initialization();
	int* stg_select_rank();
	float stg_fitness( int *individual );
	int stg_random_shuffle( int *individual );
	int stg_tmp_crossover( int *parent1, int *parent2 );
	int stg_crossover( int *parent1, int *parent2 );
	int stg_mutate( int *individual );
	int stg_get_num_of_generations();
	int stg_print_crossover( int *crossover_mask, 
		int *parent1, int *parent2, int *child1, int *child2 );
	int stg_print_population_tableau();
	int stg_print_population_tabloid();
	int stg_print_population();
	int stg_print();
};

extern STGIO vsmIO;
extern int stg_print( STGIO *stgIO );
extern int stg_initialize( STGIO *stgIO );

#endif /* _STG_H */
