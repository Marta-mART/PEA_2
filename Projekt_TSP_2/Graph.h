#pragma once
#pragma once
#include <iostream>
#include <iomanip> // setw
class Graph
{
public:
	int **A;			// macierz s¹siedztwa
	int vertices_num;	// liczba wierzcho³ków

	void display_graph();	//wyswietlenie grafu
	Graph(std::string file_name);
	Graph(int vN, int wageMin, int wageMax);
	~Graph();
	int ** get_matrix() { return A; }
	int getVertices() { return vertices_num; }
};

