/*
 * This source code is a simple C++ implementation of GAs for Young tableaux (or Young tabloid).
 * Copyright (C) Dohan Kim <dkim@airesearch.kr>
 * Author : Dohan Kim < dkim@airesearch.kr >
 *
 * File Name : main.cpp
 * Description : main file of STG
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

STGIO stgIO;
YoungTableau tableau;

void stgIO_initialize( STGIO *stgIO )
{
	char line[1024];
	char buf[1024];
	vector<string> temp_list;
	char *pc_line = NULL, *pc_line_ptr = NULL;
	int x = 0, y = 0;
	FILE *fptr = NULL;

	if( ( fptr = fopen( stgIO->parameter_input_file, "r" ) ) == NULL )
		cout << "FILE OPEN ERROR"<<endl;

	while ( fgets( line, sizeof( line ), fptr ) != NULL )
	{
		size_t i = strspn( line, " \t\n\v" );

		if ( line[i] == '#' )
			continue;
		else if( !strncmp( line, "NumberOfTasks", 13 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );
			stgIO->num_of_tasks = atoi( buf );
		}
		else if( !strncmp( line, "NumberOfAgents", 14 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );
			stgIO->num_of_hosts = atoi( buf );
		}
		else if( !strncmp( line, "Agent_List", 10 ) ) {
			char *temp_ptr = NULL;
			int index = 0;
			memset( buf, '\0' , sizeof( buf ) );
			strcpy( buf, line );
			temp_ptr = strtok( buf, "\t ");

			while( temp_ptr != NULL && index < stgIO->num_of_hosts)
			{
				temp_ptr = strtok( NULL, "\t\n " );
				stgIO->hosts_list.push_back( string( temp_ptr ) );
				index++; }
		}
		else if( !strncmp( line, "Task_Length", 11 ) )
		{
			char *temp_ptr = NULL;
			int index = 0;
			stgIO->task_length_list = 
				( float * )malloc( sizeof( float ) * stgIO->num_of_tasks );
			memset( buf, '\0' , sizeof( buf ) );
			strcpy( buf, line );
			
			temp_ptr = strtok( buf, "\t ");

			while( temp_ptr != NULL && index < stgIO->num_of_tasks)
			{
				temp_ptr = strtok( NULL, "\t " );
				stgIO->task_length_list[index++] = atof( temp_ptr );
			}
		}
		else if( !strncmp( line, "Processing_Capacity", 19 ) )
		{
			char *temp_ptr = NULL;
			int p = 0;
			stgIO->processing_power_list = 
				( float *)malloc( sizeof( float ) * stgIO->num_of_hosts );
			memset( buf, '\0' , sizeof( buf ) );
			strcpy( buf, line );

			temp_ptr = strtok( buf, "\t ");

			while( temp_ptr != NULL && p < stgIO->num_of_hosts )
			{
				temp_ptr = strtok( NULL, "\t " );
				stgIO->processing_power_list[ p++ ] = atof( temp_ptr );
			}
		}
		else if( !strncmp( line, "CrossoverRate", 13 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );
			stgIO->gaParams.crossover_rate = atof( buf );
		}
		else if( !strncmp( line, "MutationRate", 12 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );
			stgIO->gaParams.mutation_rate = atof( buf );
		}
		else if( !strncmp( line, "FitnessWeight", 13 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );
			stgIO->gaParams.fitness_weight = atof( buf );
		}
		else if( !strncmp( line, "YoungTableau", 12 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );

			if( !strncmp( buf, "Y", 1 ) )
				tableau.young_tableau = true;
			else
				tableau.young_tableau = false;
		}
		else if( !strncmp( line, "NumberOfPartitions", 18 ) )
		{
			memset( buf, '\0' , sizeof( buf) );
			sscanf( line,"%*s %s", buf );
			tableau.num_of_partitions = atoi( buf );
		}
		else if( !strncmp( line, "TableauShape", 12 ) )
		{
			char *temp_ptr = NULL;
			int index = 0;
			tableau.tableau_shape =
				( int * )malloc( sizeof( int ) * tableau.num_of_partitions );
			memset( buf, '\0' , sizeof( buf ) );
			strncpy( buf, line, strlen( line ) );

			temp_ptr = strtok( buf, "\t ");

			while( temp_ptr != NULL && index < tableau.num_of_partitions )
			{
				temp_ptr = strtok( NULL, "\t " );
				tableau.tableau_shape[index++] = atoi( temp_ptr );
			}
		}
		else if( !strncmp( line, "Precedence_Indices", 18 ) )
		{
			char *t_ptr = NULL;
			int p_index = 0;
			stgIO->precedence_constraints_indices = new int[ stgIO->num_of_tasks ];
			memset( buf, '\0' , sizeof( buf ) );
			strcpy( buf, line );
			t_ptr = strtok( line, "\t ");
            
			while( t_ptr != NULL && p_index < stgIO->num_of_tasks )
			{
				t_ptr = strtok( NULL, "\t " );
				stgIO->precedence_constraints_indices[p_index++] = atoi( t_ptr );
			}
		}
		else
		{
			if( (int)line[i] < 48 || (int)line[i] > 57 ) // Accept only numbers
				continue;

			string temp = line;

			temp_list.push_back( temp );
		}
	}

	stgIO->precedence_constraints_list = new int*[stgIO->num_of_tasks];

	for ( x = 0; x < stgIO->num_of_tasks; x++ )
	{
		if( stgIO->precedence_constraints_indices[x] == 0 )
		{
			stgIO->precedence_constraints_list[x] = new int[1];
			stgIO->precedence_constraints_list[x][0] = -1;
		}
		else
		{
			stgIO->precedence_constraints_list[x] 
				= new int[stgIO->precedence_constraints_indices[x]];

			pc_line = ( char * )temp_list[x].c_str();
			pc_line_ptr = strtok( pc_line, "\t " );

			while( pc_line_ptr != NULL && y < stgIO->precedence_constraints_indices[x] )
			{
				pc_line_ptr = strtok( NULL, "\t " );
				stgIO->precedence_constraints_list[x][y++] = atoi( pc_line_ptr );
			}
		}
		pc_line = NULL;	
		pc_line_ptr = NULL;	
		y = 0;
	}
}

void stgIO_print( STGIO *stgIO )
{
	cout<<"Number of Tasks:"<<stgIO->num_of_tasks<<endl;
    cout<<"Number of Hosts:" <<stgIO->num_of_hosts<<endl;
	cout<<"Parameter Input File :" <<stgIO->parameter_input_file<<endl;
	cout<<"Crossover Rate :" <<stgIO->gaParams.crossover_rate<<endl;
	cout<<"Mutation Rate :" <<stgIO->gaParams.mutation_rate<<endl;
	cout<<"Hosts List: ";

	for( int a = 0 ; a < stgIO->num_of_hosts ; a++ )
		cout<<stgIO->hosts_list[a]<<" ";
	cout<<endl;

	cout<<"Processing Capacity List"<<endl;
	for( int i = 0 ; i < stgIO->num_of_hosts ; i++ )
    	cout<<stgIO->processing_power_list[i]<<" ";
	cout<<endl;

	cout<<"Task Length List"<<endl;

	for( int j = 0 ; j < stgIO->num_of_tasks ; j++ )
		cout<<stgIO->task_length_list[j]<<" ";
	cout<<endl;

	for( int k = 0 ; k < stgIO->num_of_tasks ; k++ )
		cout<<stgIO->precedence_constraints_indices[k]<<" ";
	cout<<endl;

	for( int u = 0 ; u < stgIO->num_of_tasks ; u++ )
	{
		int loop;

		if ( stgIO->precedence_constraints_indices[u] == 0 )
			loop = 1;
		else
			loop = stgIO->precedence_constraints_indices[u]; 

		for( int v = 0 ; v < loop ; v++ )
			cout<<stgIO->precedence_constraints_list[u][v]<<" ";
		cout<<endl;
	}

	cout<<"Young Tableau ? "<<tableau.young_tableau<<endl;
	cout<<"Number of Partitions: "<<tableau.num_of_partitions<<endl;
	cout<<"Tableau Shape: "<<endl;

	for( int i = 0; i < tableau.num_of_partitions; i++ )
	{
		cout<<tableau.tableau_shape[i]<<" ";
	}

	cout<<endl;
	
    fflush( stdout );
}

int main(int argc, char **argv)
{
	int generation_cnt = 0;
	char ptr[256], *ptr2;

	if( argc != 2 )
	{
		cout<<"Usage: ./stg parameter_input_file"<<endl;
		exit( 1 );
	}

	srand(time(NULL));

	stgIO.parameter_input_file = argv[1];
	strncpy( ptr, argv[1], 256 );
	ptr2 = strtok ( ptr, "." );
	stgIO.job_name = ptr2;
	stgIO_initialize( &stgIO );
//	stgIO_print( &stgIO );

	STG stg ( &stgIO, &tableau );

	do
	{	
		cout << "(GENERATION "<<generation_cnt + 1 <<")"<<endl<<endl;
		stg.stg_next_generation();
		generation_cnt++;
	} while ( generation_cnt < stg.stg_get_num_of_generations() );

	return 1;
}
