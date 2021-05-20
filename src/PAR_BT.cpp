/*
 *
 *  Created on: 20 mar. 2021
 *      Author: Ruben Girela Castellón
 */
#include <iostream>
#include <armadillo>// para la matriz
#include <fstream> //lectura de ficheros
#include <stdlib.h>
#include <string.h>
#include <math.h> // para calculos matematicos (sqrt, pow (x^y), ...)
#include <algorithm>// para barajar el vector
#include <ctime>// libreria de time para el shuffle aleatorio de vectores
#include <stack>// use stack

#include "../include/PAR_BT.h"
#include "../include/random.h"
using namespace std;
using namespace arma;

PARBT::PARBT(string fichero_set, string fichero_set_const, int semilla){

	//creo el vector de atributos y la matriz de restricciones
	lectura(fichero_set, fichero_set_const);

	//en función del numero de atributos que tenga el numero de clusters sera 16 o 7
	switch(atributos[0].size()){
	case 5:
		k=16;
		break;
	default:
		k=7;
		break;
	}

	centroides.resize(k);//reservo el tamaño del vector de tamaño k

	Set_random(semilla);//creo una semilla para los valores aleatorios

	//Y genero aleatoriamente los centroides inicialmente distintos de dimensión n
	for(int i=0; i<k; ++i){
		for(unsigned int e=0; e <atributos[0].size(); ++e)
			centroides[i].push_back(Rand());
	}

	//insert all indices of atributos
	for(unsigned int i=0; i<atributos.size(); ++i){
		RSI.push_back(i);
	}

	//despues barajo los indices de los atributos
	srand(unsigned (semilla));//genero una semilla fija
	random_shuffle(RSI.begin(), RSI.end());//barajo el vector
}

void PARBT::shuffleRSI(){
	random_shuffle(RSI.begin(), RSI.end());//barajo el vector
}

void PARBT::lectura(string fichero_set, string fichero_set_const){
	//leo el fichero *_set.dat
	ifstream read(fichero_set);

	//si no lo encuentra o no puede habrirlo termina el programa
	if (!read){
		cout<<"No puedo abrir "<<fichero_set<<endl;
		exit(1);
	}

	//lee el contenido del archivo
	string cad="", sub_cad="";
	vector<float> fila;
	int size=0;//guarda el tamaño de la matriz
	while(!read.eof()) {
		++size;
		read >> cad;//leo por filas
		//recorro la fila caracter por caracter
		for(unsigned int i=0; i<cad.size(); ++i){
			if(cad.at(i)!=','){//si no encuentra una coma guarda el numero
				sub_cad+=cad.at(i);
			}else{//si encuentra una coma es que ya tiene el numero entero
				fila.push_back(stof(sub_cad));//lo añado al vector auxiliar
				sub_cad="";//y reseteo la sub cadena
			}
		}
		//añado el ultimo numero que no tiene delimitador a la derecha
		fila.push_back(stof(sub_cad));
		sub_cad="";//reseteo la subcadena

		//añado el vector de distancias de la fila leida
		atributos.push_back(fila);
		fila.clear();//limpio el vector
	}
	read.close();
	//borro el ultimo vector, ya que es la del salto de linea de la ultima linea, y contiene los mismos valores que la anterior
	atributos.pop_back();

	--size;//le decremento 1 porque cuenta el salto de linea del vacio

	matriz.set_size(size,size);//redimensiono la matriz
	size_mat = size;

	read.open(fichero_set_const);//abro el archivo que contiene la matriz

	//si no lo encuentra o no puede habrirlo termina el programa
	if (!read){
		cout<<"No puedo abrir "<<fichero_set_const<<endl;
		exit(1);
	}

	//lee el contenido del archivo *_set_const
	cad="";
	string num="";
	int col=0, row=0, col2=1;
	while(!read.eof()) {
		read >> cad;

		if(row<size){//esto lo hago porque lee el salto de linea de la ultima linea
			//recorro la linea
			for(unsigned int i=0; i<cad.size(); i+=2){
				//guardo el numero
				num = cad.at(i);
				if(num=="-"){//si es negativo guardo el - y el numero
					++i;
					num += cad.at(i);
				}
				//solo guardo la diagonal
				if(col>=col2){
					//guardo el valor en la matriz
					matriz(row,col)=stod(num);

					if(col != row and matriz(row,col) != 0){
						if(matriz(row,col) >0)
							ML.push_back(pair<int,int>(row,col));
						if(matriz(row,col) < 0)
							CL.push_back(pair<int,int>(row,col));
					}
				}

				++col;
			}
			//incremento de la fila e inicialización de la columna
			++row;
			col=0;
			col2=row+1;
		}

	}
	read.close();
}

//reset Centroides
void PARBT::resetCentroides(){
	//vuelvo a generar aleatoriamente los centroides inicialmente distintos de dimensión n
	for(int i=0; i<k; ++i){
		centroides[i].clear();
		for(unsigned int e=0; e <atributos[0].size(); ++e)
			centroides[i].push_back(Rand());
	}
}

//imprime los atributos de cada nodo
void PARBT::printDistanciasEuclideas(){
	for(vector<vector<float>>::iterator it=atributos.begin(); it != atributos.end(); ++it){
		for(vector<float>::iterator it2=(*it).begin(); it2 != (*it).end(); ++it2){
			if(it2+1 != (*it).end())
				cout << (*it2) << ", ";
			else
				cout << (*it2);
		}
		cout << endl;
	}
}
//imprime los centroides de cada cluster
void PARBT::printCentroides(){
	int i=0;
	for(vector<vector<float>>::iterator it=centroides.begin(); it != centroides.end(); ++it){
		cout << i << " [ ";
		for(vector<float>::iterator it2=(*it).begin(); it2 != (*it).end(); ++it2){
			if(it2+1 != (*it).end())
				cout << (*it2) << ", ";
			else
				cout << (*it2);
		}
		cout << " ]" << endl;
		++i;
	}
}

//print the indexes of the attributes
void PARBT::printRSI(){
	for(vector<int>::iterator it=RSI.begin(); it != RSI.end(); ++it){
		if(it+1 != RSI.end())
			cout << (*it) << ", ";
		else
			cout << (*it) << endl;
	}
}


//calculate the closest and least restriction to cluster
int PARBT::minRestrictionsDistance(int actual, bool first){
	int cluster=-1;
	float min_distance=999;//save the minimum distance and less restriction
	int less_restriction=999;
	float actual_distance=0;//save the actual distance
	int actual_restriction=0;//save the actual number of restrictions

	//go through all clusters
	for(unsigned int i=0; i < centroides.size(); ++i){

		//calculate the Euclidea distance with the current cluster
		actual_distance = distanciaEuclidea(atributos[actual],centroides[i]);

		//if it isn't the first node to enter
		if(!first)
			//calculates the number of constraints it violates
			actual_restriction = infeasibility(i, actual);

		//if the current cluster is less than the minimum saved, update the cluster and distance
		if(actual_restriction <=less_restriction){

			if((actual_restriction <less_restriction) || (actual_distance < min_distance && actual_restriction <=less_restriction)){

				min_distance = actual_distance;
				less_restriction = actual_restriction;
				cluster = i;
			}
		}
	}
	return cluster;
}

//calculate infeasibility when assigning an atribute to each cluster and return the minimum
int PARBT::infeasibility(int clust, int actual){
	// number of restrictions, matrix column and row and not empty cluster indexes
	int rest=0;
	//walk through each constrain ML
	for (unsigned int i = 0; i<ML.size(); ++i) {
		if(i < ML.size()){
			//if it find the current node in the constrain
			if(ML[i].first == actual){
				//and the second node has an assigned cluster and isn't in the same cluster
				if(S[ML[i].second] != -1 && S[ML[i].second] != clust){
					++rest;//increases the number of restrictions violated
				}
			}else if(ML[i].second == actual){
				if(S[ML[i].first] != -1 && S[ML[i].first] != clust){
					++rest;
				}
			}
		}
	}
	//walk through each constrain CL
	for (unsigned int i = 0; i<CL.size(); ++i) {
		//if it find the current node in the constrain
		if(CL[i].first == actual){
			//and the second node has an assigned cluster and is in the same cluster
			if(S[CL[i].second] != -1 && S[CL[i].second] == clust){
				++rest;//increases the number of restrictions violated
			}
		}else if(CL[i].second == actual){
			if(S[CL[i].first] != -1 && S[CL[i].first] == clust){
				++rest;
			}
		}
	}

	return rest;// return the number of restrictions it violates
}

//same the infeasibility(int clust, int actual) except it receives the solution
int PARBT::infeasibility(vector<int> S_cop){
	int restrictions = 0;
	//CALCULATE the max size
	unsigned int max = CL.size();
	if(ML.size()>max) max = ML.size();

	//walk through all restrictions
	for(unsigned int i = 0; i< max; ++i){
		if(i<CL.size()){
			//check if it violate the CL constraint
			if(S_cop[CL[i].first] == S_cop[CL[i].second])
				++restrictions;
		}

		if(i<ML.size()){
			//check if it violate the ML constraint
			if(S_cop[ML[i].first] != S_cop[ML[i].second])
				++restrictions;
		}
	}

	return restrictions;
}

//calcula la distancia euclidea entre 2 nodos
float PARBT::distanciaEuclidea(vector<float> nod1, vector<float> nod2){
	//la formula es: sqrt(sumatoria((a_i - b_i)²))
	float suma=0;

	//sumatoria((a_i - b_i)²)
	for(unsigned int i=0; i<nod1.size();++i){
		suma += pow((nod1[i]-nod2[i]),2);
	}
	//sqrt(sumatoria)
	return sqrt(suma);
}

//random assignment of each node with a cluster
vector<vector<int>> PARBT::randomAssign(int n){
	bool check = true;
	vector<int> new_S;
	vector<int> clusters_selected;
	int cluster_null = -1;
	clusters_selected.resize(k);
	new_S.resize(RSI.size());

	vector<vector<int>> solutions;

	for(int j=0; j<n; ++j){

		for(unsigned int l=0; l <clusters_selected.size(); ++l)
			clusters_selected[l] = 0;

		//go through all nodes
		for(unsigned int i = 0; i < RSI.size(); ++i){
			//if it has traversed half the nodes
			if(i > RSI.size()/2 && check){
				//check that no cluster is empty
				for(unsigned int e=0; e<clusters_selected.size(); ++e){
					//if the cluster is empty
					if(clusters_selected[i] == 0){//add the cluster to the stack
						cluster_null = e;
						break;
					}
				}
				if(cluster_null != -1){
					new_S[RSI[i]] = cluster_null;
					clusters_selected[cluster_null] = 1;
					cluster_null = -1;
				}else{
					check = false;
					new_S[RSI[i]]=rand() % k + 0;//randomly assign a cluster
				}
			}else{
				new_S[RSI[i]] = rand() % k + 0;//randomly assign a cluster
				if(clusters_selected[new_S[RSI[i]]] == 0)
					clusters_selected[new_S[RSI[i]]] = 1;

			}
		}
		solutions.push_back(new_S);
	}
	return solutions;
}

vector<int> PARBT::BMB(int max_iter){
	vector<int> best_solution;
	vector<vector<int>> soluciones = randomAssign(10);
	int it=0;

	for(unsigned int i = 0; i < soluciones.size(); ++i){
		soluciones[i] = algoritmoBL(soluciones[i], max_iter, it);
	}

	return best_solution;
}

vector<int> PARBT::algoritmoBL(vector<int> S_cop, int max_iter, int &iterations){

	vector<int> S_cop2 = S_cop;
	int infease = infeasibility(S_cop);//calculate infeasibility
	float gen_deviation = generalDeviation(S_cop); //calculate General Deviation
	float landa = createLanda();//calculate landa
	//and calculate the fitness
	float f = gen_deviation + (infease * landa);

	vector<pair<int,int>> vecindario = generateNeig(S_cop);//create the neighborhood
	random_shuffle(vecindario.begin(), vecindario.end());//shuffle vector

	bool end = false;//algorithm completion
	int iterate = 0;

	do{
		//if the number of iterate don't reach 100000
		if(iterate < max_iter){//calculate the new solution
			S_cop2 = betterFitness(S_cop, vecindario, f, landa, iterate, max_iter);
			//update centroides
			centroides = updateDistance(S_cop);

			//calculate the new neighborhood
			random_shuffle(vecindario.begin(), vecindario.end());//shuffle vector
		}else
			end = true;//else the algorithm ends

	}while(!end or iterations < max_iter);
	//cout << iterate << endl;
	return S_cop2;
}

//Update the distance
vector<vector<float>> PARBT::updateDistance(const vector<int> &nodes){
	//save the actual distance
	vector<float> distance(atributos[nodes[0]].size(),0);
	vector<vector<float>> centroides;
	centroides.resize(k);

	int size = 0;
	for(int j = 0; j<k; ++j){
		for(unsigned int e=0; e<nodes.size(); ++e){
			if(nodes[e] == j){
				++size;
				//calculate average distance
				for(unsigned int i=0; i < atributos[e].size(); ++i){
					//sumatorry
					distance[i] += atributos[e][i];
				}
			}
		}

		//average
		for(unsigned int i = 0; i < distance.size(); ++i){
			distance[i] = distance[i]/nodes.size();
		}
		centroides[j] = distance;
	}


	return centroides;
}

//genereate the possible neighborhoods
vector<pair<int,int>> PARBT::generateNeig(vector<int> S_cop){
	vector<pair<int,int>> vecindario;
	for(unsigned int i=0; i<RSI.size(); ++i){
		//if the clusters is not equal to actual add
		for(int e = 0; e<k; ++e){
			vecindario.push_back(pair<int, int>(RSI[i],e));
		}
	}
	return vecindario;
}

//calculates the new better solution than the current solution
vector<int> PARBT::betterFitness(const vector<int> &S_cop, const vector<pair<int,int>> &vecindario, float &f, float landa, int &it, int max){

	int infease = 0;//calculate infeasibility
	float gen_deviation = 0; //calculate General Deviation
	//copy the solution and neighborhood
	vector<int> S_cop2 = S_cop;
	vector<int> S_cop3 = S_cop2;
	int nodesInClust;
	float new_f = 0;

	//walk through all neigbours
	for(auto vecino:vecindario){

		//if the cluster have more 1 node
		if(S_cop2[vecino.first] != vecino.second){
			//count the number of nodes of the cluster
			nodesInClust = countCluster(S_cop2, S_cop2[vecino.first]);
			if(nodesInClust>1){
				//change cluster
				S_cop2[vecino.first] = vecino.second;
				//calculate the fitness
				gen_deviation = generalDeviation(S_cop2);
				infease = infeasibility(S_cop2);
				new_f = gen_deviation + (infease * landa);

				//if the new fitness is better
				if(new_f < f){
					//update the current solution
					f = new_f;
					S_cop3 = S_cop2;

				}//else restore to the previous solution
				S_cop2 = S_cop3;

				//if the iterate reaches 100000
				if(it == max){
					return S_cop3;//ends
				}
			}
		}
		++it;
	}

	//if it calculate all the possibilities, it ends
	return S_cop3;
}

//find all elements of the cluster
vector<int> PARBT::findInCluster(const vector<int> &s_cop, int clust){
	vector<int> elements;
	for(unsigned int i=0; i<s_cop.size(); ++i)
		if(clust == s_cop[i])
			elements.push_back(i);

	return elements;
}
//find all elements of the cluster
int PARBT::countCluster(const vector<int> &s_cop, int clust){
	int elements=0;
	for(unsigned int i=0; i<s_cop.size(); ++i)
		if(clust == s_cop[i])
			elements++;

	return elements;
}

//calculate the general deviation
float PARBT::generalDeviation(const vector<int> &s_cop){
	float distance = 0, intra_cluster = 0;
	vector<int> elements;
	vector<vector<float>> centroides;
	centroides = updateDistance(s_cop);

		//walk through each cluster
		for(int i = 0; i< k; ++i){

			elements = findInCluster(s_cop,i);
			//sumatorry(euclidean distance of all nodes in the cluster)
			for(vector<int>::iterator it = elements.begin(); it != elements.end(); ++it){
				distance += distanciaEuclidea(atributos[(*it)],centroides[i]);
			}
			//mean intra-cluster distance
			distance = distance / elements.size();
			intra_cluster += distance;
			distance = 0;
		}

		return intra_cluster/k;
}



//calculate Landa
float PARBT::createLanda(){
	float lan = 0, actual_distance=0;

	//calculate the maximum distance
	for(unsigned int i = 0; i < atributos.size(); ++i){
		for(unsigned int e = i+1; e < atributos.size(); ++e){
			actual_distance = distanciaEuclidea(atributos[i],atributos[e]);
			if(actual_distance > lan){
				lan = actual_distance;
			}
		}
	}
	//count the number of restrictions total
	uvec rest = (find(matriz == 1 or matriz == -1));

	//max distance / total number of problem restrictions
	lan = lan/rest.size();

	return lan;
}
