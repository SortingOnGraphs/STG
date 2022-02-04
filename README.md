Simple tableaux-based genetic algorithms (STG), is to show how Young tableaux and their partitions are applied to the existing genetic algorithms for task assignment problems. This is the implementation for the paper published in 2016: "Representations of task assignments in distributed systems using Young tableaux and symmetric groups", Dohan Kim, International Journal of Parallel, Emergent and Distributed Systems, vol. 31, no. 2, pp. 152-175, 2016.

The input of STG specifies the number of tasks, number of agents, the list of task lengths, the list of processing capacities of agents, parameters of genetic algorithms, etc. The output of STG shows the candidate solution represented by the one-line permutation notation for the n-task-n-agent assignment problem after a certain number of generations.

To compile: make

To clean: make clean

To run:
./stg job_profile > log_name

example)
./stg simpleApp.txt > simpleApp_log.txt
./stg 20tasks.txt > 20tasks_logs.txt

 

An example of (file) input and output of STG for the n-task-n-agent-assignment problem for n = 20.
1: Input:
2: NumberOfTasks 20
3: NumberOfAgents 20
4: . . .
5: TaskLength 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160 170 180 190 200
6: ProcessingCapacity 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160 170 180 190 200
7: . . .
8: #GAs
9: CrossoverRate 0.8
10: MutationRate 0.005
11: . . .
12: #Young Tableau: Y, Young Tabloid: N
13: YoungTableau Y
14: NumberOfPartitions 4
15: TableauShape 6 6 4 4
16: . . .
17: 1
18: 2
19: . . .
20: 19 16 17
21: 20 18 19

22: Output:
23: (GENERATION 800)
24: ********************** Candidate Solution********************
25: Candidate Solution Fitness:0.205212
26: 1 2 3 4 5 6 7 9 8 11 12 13 14 16 10 17 18 15 19 20
27: . . .
28: Tableau Shape: 6 6 4 4
29: . . .
30: {{1; 2; 3; 4; 5; 6}; {7; 9; 8; 11; 12; 13}; {14; 16; 10; 17}; {18; 15; 19; 20}},
31: . . .

 

References

[1] Representations of task assignments in distributed systems using Young tableaux and symmetric groups, Dohan Kim, International Journal of Parallel, Emergent and Distributed Systems, vol. 31, no. 2, pp. 152-175, 2016.
