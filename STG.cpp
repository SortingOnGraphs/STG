/*
 * This source code is a simple C++ implementation of GAs for Young tableaux (or Young tabloid).
 * Copyright (C) Dohan Kim <dkim@airesearch.kr>
 * Author : Dohan Kim < dkim@airesearch.kr >
 *
 * File Name : STG.cpp
 * Description : Header file of STG
 *
 */

#include "STG.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace std;

STG::STG ( STGIO *stgIO, YoungTableau *tableau )
{
	this->stgIO = stgIO;
	this->tableau = tableau;
	this->num_of_population = stgIO->num_of_tasks * 4;
	this->num_of_generations = stgIO->num_of_tasks * 40;
	this->total_exp_values = 0.0;
	this->total_fitness_values = 0.0;
	this->current_total_fitness_values = 0.0;
	this->best_individual_index = 0;
	this->generation_cnt = 0;
	this->candidate_solution_fitness_value = 0.0;

	this->best_fitness_individual = ( int * )malloc( this->stgIO->num_of_tasks * sizeof(int ) );
	this->candidate_solution_individual = ( int * )malloc( this->stgIO->num_of_tasks * sizeof(int ) );
	this->fitness_array = ( float * )malloc( this->num_of_population * sizeof( float ) );
	this->current_fitness_array = ( float * )malloc( this->num_of_population * sizeof( float ) );
	this->fitness_rank = ( Fitness_Rank * )malloc( sizeof( Fitness_Rank ) );
	this->fitness_rank->rank = ( int * )malloc( this->num_of_population * sizeof( int ) );
	this->fitness_rank->exp_array = ( float * )malloc( this->num_of_population * sizeof( float ) );
	this->fitness_rank->fitness_array = ( float * )malloc( this->num_of_population * sizeof( float ) );

	this->population = ( int ** )malloc( this->num_of_population * sizeof( int ) );
	this->current_population = ( int ** )malloc( this->num_of_population * sizeof( int ) );

	for( int i = 0; i < this->num_of_population; i++ ) 
	{
		this->population[i] = ( int * )malloc( this->stgIO->num_of_tasks * sizeof( int ) );
		this->current_population[i] = ( int * )malloc( this->stgIO->num_of_tasks * sizeof( int ) );
		stg_random_shuffle( this->population[i] );
		this->fitness_array[i] = stg_fitness( population[i] );
		this->total_fitness_values += this->fitness_array[i];

		if( this->fitness_array[i] > this->fitness_array[this->best_individual_index] )
		{
			this->best_individual_index = i;
		}
	}
	
	stg_print_population();
}

STG::~STG()
{
	free( this->fitness_rank->rank );
	free( this->fitness_rank->fitness_array );
	free( this->fitness_rank->exp_array );
	free( this->fitness_rank );

	for( int i = 0; i < this->num_of_population; i++ ) 
	{
		free( this->population[i] );
		free( this->current_population[i] );
	}

	free( this->population );
	free( this->current_population);
	free( this->fitness_array);
	free( this->current_fitness_array );
	free( this->candidate_solution_individual );
}

/*
 * Rank selection
 */

int STG::stg_select_initialization()
{
	this->total_exp_values = 0.0;

	float MAX_EXP = 1.7, MIN_EXP = 0.3;

	memcpy( this->fitness_rank->fitness_array, this->fitness_array, this->num_of_population * sizeof( float ) );	

	sort( this->fitness_rank->fitness_array, this->fitness_rank->fitness_array + this->num_of_population );

	for( int j = 0; j < this->num_of_population; j++ )
	{
		for( int k = 0; k < this->num_of_population; k++ )
		{
			if( this->fitness_array[j] == this->fitness_rank->fitness_array[k] )
			{
				this->fitness_rank->rank[j] = k + 1;
				break;
			}
		}
	}

	for( int m = 0; m < this->num_of_population; m++ )
	{
		this->fitness_rank->exp_array[m] = 
			MIN_EXP + ( MAX_EXP - MIN_EXP )*( ( this->fitness_rank->rank[m] - 1.0 )/ ( this->num_of_population - 1.0 ) );
	}

	for( int n = 0; n < this->num_of_population; n++ )
	{
		this->total_exp_values += this->fitness_rank->exp_array[n];
	}

	return 1;
}

int STG::stg_random_shuffle( int *individual )
{
	int *hosts = ( int * )malloc( sizeof( int ) * this->stgIO->num_of_hosts );
	int *tasks = ( int * )malloc( sizeof( int ) * this->stgIO->num_of_tasks );

	for( int i = 0; i < this->stgIO->num_of_hosts; i++ )
	{
		hosts[i] = i + 1;
	}

	random_shuffle( hosts, hosts + this->stgIO->num_of_hosts );

	if( this->stgIO->num_of_hosts >= this->stgIO->num_of_tasks )
	{
		for( int j = 0; j < this->stgIO->num_of_tasks; j++ )
			tasks[j] = hosts[j];
	}
	else
	{
		for( int k = 0; k < this->stgIO->num_of_tasks; k++ )
            tasks[k] = hosts[rand() % this->stgIO->num_of_hosts];
	}

	memcpy( individual, tasks, this->stgIO->num_of_tasks * sizeof(int) );

	return 1;
}

/*
 * In this function we referred an algorithm of FGA's cycle() function.
 * 
 *
 *	FGA - Fast Genetic Algorithm
 *	Copyright (C) 2007 Alessandro Presta (alessandro.presta@gmail.com)
 *	All rights reserved.
 * 
 *  http://fga.sourceforge.net/
 */

int STG::stg_next_generation()
{
	this->current_best_individual_index = 0;
	this->current_total_fitness_values = 0.0;
	this->current_marked_individual_index = 0;

	stg_select_initialization();

	for (int i = 0; i < ( this->num_of_population / 2) * 2; i += 2) 
	{
		int *parent1 = stg_select_rank();
		int *parent2 = stg_select_rank();

		int *child1 = current_population[i];
		int *child2 = current_population[i + 1];

		memcpy( child1, parent1, this->stgIO->num_of_tasks * sizeof( int ) );
		memcpy( child2, parent2, this->stgIO->num_of_tasks * sizeof( int ) );	

		stg_crossover( child1, child2 );
		stg_mutate( child1 );
		stg_mutate( child2 );

		this->current_fitness_array[i] = stg_fitness( child1 );
		this->current_fitness_array[i + 1] = stg_fitness( child2 );
		this->current_total_fitness_values += 
			this->current_fitness_array[i] + this->current_fitness_array[i + 1]; 

		if( current_fitness_array[i] > current_fitness_array[current_best_individual_index] )
			current_best_individual_index = i;
		if( current_fitness_array[i + 1] > current_fitness_array[current_best_individual_index] )
			current_best_individual_index = i + 1;
		if( current_fitness_array[i] < current_fitness_array[current_marked_individual_index] )
			current_marked_individual_index = i;
		if( current_fitness_array[i + 1] < current_fitness_array[current_marked_individual_index] )
			current_marked_individual_index = i + 1;
	}

	int **tmp_population = this->population;
	this->population = this->current_population;
	this->current_population = tmp_population;
	float *tmp_fitness_array = this->fitness_array;
	fitness_array = this->current_fitness_array;
	this->current_fitness_array = tmp_fitness_array;
	this->total_fitness_values = this->current_total_fitness_values;
	this->best_individual_index = this->current_best_individual_index;
	this->generation_cnt++;

	if (candidate_solution_fitness_value < fitness_array[this->best_individual_index]) 
	{
		memcpy( this->candidate_solution_individual, 
			this->population[this->best_individual_index], this->stgIO->num_of_tasks * sizeof( int ) );
		this->candidate_solution_fitness_value = fitness_array[this->best_individual_index];

		cout<<" ********************** Candidate Solution********************"<<endl<<endl;
		cout<<"Candidate Solution Fitness:"<< this->candidate_solution_fitness_value <<endl<<endl;
   
		for( int x = 0; x < this->stgIO->num_of_tasks; x++ )
		{
			cout<<this->candidate_solution_individual[x]<<" ";
		}
		cout << endl;
	}	
	else /* Eliticism : Keep the best individual to the next generation */
	{
		cout<<" ********************** Candidate Solution********************"<<endl<<endl;
		cout<<"Candidate Solution Fitness:"<< this->candidate_solution_fitness_value <<endl<<endl;

		for( int n = 0; n < this->stgIO->num_of_tasks; n++ )
		{
			cout<<this->candidate_solution_individual[n]<<" ";
		}

		cout << endl<<endl;
		
		memcpy( this->population[this->current_marked_individual_index], 
			this->candidate_solution_individual, this->stgIO->num_of_tasks * sizeof( int ) ); 
	}

	cout << endl<<"Average Fitness:";
	cout <<this->total_fitness_values / (float)this->num_of_population<<endl;

	stg_print_population();

	if( this->tableau->young_tableau )
		stg_print_population_tableau();
	else
		stg_print_population_tabloid();


	return 1;
}

int * STG::stg_select_rank()
{
	float wheel_index = (float)rand() / RAND_MAX * this->total_exp_values;
	float index = 0.0;

	for( int i = 0; i < this->num_of_population; i++ )
	{
		if( index + this->fitness_rank->exp_array[i] >= wheel_index )
		{
			return this->population[i];
		}
		else
		{
			index += this->fitness_rank->exp_array[i];
		}
	}
	
	return NULL;
}

float STG::stg_fitness( int *individual )
{
	float fitness_value = 0.0, length = 0.0, temp_val = 0.0, temp_max = 0.0;
	float job_completion_factor = 0.0, resource_utilization_factor = 0.0;

	float **taskspan = ( float ** )malloc( this->stgIO->num_of_tasks * sizeof( float * ) );
	
	for( int i = 0; i < this->stgIO->num_of_tasks; i++ )
		taskspan[i] = ( float * )malloc( 2 * sizeof( float ) );

	for( int j = 0; j < this->stgIO->num_of_tasks; j++ )
	{
		length = this->stgIO->task_length_list[j] / this->stgIO->processing_power_list[individual[j] - 1];

		if( this->stgIO->precedence_constraints_indices[j] == 0 )
		{
			taskspan[j][0] = 0.0; 		// starting time
			taskspan[j][1] = length;	// completion time
		}
		else
		{
			vector<float> temp_vector;
			vector<float>::iterator start;
			
			for( int k = 0; k < this->stgIO->precedence_constraints_indices[j]; k++ )
				temp_vector.push_back( taskspan[this->stgIO->precedence_constraints_list[j][k] - 1][1] );	

			start = max_element( temp_vector.begin(), temp_vector.end() );

			taskspan[j][0] = *start;	
			taskspan[j][1] = *start + length;
		}
	}

	for( int m = 0; m < this->stgIO->num_of_tasks; m++ )
	{
		temp_val = taskspan[m][1];

		if( temp_val > temp_max )
			temp_max = temp_val;
	}

	if( this->stgIO->gaParams.fitness_weight == 0.0 )
	{
		fitness_value = 1.0 / temp_max ;
	}
	else
	{
		job_completion_factor = 1.0 / temp_max;
		
		for( int n = 0; n < this->stgIO->num_of_tasks; n++ )
			resource_utilization_factor += ( ( taskspan[n][1] - taskspan[n][0] ) / temp_max );

		resource_utilization_factor = resource_utilization_factor / (float)this->stgIO->num_of_tasks;				
		fitness_value = 
			( ( 1.0 - this->stgIO->gaParams.fitness_weight ) * job_completion_factor ) + 
			( this->stgIO->gaParams.fitness_weight * resource_utilization_factor );

		cout <<"job_completion_factor: "<<job_completion_factor<<endl;
		cout <<"resource_utilization_factor: "<<resource_utilization_factor<<endl;
	}

	for( int n = 0; n < this->stgIO->num_of_tasks; n++ )
		free( taskspan[n] );

	free( taskspan );

	return fitness_value;
}

/* 
 * Uniform Permutation Crossover
 */

int STG::stg_crossover( int *parent1, int *parent2 )
{
	if (((float)rand()/RAND_MAX) <= this->stgIO->gaParams.crossover_rate )
	{
		int *crossover_mask = ( int * )malloc( sizeof( int ) * this->stgIO->num_of_tasks );
		int *child1 = ( int * )malloc( sizeof( int ) * this->stgIO->num_of_tasks );
		int *child2 = ( int * )malloc( sizeof( int ) * this->stgIO->num_of_tasks );

		vector<int> child1_selected, child2_selected;
		vector<int> parent1_remainder, parent2_remainder;

		for( int i = 0; i < this->stgIO->num_of_tasks; i++ )
		{
			crossover_mask[i] = rand() % 2;
		}

		for( int j = 0; j < this->stgIO->num_of_tasks; j++ )
		{
			if( crossover_mask[j] == 1 )
			{
				child1[j] = parent1[j];
				child1_selected.push_back( child1[j] );
			}
			else
			{
				child2[j] = parent2[j];
				child2_selected.push_back( child2[j] );
			}
		}

		for( int k = this->stgIO->num_of_tasks - 1; k >= 0 ; k-- )
		{
			if( child1_selected.size() == 0 )
			{
				parent2_remainder.push_back( parent2[k] );
			}
			else
			{
				for( int m = 0; m < (int) child1_selected.size(); m++ )
				{
					if( parent2[k] == child1_selected[m] )
						break;
					else
					{
						if( m == (int)child1_selected.size() - 1 )
							parent2_remainder.push_back( parent2[k] );
					}
				}
			}

			if( child2_selected.size() == 0 )
			{
				parent1_remainder.push_back( parent1[k] );
			}
			else
			{
				for( int n = 0; n < (int)child2_selected.size(); n++ )
				{
					if( parent1[k] == child2_selected[n] )
						break;
					else
					{
						if( n == (int)child2_selected.size() - 1 )
							parent1_remainder.push_back( parent1[k] );
					}
				}
			}
		}
	
		for( int u = 0; u < this->stgIO->num_of_tasks; u++ )
		{
			if( crossover_mask[u] == 0 )
			{
				if( !parent2_remainder.empty() )
				{
					child1[u] = parent2_remainder.back();
					parent2_remainder.pop_back();
				}
			}
			else
			{
				if( !parent1_remainder.empty() )
				{
					child2[u] = parent1_remainder.back();
					parent1_remainder.pop_back();
				}
			}
		}

		/* stg_print_crossover( crossover_mask, parent1, parent2, child1, child2 ); */

		memcpy( parent1, child1, this->stgIO->num_of_tasks * sizeof( int ) );
		memcpy( parent2, child2, this->stgIO->num_of_tasks * sizeof( int ) );

		child1_selected.empty();
		child2_selected.empty();
		parent1_remainder.empty();
		parent2_remainder.empty();

		free( child1 );
		free( child2 );
		free( crossover_mask );
	}

	return 1;
}

/*
 * Swap Mutation
 */

int STG::stg_mutate( int *individual )
{
	int a = rand() % stgIO->num_of_tasks;
	int b = rand() % stgIO->num_of_tasks;

	if( ( ( float )rand()/RAND_MAX ) <= stgIO->gaParams.mutation_rate * ( 1.0 / (float)(abs(a-b) + 1.0 ) ) )
	{
		int tmp = individual[a];
		individual[a] = individual[b];
		individual[b] = tmp;

		cout<<endl<<" Mutation arises "<<endl;

		return 1;
	}

	return 1;
}

int STG::stg_print_population()
{
	cout<<endl<<"***************** POPULATION ********************"<<endl;
	cout <<"list := [";

	for( int i = 0; i < this->num_of_population; i++ )
	{
		cout <<"[";

		for( int j = 0; j < stgIO->num_of_tasks; j++ )
		{
			if( j != ( stgIO->num_of_tasks -1 ) )
				cout<<population[i][j]<<", ";
			else
				cout<<population[i][j];
		}

		if( i != ( this->num_of_population -1 ) )
			cout <<"]," <<endl;
		else
			cout <<"]";
	}

	cout <<"]; "<<endl;
	cout<<"*******************************************************"<<endl;

	return 1;
}

/* 
 * Print the population with the Young Tableau format
 */

int STG::stg_print_population_tableau()
{
	cout<<endl<<"********* POPULATION( Young Tableau Representation )********************"<<endl;

	cout<<"Tableau Shape: ";

	for( int i = 0; i < this->tableau->num_of_partitions; i++ )
	{
		cout<<this->tableau->tableau_shape[i]<<" ";
	}

	cout<<endl<<endl;

	cout<<"{ ";

	for( int i = 0; i < this->num_of_population; i++ )
	{
		int m = 0,  n = 0;

		cout<<"{";

		for( int j = 0; j < this->stgIO->num_of_tasks; j++ )
		{
			if( j == n )
			{
				cout <<"{";
				n += this->tableau->tableau_shape[m++];
			}
				
			cout<<this->population[i][j];

			if( j != this->stgIO->num_of_tasks - 1 && j == n-1 )
				cout<<"},";
			else if ( j != this->stgIO->num_of_tasks -1 )
				cout<<",";
		}

		if( i !=  this->num_of_population - 1 )
			cout<<"}}, "<<endl;
		else
			cout<<"}}";
	}

	cout<<" }"<<endl;
	cout<<"*******************************************************"<<endl;

	return 1;
}

/*
 * Print the population with the Young Tabloid format
 */

int STG::stg_print_population_tabloid()
{
	int **sorted_population;
	sorted_population = new int*[num_of_population];

	for( int n = 0; n < this->num_of_population; n++ )
	{
		sorted_population[n] = new int[stgIO->num_of_tasks];
	}

	for( int r = 0; r < this->num_of_population; r++ )
		for( int s = 0; s < this->stgIO->num_of_tasks; s++ )
			sorted_population[r][s] = population[r][s];

	cout<<endl<<"******** POPULATION( Young Tabloid Representation )*****************"<<endl;

	cout<<"Tabloid Shape: ";

	for( int i = 0; i < this->tableau->num_of_partitions; i++ )
	{
		cout<<this->tableau->tableau_shape[i]<<" ";
	}

	cout<<endl<<endl;

	cout<<"{ ";

	for( int i = 0; i < num_of_population; i++ )
	{
		int m = 0,  n = 0;

		cout<<"{";

		for( int j = 0; j < stgIO->num_of_tasks; j++ )
		{
			if( j == n )
			{
				sort( sorted_population[i] + j,  ( sorted_population[i] + j ) + tableau->tableau_shape[m] ); 
				cout <<"{";
				n += tableau->tableau_shape[m++];
			}

			cout<<sorted_population[i][j];

			if( j != stgIO->num_of_tasks - 1 && j == n-1 )
				cout<<"},";
			else if ( j != stgIO->num_of_tasks -1 )
				cout<<",";
		}

		if( i !=  num_of_population - 1 )
			cout<<"}}, "<<endl;
		else
			cout<<"}}";
	}

	cout<<" }"<<endl;
	cout<<"*******************************************************"<<endl;

	return 1;
}

int STG::stg_print_crossover( int *crossover_mask, int *parent1, int *parent2, int *child1, int *child2 )
{
	cout<<endl<<"Crossover_Mask"<<endl;
	for( int m = 0; m < this->stgIO->num_of_tasks; m++ )
		cout<<crossover_mask[m]<<" ";

	cout<<endl<<"Parent 1"<<endl;
	for( int a = 0; a < this->stgIO->num_of_tasks; a++ )
		cout<<parent1[a]<<" ";

	cout<<endl<<"Parent 2"<<endl;
	for( int b = 0; b < this->stgIO->num_of_tasks; b++ )
		cout<<parent2[b]<<" ";

	cout<<endl<<"Child 1"<<endl;
	for( int c = 0; c < this->stgIO->num_of_tasks; c++ )
		cout<<child1[c]<<" ";

	cout<<endl<<"Child 2"<<endl;
	for( int d = 0; d < this->stgIO->num_of_tasks; d++ )
		cout<<child2[d]<<" ";
	cout<<endl<<endl;

	return 1;
}

int STG::stg_get_num_of_generations()
{
	return this->num_of_generations;
}
