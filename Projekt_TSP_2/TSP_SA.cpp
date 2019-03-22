#include "TSP_SA.h"



float TSP_SA::minimum(int x, int y)
{
	return ((x<y)? x:y);
}

int TSP_SA::calculate_cost_SA(std::vector<int>& permutation)
{
	//pocz¹tkowy koszt drogi
	int cost = 0;

	for (int i = 0; i < N - 1; i++)
	{
		cost += matrix_distance[permutation.at(i)][permutation.at(i+1)];
	}
	//dodajemy koszt trasy o koszt powrotu do wierzcho³ka 0

	cost += matrix_distance[permutation.at(N - 1)][0];
	return cost;
}

std::vector<int> TSP_SA::generate_best_neighbour(std::vector<int> permutation)
{
	std::vector<int> next_neighbour = permutation;
	
		//dwie losowe wartoœci ró¿ne od zera
		int random_city = rand() % (N - 1) + 1;
		int another_random_city = rand() % (N - 1) + 1;

		while (random_city == another_random_city)
		{
			another_random_city = rand() % (N - 1) + 1;
		}

		// Zamieñ miasta
		std::swap(next_neighbour[random_city], next_neighbour[another_random_city]);

	return next_neighbour;
}

TSP_SA::TSP_SA(int number_of_vertices, int** A)
{
	N = number_of_vertices;
	

	//Kopiowanie macierzy A do macierzy matrix_distance
	// Wierszy tyle, ile wierzcholkow
	matrix_distance = new int *[N];

	// Kolumn tyle, ile wierzcho³ków
	for (int i = 0; i < N; i++) matrix_distance[i] = new int[N];

	// Wczytanie macierzy, ale zast¹pienie 0 na przek¹tnej -1
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			matrix_distance[i][j] = A[i][j];
			if (i == j)	matrix_distance[i][j] = 0;
		}
}



void TSP_SA::solve(float temp_start, float temp_min, float temp_cooling, int iterations, double max_time_s)
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
	//start zliczania czasu
	double elapsed_time = 0;

	if(temp_cooling < 1) alfa = temp_cooling;
	else alfa =0.1f; //na sztywno

	int Tmax;
	if(temp_start > 0) Tmax = temp_start; //jako sta³a
	else Tmax = 100; //na sztywno

	int Tmin;
	if(temp_min < temp_start) Tmin = temp_min;
	else Tmin = 0.01 * Tmax; //na sztywno
						  
	std::vector <int>  permutation(N);	//aktualna, obecna trasa
	std::vector <int>  new_permutation(N);
	int new_cost;
	int previous_cost;



	//najlepsza œcie¿ka
	std::vector <int>  best_route(N);
	//i jej koszt
	best_cost = INT_MAX;
	
	//1. Wygeneruj punkt startowy
	// ------1a Wygeneruj losowo lub heurystycznie punkt startowy x0 (rozwi¹zanie)
	for (int i = 0; i < N; i++)                     //permutacja[0] = 0 itd.
		permutation[i] = i;

	//------ 1b Przypisz to na razie jako najlepsze rozwi¹zanie
	best_route = permutation;

	//2. Ustaw temp pocz¹tkow¹ jako maksymaln¹ (bêdzie obni¿ana podczas trwania algoryytmu)
	int T = Tmax;

	//3. Pêtla, która siê wykonuje dopóki nie napotka warunku równowagi (np. temperatura minimalna)
	while(T>Tmin)
	{
		if (elapsed_time >= max_time_s * 1000000) {
			std::cout << "Warunek stopu: Przekroczono czas" << std::endl;
			break;
		}

		for (int i = 0; i < iterations; i++) //warunek koñca
		{
			

			//4. Wybranie losowo s¹siada
			previous_cost = calculate_cost_SA(permutation);

			new_permutation = generate_best_neighbour(permutation);

			new_cost = calculate_cost_SA(new_permutation);

			//5. Czy koszt s¹siada mniejszy ni¿ aktualnie najmniejszy? Jeœli tak zaktualizuj dane
			if (new_cost < best_cost)
			{
				//6. Aktualny s¹siad
				permutation = new_permutation;

				best_route = new_permutation;
				best_cost = new_cost;
			}
			//7. Wylosowanie wartoœci z przedzia³u [0,1) i sprawdzenie czy jest mniejsza ni¿ funkcja prawdopodobieñstwa (nowe rozw, stare rozw)
			else if ((float)rand() / (float)RAND_MAX < P(previous_cost, new_cost, T))
			{
				//8. Aktualny s¹siad
				permutation = new_permutation;					
			}					
		}
		//9. Obni¿ temperaturê funckja G
		T = alfa*T;

		//Podliczenie, ile czasu trwa³o
		elapsed_time = read_QPC() - start;
	}

	//Dodanie zera i wyœwietlenie wyników
	best_route.push_back(0);

	std::cout << "Minimalny koszt: " << best_cost << std::endl;
	for (unsigned i = 0; i < N + 1; i++) {
		std::cout << best_route[i];
		if (i != N)	std::cout << " -> ";
	}
	std::cout << std::endl;
}

float TSP_SA::P(int x0, int x, float Ti)
{
	
	return minimum(exp(-1 * ((x- x0)/Ti)),1);
}

long long int TSP_SA::read_QPC()
{
	LARGE_INTEGER count;
	DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(&count);
	SetThreadAffinityMask(GetCurrentThread(), oldmask);
	return((long long int)count.QuadPart);
}

TSP_SA::~TSP_SA()
{
	for (int i = 0; i < N; i++)
	{
		delete[] matrix_distance[i];
	}

	if (matrix_distance) delete[] matrix_distance;

}
