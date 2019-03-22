#include "TSP_TS.h"

TSP_TS::TSP_TS(int number_of_vertices, int ** A)
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

	//TABU LIST
	// Wierszy tyle, ile wierzcholkow
	tabu_list = new int *[N];

	// Kolumn tyle, ile wierzcho³ków
	for (int i = 0; i < N; i++) tabu_list[i] = new int[N];

	neighbour_criterion = 0; //domyœlne ustawienie 
}

TSP_TS::~TSP_TS()
{
	for (int i = 0; i < N; i++)
	{
		delete[] matrix_distance[i];
	}

	if (matrix_distance) delete[] matrix_distance;

	//Usuwamy macierz z pamiêci komputera
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

	//najlepsza œcie¿ka
	std::vector <int>  best_route(N);
	//i jej koszt
	best_cost = INT_MAX;


	// ------1a Wygeneruj losowo lub heurystycznie punkt startowy x0 (rozwi¹zanie)
	for (int i = 0; i < N; i++)                     //permutacja[0] = 0 itd.
		permutation[i] = i;

	//------ 1b Przypisz to na razie jako najlepsze rozwi¹zanie
	best_route = permutation;
	best_cost = calculate_cost(permutation);

	// ------2 Tabu List pusta
	clear_tabu_list();

	//sprawdzenie kosztu aktualnej drogi
	permutation_cost = calculate_cost(permutation);

	//Start czas (do warunku stopu)
	start = read_QPC();

	// ------3 Petla trwa dopoki warunek konca = true (Np. przekroczenie okreœlonej liczby itaracji)
	for(int i=0; i<iterations; i++)	
	{
		//Przerwanie pêtli po wybranym czasie (nie mo¿na ustawiæ tego jako & w pêtli for, wiêc trzeba tutaj
		//jako ifa z breakiem
		if (elapsed_time >= max_time_s*1000000) {
			//std::cout << "Warunek stopu: Przekroczono czas" << std::endl;
			break;
		}

		//patrzymy jaki byl koszt przed przemieszaniem (wyborem s¹siada)
		//previous_cost = calculate_cost(permutation);

		// ------4 ZnajdŸ rozwi¹zanie (zmiana elementów w œcie¿ce), dla którego funkcja celu jest nawiêksza
		//(strategia wyboru rozwi¹zania: przejrzeæ wszystkie, braæ pod uwagê listê Tabu i kryterium aspiracji). Albo metoda Aspiration Plus
		//Uwaga! Wyklucz te rozwi¹zania, które s¹ na Tabu List
		//Uwaga2! Kryterium aspiracji Aspiration(x,y) = TRUE
		if(neighbour_criterion) permutation = generate_best_neighbour(permutation, lifetime);
		else permutation = generate_best_neighbour_random(permutation, lifetime);

		// ------5 Najlepsze rozwi¹zanie <- aktualne rozwi¹zanie
		permutation_cost = calculate_cost(permutation);

		// ------6 Jeœli koszt drogi jest mniejszy ni¿ aktualnie najlepszy, przypisz do najlepszego rozwi¹zanie aktualne rozwi¹zanie
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
		
		// ------7 (Realizowany w punkcie 4, bo mo¿e byæ wykonywany wspó³bie¿nie)
		//Dodaj nowe elementy do Tabu List (z okreœlon¹ kandecj¹)

		// ------8 Aktualizacja Tabu List
		//Dla ka¿dego rozwi¹zania bêd¹cego na Tabu List, obni¿ kadencjê o 1
		//Jeœli kadencja = 0, usuñ z Tabu List (w mojej implementacji zostaje po prostu zero
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				if (tabu_list[i][j] > 0)
					tabu_list[i][j] = tabu_list[i][j] - 1;
			}
		}

		// ------9 Jeœli CriticalEvent = true (czyli zasz³y okreœlone warunki wymagaj¹ce wygenerowania nowego startowego rozwi¹zania
		// (Np. przez k kolejnych iteracji nie by³o lepszego rozwi¹zania lub alg wygenerowa³ k iteracji od wygenerowania ost rozw startowego)
		// -> to Restart (dywersyfikacja)
		//if (previous_cost > permutation_cost)
		//else number_of_the_same_order = 0;

		if (number_of_the_same_order == max_critical_events && (max_critical_events != 0))
		{
			//Wygenerowanie nowego rozwi¹zania poprzez wywo³anie wielokrotne pojedynczego swapniêcia losowych miast
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

		//Jeœli koszt aktualnej drogi mniejszy ni¿ dotychczas najlepszej, to przypisz do najlepszej trasy aktualn¹ drogê

		//Podliczenie, ile czasu trwa³o
		elapsed_time = read_QPC() - start;
		
		}

	//Dodanie zera i wyœwietlenie wyników
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
	//pocz¹tkowy koszt drogi
	int cost = 0;

	for (int i = 0; i < N - 1; i++)
	{
		//std::cout << permutation.at(i) << std::endl;
		//std::cout << permutation.at(i+1) << std::endl;
		cost += matrix_distance[permutation.at(i)][permutation.at(i+1)];
		//std::cout << cost << std::endl;
	}
	//dodajemy koszt trasy o koszt powrotu do wierzcho³ka 0

	cost += matrix_distance[permutation.at(N-1)][0];
	return cost;
}


void TSP_TS::random_swap(std::vector<int> &route)
{
	int random_city = rand() % (N - 1) + 1;		//generujemy dwie wartoœci ró¿ne od zera oraz od siebie
	int another_random_city = rand() % (N - 1) + 1;
	while (random_city == another_random_city) another_random_city = rand() % (N - 1) + 1;

	std::swap(route[random_city], route[another_random_city]);
}

//Generowanie s¹siedztwa
//TODO mo¿e zwróciæ to samo rozwi¹zanie, gdy tabulist puste
std::vector<int> TSP_TS::generate_best_neighbour(std::vector<int> permutation, int lifetime)
{
	//Wykonujemy tê operacjê, ¿eby w razie czego zwróciæ jakiegokolwiek s¹siada i nie utkn¹æ w martwym punkcie
	//int random_city = rand() % (N - 1) + 1;		//generujemy dwie wartoœci ró¿ne od zera oraz od siebie
	//int another_random_city = rand() % (N - 1) + 1;
	//while (random_city == another_random_city) another_random_city = rand() % (N - 1) + 1;

	//std::swap(permutation[random_city], permutation[another_random_city]);

	//Przypisz najpierw jakiegokolwiek s¹siada (potem siê polepszy)
	std::vector<int> best_neighbour = permutation;

	//Aktualne (i póŸniej koñcowe) miasta s¹siada, które zosta³y swapniête
	int best_city_x, best_city_y;

	int best_cost_neighbours = INT_MAX;

	int cost_current;

	//Zaczynamy od 1, bo zero musi byæ zawsze na pocz¹tku
	for (int i = 1; i < N; i++)
	{
		for (int j = 2; j < N; j++)
		{
			//s¹siadem nie jest to samo rozwi¹zanie, a stanie siê tak, gdy i == j
			//if (i == j) continue;						

			//Wykonaj tylko dla tych krawêdzi, które nie s¹ na zakazanej liœcie
			if (tabu_list[i][j] == 0)
			{			
				std::vector<int> new_perm = permutation;
				std::swap(new_perm[i], new_perm[j]);
				cost_current = calculate_cost(new_perm);

				if (cost_current < best_cost_neighbours)
				{
					//Tu teoretycznie mo¿na juz zrealizowac punkt 5, ale dla czytelnoœci zostawiamy w funkcji g³ównej solve
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

//Druga funkcja generuj¹ca najlepszego s¹siada, ale zawsze zwróci jakieœ przemieszanie
std::vector<int> TSP_TS::generate_best_neighbour_random(std::vector<int> permutation, int lifetime)
{
	//Aktualne (i póŸniej koñcowe) miasta s¹siada, które zosta³y swapniête
	int best_city_x = 0, best_city_y = 0;

	std::vector<int> next_neighbour = permutation;


	//Zaczynamy od 1, bo zero musi byæ zawsze na pocz¹tku
	for (int i = 1; i < N; i++)
	{
		//dwie losowe wartoœci ró¿ne od zera
		int random_city = rand() % (N - 1) + 1;
		int another_random_city = rand() % (N - 1) + 1;

		while (random_city == another_random_city)
		{
			another_random_city = rand() % (N - 1) + 1;
		}

		std::vector<int> new_neighbour = next_neighbour;

		// Zamieñ miasta
		std::swap(new_neighbour[random_city], new_neighbour[another_random_city]);

		//int cost = calculate_cost(permutation);
			

		//aktualy koszt
		//int new_cost = calculate_cost(best_neighbour);

			//Wykonaj tylko dla tych krawêdzi, które nie s¹ na zakazanej liœcie
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


