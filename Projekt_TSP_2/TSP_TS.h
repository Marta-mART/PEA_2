#pragma once
#include  <vector>
#include  <iostream>
#include <ctime>
#include <chrono> // czas
#include <windows.h>
#include <iomanip>


class TSP_TS
{
	int N; //Liczba wierzcho³ków w grafie = matrix size
	int **matrix_distance;
	int **tabu_list;
	bool neighbour_criterion;

	int best_cost;

	int calculate_cost(std::vector<int>& permutation);
	void clear_tabu_list();
	void random_swap(std::vector<int> &route);
	std::vector<int> generate_best_neighbour(std::vector<int>permutation, int lifetime);
	std::vector<int> TSP_TS::generate_best_neighbour_random(std::vector<int> permutation, int lifetime);

	long long int read_QPC();
	long long int frequency, start, elapsed;
public:
	TSP_TS(int number_of_vertices, int** A);
	~TSP_TS();
	void solve(int iterations, int lifetime, int max_critical_events, double max_time_s, int neighbour_criterion);
	int get_cost() { return best_cost;	}
};

