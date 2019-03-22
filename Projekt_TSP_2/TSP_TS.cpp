#include "TSP_TS.h"

TSP_TS::TSP_TS(int number_of_vertices, int ** A)
{
	N = number_of_vertices;
	
	//Kopiowanie macierzy A do macierzy matrix_distance
	// Wierszy tyle, ile wierzcholkow
	matrix_distance = new int *[N];

	// Kolumn tyle, ile wierzcho�k�w
	for (int i = 0; i < N; i++) matrix_distance[i] = new int[N];

	// Wczytanie macierzy, ale zast�pienie 0 na przek�tnej -1
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			matrix_distance[i][j] = A[i][j];
			if (i == j)	matrix_distance[i][j] = 0;
		}

	//TABU LIST
	// Wierszy tyle, ile wierzcholkow
	tabu_list = new int *[N];

	// Kolumn tyle, ile wierzcho�k�w
	for (int i = 0; i < N; i++) tabu_list[i] = new int[N];

	neighbour_criterion = 0; //domy�lne ustawienie 
}

TSP_TS::~TSP_TS()
{
	for (int i = 0; i < N; i++)
	{
		delete[] matrix_distance[i];
	}

	if (matrix_distance) delete[] matrix_distance;

	//Usuwamy macierz z pami�ci komputera
	for (int i = 0; i < N; i++)
	{
		delete[] tabu_list[i];
	}

	if (tabu_list) delete[] tabu_list;
}


void TSP_TS::solve(int iterations, int lifetime, int max_critical_events, double max_time_s, int neighbour_criterion)
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
	//start zliczania czasu
	double elapsed_time = 0;

	//zmienna iteracyjna do krytycznego warunku
	int number_of_the_same_order = 0;

	//aktualna sciezka
	std::vector <int>  permutation(N);             
	//i jej koszt
	int permutation_cost = INT_MAX;
	int previous_cost;

	//najlepsza �cie�ka
	std::vector <int>  best_route(N);
	//i jej koszt
	best_cost = INT_MAX;


	// ------1a Wygeneruj losowo lub heurystycznie punkt startowy x0 (rozwi�zanie)
	for (int i = 0; i < N; i++)                     //permutacja[0] = 0 itd.
		permutation[i] = i;

	//------ 1b Przypisz to na razie jako najlepsze rozwi�zanie
	best_route = permutation;
	best_cost = calculate_cost(permutation);

	// ------2 Tabu List pusta
	clear_tabu_list();

	//sprawdzenie kosztu aktualnej drogi
	permutation_cost = calculate_cost(permutation);

	//Start czas (do warunku stopu)
	start = read_QPC();

	// ------3 Petla trwa dopoki warunek konca = true (Np. przekroczenie okre�lonej liczby itaracji)
	for(int i=0; i<iterations; i++)	
	{
		//Przerwanie p�tli po wybranym czasie (nie mo�na ustawi� tego jako & w p�tli for, wi�c trzeba tutaj
		//jako ifa z breakiem
		if (elapsed_time >= max_time_s*1000000) {
			//std::cout << "Warunek stopu: Przekroczono czas" << std::endl;
			break;
		}

		//patrzymy jaki byl koszt przed przemieszaniem (wyborem s�siada)
		//previous_cost = calculate_cost(permutation);

		// ------4 Znajd� rozwi�zanie (zmiana element�w w �cie�ce), dla kt�rego funkcja celu jest nawi�ksza
		//(strategia wyboru rozwi�zania: przejrze� wszystkie, bra� pod uwag� list� Tabu i kryterium aspiracji). Albo metoda Aspiration Plus
		//Uwaga! Wyklucz te rozwi�zania, kt�re s� na Tabu List
		//Uwaga2! Kryterium aspiracji Aspiration(x,y) = TRUE
		if(neighbour_criterion) permutation = generate_best_neighbour(permutation, lifetime);
		else permutation = generate_best_neighbour_random(permutation, lifetime);

		// ------5 Najlepsze rozwi�zanie <- aktualne rozwi�zanie
		permutation_cost = calculate_cost(permutation);

		// ------6 Je�li koszt drogi jest mniejszy ni� aktualnie najlepszy, przypisz do najlepszego rozwi�zanie aktualne rozwi�zanie
		if (permutation_cost < best_cost)
		{
			best_cost = permutation_cost;
			best_route = permutation;
			number_of_the_same_order = 0;
		}
		else
		{
			number_of_the_same_order++;
		}
		
		// ------7 (Realizowany w punkcie 4, bo mo�e by� wykonywany wsp�bie�nie)
		//Dodaj nowe elementy do Tabu List (z okre�lon� kandecj�)

		// ------8 Aktualizacja Tabu List
		//Dla ka�dego rozwi�zania b�d�cego na Tabu List, obni� kadencj� o 1
		//Je�li kadencja = 0, usu� z Tabu List (w mojej implementacji zostaje po prostu zero
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				if (tabu_list[i][j] > 0)
					tabu_list[i][j] = tabu_list[i][j] - 1;
			}
		}

		// ------9 Je�li CriticalEvent = true (czyli zasz�y okre�lone warunki wymagaj�ce wygenerowania nowego startowego rozwi�zania
		// (Np. przez k kolejnych iteracji nie by�o lepszego rozwi�zania lub alg wygenerowa� k iteracji od wygenerowania ost rozw startowego)
		// -> to Restart (dywersyfikacja)
		//if (previous_cost > permutation_cost)
		//else number_of_the_same_order = 0;

		if (number_of_the_same_order == max_critical_events && (max_critical_events != 0))
		{
			//Wygenerowanie nowego rozwi�zania poprzez wywo�anie wielokrotne pojedynczego swapni�cia losowych miast
			for (int i = 0; i < 50; i++) random_swap(permutation);

			if (calculate_cost(permutation) < best_cost)
			{
				best_cost = permutation_cost;
				best_route = permutation;
				number_of_the_same_order = 0;
			}

			number_of_the_same_order = 0;
			// Wyczyszczenie TabuList
			clear_tabu_list();
		}

		//Je�li koszt aktualnej drogi mniejszy ni� dotychczas najlepszej, to przypisz do najlepszej trasy aktualn� drog�

		//Podliczenie, ile czasu trwa�o
		elapsed_time = read_QPC() - start;
		
		}

	//Dodanie zera i wy�wietlenie wynik�w
	best_route.push_back(0);

	std::cout << "Minimalny koszt: " << best_cost << std::endl;
	for (unsigned i = 0; i < N+1; i++) {
		std::cout << best_route[i];
		if (i != N)	std::cout << " -> ";
	}
	std::cout << std::endl;

}

int TSP_TS::calculate_cost(std::vector<int>& permutation)
{
	//pocz�tkowy koszt drogi
	int cost = 0;

	for (int i = 0; i < N - 1; i++)
	{
		//std::cout << permutation.at(i) << std::endl;
		//std::cout << permutation.at(i+1) << std::endl;
		cost += matrix_distance[permutation.at(i)][permutation.at(i+1)];
		//std::cout << cost << std::endl;
	}
	//dodajemy koszt trasy o koszt powrotu do wierzcho�ka 0

	cost += matrix_distance[permutation.at(N-1)][0];
	return cost;
}


void TSP_TS::random_swap(std::vector<int> &route)
{
	int random_city = rand() % (N - 1) + 1;		//generujemy dwie warto�ci r�ne od zera oraz od siebie
	int another_random_city = rand() % (N - 1) + 1;
	while (random_city == another_random_city) another_random_city = rand() % (N - 1) + 1;

	std::swap(route[random_city], route[another_random_city]);
}

//Generowanie s�siedztwa
//TODO mo�e zwr�ci� to samo rozwi�zanie, gdy tabulist puste
std::vector<int> TSP_TS::generate_best_neighbour(std::vector<int> permutation, int lifetime)
{
	//Wykonujemy t� operacj�, �eby w razie czego zwr�ci� jakiegokolwiek s�siada i nie utkn�� w martwym punkcie
	//int random_city = rand() % (N - 1) + 1;		//generujemy dwie warto�ci r�ne od zera oraz od siebie
	//int another_random_city = rand() % (N - 1) + 1;
	//while (random_city == another_random_city) another_random_city = rand() % (N - 1) + 1;

	//std::swap(permutation[random_city], permutation[another_random_city]);

	//Przypisz najpierw jakiegokolwiek s�siada (potem si� polepszy)
	std::vector<int> best_neighbour = permutation;

	//Aktualne (i p�niej ko�cowe) miasta s�siada, kt�re zosta�y swapni�te
	int best_city_x, best_city_y;

	int best_cost_neighbours = INT_MAX;

	int cost_current;

	//Zaczynamy od 1, bo zero musi by� zawsze na pocz�tku
	for (int i = 1; i < N; i++)
	{
		for (int j = 2; j < N; j++)
		{
			//s�siadem nie jest to samo rozwi�zanie, a stanie si� tak, gdy i == j
			//if (i == j) continue;						

			//Wykonaj tylko dla tych kraw�dzi, kt�re nie s� na zakazanej li�cie
			if (tabu_list[i][j] == 0)
			{			
				std::vector<int> new_perm = permutation;
				std::swap(new_perm[i], new_perm[j]);
				cost_current = calculate_cost(new_perm);

				if (cost_current < best_cost_neighbours)
				{
					//Tu teoretycznie mo�na juz zrealizowac punkt 5, ale dla czytelno�ci zostawiamy w funkcji g��wnej solve
					best_cost_neighbours = cost_current;
					best_neighbour = new_perm;
					best_city_x = i;
					best_city_y = j;
				}

			}			
		}
	}
	
	//Ad. ------ 7 Ustawienie kadencji
	tabu_list[best_city_x][best_city_y] = lifetime;

	return best_neighbour;
}

//Druga funkcja generuj�ca najlepszego s�siada, ale zawsze zwr�ci jakie� przemieszanie
std::vector<int> TSP_TS::generate_best_neighbour_random(std::vector<int> permutation, int lifetime)
{
	//Aktualne (i p�niej ko�cowe) miasta s�siada, kt�re zosta�y swapni�te
	int best_city_x = 0, best_city_y = 0;

	std::vector<int> next_neighbour = permutation;


	//Zaczynamy od 1, bo zero musi by� zawsze na pocz�tku
	for (int i = 1; i < N; i++)
	{
		//dwie losowe warto�ci r�ne od zera
		int random_city = rand() % (N - 1) + 1;
		int another_random_city = rand() % (N - 1) + 1;

		while (random_city == another_random_city)
		{
			another_random_city = rand() % (N - 1) + 1;
		}

		std::vector<int> new_neighbour = next_neighbour;

		// Zamie� miasta
		std::swap(new_neighbour[random_city], new_neighbour[another_random_city]);

		//int cost = calculate_cost(permutation);
			

		//aktualy koszt
		//int new_cost = calculate_cost(best_neighbour);

			//Wykonaj tylko dla tych kraw�dzi, kt�re nie s� na zakazanej li�cie
			if (tabu_list[random_city][another_random_city] == 0)
			{		
				if (calculate_cost(new_neighbour) < calculate_cost(next_neighbour))
				{
					next_neighbour = new_neighbour;
					best_city_x = random_city;
					best_city_y = another_random_city;
				}
			}
		
	}

	//Ad. ------ 7 Ustawienie kadencji
	tabu_list[best_city_x][best_city_y] = lifetime;

	return next_neighbour;
}

long long int TSP_TS::read_QPC()
{
	LARGE_INTEGER count;
	DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(&count);
	SetThreadAffinityMask(GetCurrentThread(), oldmask);
	return((long long int)count.QuadPart);
}


void TSP_TS::clear_tabu_list() {
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			tabu_list[i][j] = 0;
}


