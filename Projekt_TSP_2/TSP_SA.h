#pragma once
#include  <vector>
#include  <iostream>
#include <chrono> // czas
#include <windows.h>
#include <iomanip>
class TSP_SA
{
	int N; //Liczba wierzcho³ków w grafie = matrix size
	int **matrix_distance;

	float alfa; //sta³a obni¿ania temperatury
	int best_cost;
	float minimum(int x, int y);
	int calculate_cost_SA(std::vector<int>& permutation);
	std::vector<int> generate_best_neighbour(std::vector<int>permutation);
	float P(int x0, int x, float Ti);

	long long int read_QPC();
	long long int frequency, start, elapsed;
public:
	TSP_SA(int number_of_vertices, int** A);
	void solve(float temp_start, float temp_min, float temp_cooling, int iterations, double max_time_s);
	int get_cost() { return best_cost; }
	~TSP_SA();
};

