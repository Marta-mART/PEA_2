#include <iostream>
#include <string>
#include <fstream>
#include <chrono> // czas
#include <iomanip>
#include <fstream>
#include <windows.h>
#include "Graph.h"
#include "TSP_TS.h"
#include "TSP_SA.h"

//-------------------------------------------------------
// PEA Marta G³owacka 8.11.2018
//-------------------------------------------------------

using namespace std;

//Zmienne globalne do pomiaru czasu
long long int frequency, start, elapsed;

long long int read_QPC();

void display_time();
std::string get_data();

void load_to_file_times(std::string file_name, double time);
void load_to_file_costs(std::string file_name, int cost);

//--------------------------TS---------------------------
double time_criterion_TS = 30; //[s]
int iteration_criterion_TS = 10000;
int attempts_until_diversification = 1000;
int lifetime = 10;
bool neighour_selection = 0;

void give_stop_criterion_TS();
void give_neighbour_criterion();
void give_lifetime_criterion();
void give_divers_criterion();
void display_settings_TS();
void test_TS();

//--------------------------SA---------------------------
float Tmin = 0.0001;
float Tmax = 100000;
float Tcooling = 0.9999;
int iteriation_criterion_SA = 70;
double time_criterion_SA = 10; //[s]

void display_settings_SA();

void get_data_temp();
void test_SA();


int main()
{
	//menu
	srand(time(0));
	int choice;

	do
	{
		start = 0, elapsed = 0;
		system("cls");

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);

		std::cout << std::endl;
		std::cout << "----------------" << std::endl;
		std::cout << "PROBLEM KOMIWOJAZERA - PROJEKT 2" << std::endl;
		std::cout << "----------------" << std::endl;

		// Zmiana koloru czcionki
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

		// Wyswietlenie opcji - wybór struktury
		std::cout << std::endl << "MENU: \n";
		std::cout << "-------------------------------\n";
		std::cout << "1. Wczytanie danych z pliku \n\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
		std::cout << "TABU SEARCH \n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		std::cout << "2. Wprowadzenie kryterium stopu \n";
		std::cout << "3. Wlaczenie/Wylaczenie dywersyfikacji \n";
		std::cout << "4. Wybor sasiedztwa \n";
		std::cout << "5. Wprowadz kadencje \n";
		std::cout << "6. Uruchomienie algorytmu Tabu Search \n\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
		std::cout << "SIMULATED ANNEALING \n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		std::cout << "7. Wprowadzenie ustawien \n";
		std::cout << "8. Uruchomienie Simulated annealing \n\n";
		std::cout << "9. Wyjscie \n";
		std::cout << "-------------------------------\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
		cin >> choice;
				
		Graph myGraph("tsp_17.txt");
		
		switch (choice)
		{
		case 1:
		{
			myGraph.display_graph();
		}
		break;
		case 2:
		{
			give_stop_criterion_TS();
		}
		break;
		case 3:
		{
			give_divers_criterion();
		}
		break;
		case 4:
		{
			give_neighbour_criterion();
		}
		break;
		case 5:
		{
			give_lifetime_criterion();
		}
		break;
		case 6:
		{
			display_settings_TS();
			TSP_TS algorytm_TSP_TS(myGraph.getVertices(), myGraph.get_matrix());
			start = read_QPC();
			algorytm_TSP_TS.solve(iteration_criterion_TS, lifetime, attempts_until_diversification, time_criterion_TS, neighour_selection);
			elapsed = read_QPC() - start;

			display_time();
		}
		break;
		case 7:
		{
			get_data_temp();
		}
		break;
		case 8:
		{
			display_settings_SA();
			TSP_SA algorytm_TSP_SA(myGraph.getVertices(), myGraph.get_matrix());
			start = read_QPC();
			algorytm_TSP_SA.solve(Tmax, Tmin, Tcooling, iteriation_criterion_SA,time_criterion_SA);
			elapsed = read_QPC() - start;

			display_time();
		}
		break;

		}

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
		std::cout << "[Wcisnij ENTER, aby wybrac kolejna opcje]" << std::endl;

		cin.get();
		cin.get();
		

	} while (choice != 9);

}

std::string get_data()
{
		std::string path;
		std::cout << "Podaj sciezke pliku do wczytania: " << std::endl;
		std::cin >> path;
		return path;
}

//-------------------------------------------------------
//Funkcja do odczytu minionego czasu
//-------------------------------------------------------
long long int read_QPC()
{
	LARGE_INTEGER count;
	DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(&count);
	SetThreadAffinityMask(GetCurrentThread(), oldmask);
	return((long long int)count.QuadPart);

}




void load_to_file_times(std::string file_name, double time)
{
	// Zmienna do wpisywania w pliku
	std::ofstream plik;

	// Otwieranie pliku
	plik.open(file_name.c_str(), std::ios::app);

	// Gdy blad otwarcia pliku
	if (plik.bad() || plik.fail())
	{
		cout << "Nie udalo sie otworzyc pliku.";
	}

	plik << time << endl;

}

void load_to_file_costs(std::string file_name, int cost)
{
	// Zmienna do wpisywania w pliku
	std::ofstream plik;

	// Otwieranie pliku
	plik.open(file_name.c_str(), std::ios::app);

	// Gdy blad otwarcia pliku
	if (plik.bad() || plik.fail())
	{
		cout << "Nie udalo sie otworzyc pliku.";
	}

	plik << cost << endl;

}

void display_time()
{
	// Zmiana koloru czcionki
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
	std::cout << "Czas wykonania: " << elapsed << " us" << std::endl;
}

void give_stop_criterion_TS()
{
	
		cout << "Podaj czas w sekundach		" << std::endl;
		cin >> time_criterion_TS;

		cout << "Podaj liczbe maksymalnych iteracji		" << std::endl;
		cin >> iteration_criterion_TS;

}

void give_divers_criterion()
{
	cout << "Ile max prob przed dywersyfikacja?		" << endl;
	cin >> attempts_until_diversification;

}

void give_lifetime_criterion()
{
	cout << "Kandecja (0) Losowo (1) Wprowadz dlugosc kadencji" << endl;
	int choice = 1;
	cin >> choice;
	switch (choice)
	{
	case 0:
		lifetime = rand() % 100 + 1;
		break;
	case 1:
		cout << "Wprowadz dlugosc kadencji:		" << endl;
		cin >> lifetime;
		break;
	default:
		break;
	}
}

void give_neighbour_criterion()
{
	cout << "Wybor sasiedztwa (0) Losowo (1) Najlepsze" << endl;
	cin >> neighour_selection;
}

void display_settings_TS() 
{
	cout << setw(27) << "Kryterium stopu :" << setw(15) << time_criterion_TS << " s " << setw(7) << iteration_criterion_TS << " iteracji" << endl;
	cout << setw(27) << "Kryterium dywersyfikacji :" << setw(15) <<  attempts_until_diversification << " prob przed dywersyfikacja" << endl;
	
	cout << setw(27) << "Sasiedztwo :";
	if (neighour_selection == 0) cout << setw(15) << "losowo" << endl;
	else cout << setw(15) << "najlepszy sasiad" << endl;

	cout << setw(27) << "Kandencja :" << setw(15) << lifetime << endl << endl;
}

void test_TS()
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

	std::string tab_tsp[3] = { "data17.txt", "data56.txt", "data171.txt" };
	const int number_of_tests = 50;

	double criteria[5] = {INT_MAX,//iterations,
		30, // lifetime,
		100, // max_critical_events,
		0, // max_time,
		1}; // neighbourhood browsing type choice
	
		std::cout << "-------------------------------------------------" << std::endl;
		std::cout << "Instancja : " << tab_tsp[2];

		Graph test_graph(tab_tsp[0]);
		TSP_TS tsp_ts(test_graph.getVertices(), test_graph.get_matrix());

		//testy dla ró¿nych rozmiarów iteracji = czas siê wyd³u¿a
		for (int j = 0; j < number_of_tests; j++)
		{
			criteria[3] = (0.01f *j);
			 //---Pomiar czasu
			start = read_QPC();
			tsp_ts.solve(criteria[0], criteria[1], criteria[2], criteria[3], criteria[4]);
			elapsed = read_QPC() - start;
			//---Koniec pomiaru czasu

			load_to_file_times("wyniki_pomiarow_TS_times.txt", criteria[3]);
			load_to_file_costs("wyniki_pomiarow_TS_costs.txt", tsp_ts.get_cost());
			
			//criteria[0] += (10);
		}	


}

void display_settings_SA()
{
	cout << setw(27) << "Temperatura poczatkowa :" << setw(15) << Tmax << endl;
	cout << setw(27) << "Temperatura chlodzenia :" << setw(15) << Tcooling << endl;
	cout << setw(27) << "Temperatura koncowa :" << setw(15) << Tmin << endl << endl;
	cout << setw(27) << "Liczba iteracji :" << setw(15) << iteriation_criterion_SA<< endl << endl;
}

void get_data_temp()
{
	cout << "Temperatura poczatkowa		" << endl;
	cin >> Tmax;
	cout << "Temperatura koncowa		" << endl;
	cin >> Tmin;
	cout << "Temperatura chlodzenia		" << endl;
	cin >> Tcooling;
	cout << "Liczba iteracji	" << endl;
	cin>> iteriation_criterion_SA;
	cout << "Max czas	" << endl;
	cin >> time_criterion_SA;
}

void test_SA()
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

	std::string tab_tsp[3] = { "data17.txt", "data56.txt", "data171.txt" };
	const int number_of_tests = 200;

	double criteria[5] = { 
		100000, // start
		0.0001, // min
		0.9999, // cooling
		100,//iterations,
		1 }; // time

	std::cout << "-------------------------------------------------" << std::endl;
	std::cout << "Instancja : " << tab_tsp[0];

	Graph test_graph(tab_tsp[0]);
	TSP_SA tsp_sa(test_graph.getVertices(), test_graph.get_matrix());

	//testy dla ró¿nych rozmiarów iteracji = czas siê wyd³u¿a
	for (int j = 0; j < number_of_tests; j++)
	{
		criteria[4] += (1.0f *j);
		criteria[3] += 1;
		//---Pomiar czasu
		start = read_QPC();
		tsp_sa.solve(criteria[0], criteria[1], criteria[2], criteria[3], criteria[4]);
		elapsed = read_QPC() - start;
		//---Koniec pomiaru czasu

		load_to_file_times("wyniki_pomiarow_SA_times.txt", elapsed/1000000.0f);
		load_to_file_costs("wyniki_pomiarow_SA_costs.txt", tsp_sa.get_cost());

	}

}