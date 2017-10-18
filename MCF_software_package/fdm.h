#ifndef FDM_H
#define FDM_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<time.h>
#include <stack>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<string>
#include <iostream>
using namespace std;

#define EPSILON 0.0001
#define DELTA 0.002
#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)
#define STEP 0.05




void SetLinkLens(int nl, double Flow[], double Cap[], int MsgLen, double Len[], double Cost[]);
void SetSP(int nn, int link, int *End2, double* Len, int** Adj, double** SPdist, int** SPpred);
void Bellman(int nn, int link, int root, int* End2, double* LinkLength, int** Adj, int* Pred, double* Dist);
void LoadLinks(int nn, int nl, double** Req, int** SPpred, int* End1, double* Flow);
double AdjustCaps(int nl, double Flow[], double Cap[], double NewCap[]);
double CalcDelay(int nl, double* Flow, double Cap[], int MsgLen, double TotReq, double Cost[]);
void Superpose(int nl, double Eflow[], double Gflow[], double Cap[], double TotReq, int MsgLen, double Cost[]);
double FindX(int nl, double Gflow[], double Eflow[], double Cap[], double TotReq, int MsgLen, double Cost[]);
double LinkDelay(double Flow, double Cap, int MsgLen, double Cost);
double DerivDelay(double Flow, double Cap, int MsgLen, double Cost);
double Deriv2Delay(double Flow, double Cap, int MsgLen);
double DelayF(double x, int nl, double Eflow[], double Gflow[], double Cap[], int MsgLen, double TotReq, double Cost[]);



void SetLinkLens(int nl, double Flow[], double Cap[], int MsgLen, double Len[], double Cost[]) {
	for (int l = 0; l < nl; l++) {
		Len[l] = DerivDelay(Flow[l], Cap[l], MsgLen, Cost[l]);
	}
}
void SetSP(int nn, int link, int *End2, double* Len, int** Adj, double** SPdist, int** SPpred) {

	for (int node = 0; node < nn; node++) {
		Bellman(nn, link, node, End2, Len, Adj, SPpred[node], SPdist[node]);
	}
}

void Bellman(int nn, int link, int root, int* End2, double* LinkLength, int** Adj, int* Pred, double* Dist) {
	vector<int> Hop(nn, 0);
	for (int i = 0; i < nn; i++) {
		Dist[i] = INFINITY;
		Hop[i] = 0;
	}
	Dist[root] = 0;
	Pred[root] = root;


	stack<int> scanqueue;
	scanqueue.push(root);
	while (!scanqueue.empty()) {
		int node = scanqueue.top();
		scanqueue.pop();
		for (int i = 0; i < link; i++) {
			int curlink = Adj[node][i];
			if (curlink == -1)
				break;
			int node2 = End2[curlink];
			double d = Dist[node] + LinkLength[curlink];
			if (Dist[node2] > d) {
				Dist[node2] = d;
				//if (node2 < 0 || node2 >= nn)
				//	cout << "dd\n";
				Pred[node2] = curlink;
				Hop[node2] = Hop[node] + 1;
				//setting hop limit to restrict ship relays
				if (Hop[node2]<3)
					scanqueue.push(node2);
			}
		}

	}
}




void LoadLinks(int nn, int nl, double** Req, int** SPpred, int* End1, double* Flow) {
	int m;
	int p;
	int link;
	for (int i = 0; i < nl; i++) {
		Flow[i] = 0;
	}
	for (int s = 0; s < nn; s++) {
		for (int d = 0; d < nn; d++) {
			if (Req[s][d] > 0) {
				m = d;
				while (m != s) {
					link = SPpred[s][m];
					p = End1[link];
					Flow[link] += Req[s][d];
					m = p;
				}
			}
		}
	}
}
double AdjustCaps(int nl, double Flow[], double Cap[], double NewCap[]) {
	double factor = 1;
	for (int i = 0; i < nl; i++) {
		factor = max(factor, (1 + DELTA)*Flow[i] / Cap[i]);
	}
	for (int q = 0; q < nl; q++) {
		NewCap[q] = factor*Cap[q];
	}
	return factor;
}
double CalcDelay(int nl, double* Flow, double Cap[], int MsgLen, double TotReq, double Cost[]) {
	double sum = 0.0;
	for (int u = 0; u < nl; u++) {
		sum = sum + Flow[u] * LinkDelay(Flow[u], Cap[u], MsgLen, Cost[u]);
	}
	return sum / TotReq;
}
void Superpose(int nl, double Eflow[], double Gflow[], double Cap[], double TotReq, int MsgLen, double Cost[]) {
	double x = FindX(nl, Gflow, Eflow, Cap, TotReq, MsgLen, Cost);
	for (int l = 0; l < nl; l++) {
		//		Pflow[l] = Gflow[l];
		Gflow[l] = x*Eflow[l] + (1 - x)*Gflow[l];

	}
}


double FindX(int nl, double Gflow[], double Eflow[], double Cap[], double TotReq, int MsgLen, double Cost[]) {

	double xLimit, st = 0, end = 1;
	vector<double> Flow(nl, 0);

	for (; (end - st)>0.0001;) {
		bool exc = false;
		xLimit = st + (end - st) / 2;
		for (int i = 0; i < nl; i++) {
			Flow[i] = xLimit*Eflow[i] + (1 - xLimit)*Gflow[i];
			if (Flow[i] > Cap[i]) {
				exc = true;
				break;
			}
		}
		if (exc) {
			end = xLimit;
		}
		else
			st = xLimit;
	}

	xLimit = st;


	double x0 = 0.0; double f0 = DelayF(x0, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);
	//double x4 = 1.0; double f4 = DelayF(x4, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);
	double x4 = xLimit; double f4 = DelayF(x4, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);
	//double x2 = 0.5; double f2 = DelayF(x2, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);
	double x2 = (x0 + x4) / 2; double f2 = DelayF(x2, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);

	while (x4 - x0 > EPSILON) {
		double x1 = (x0 + x2) / 2; double f1 = DelayF(x1, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);
		double x3 = (x2 + x4) / 2; double f3 = DelayF(x3, nl, Eflow, Gflow, Cap, MsgLen, TotReq, Cost);
		if ((f0 <= f1) || (f1 <= f2)) {
			x4 = x2; x2 = x1;
			f4 = f2; f2 = f1;
		}
		else if (f2 <= f3) {
			x0 = x1; x4 = x3;
			f0 = f1; f4 = f3;
		}
		else {
			x0 = x2; x2 = x3;
			f0 = f2; f2 = f3;
		}
	}
	if ((f0 <= f2) && (f0 <= f4)) {
		return(x0);
	}
	else if (f2 <= f4) {
		return(x2);
	}
	else {
		return (x4);
	}
}
double DelayF(double x, int nl, double Eflow[], double Gflow[], double Cap[], int MsgLen, double TotReq, double Cost[]) {
	double* Flow = new double[nl];
	for (int l = 0; l < nl; l++) {
		Flow[l] = x*Eflow[l] + (1 - x)*Gflow[l];
	}

	double res = CalcDelay(nl, Flow, Cap, MsgLen, TotReq, Cost);
	delete[] Flow;
	return res;
}
double LinkDelay(double Flow, double Cap, int MsgLen, double Cost) {
	return ((MsgLen / Cap) / (1 - Flow / Cap) + Cost);
}
double DerivDelay(double Flow, double Cap, int MsgLen, double Cost) {
	double f = 1 - Flow / Cap;
	return((MsgLen / Cap) / (f*f) + Cost);
}
double Deriv2Delay(double Flow, double Cap, int MsgLen) {
	double f = 1 - Flow / Cap;
	return(2 * (MsgLen / Cap) / (Cap*f*f*f));
}


pair<bool,double> fdm(int n_ship, int n_sat, double cap, double req, double prob_of_conn, double density, double* Result, int cc) {

	int choice=cc;

	vector<vector<int> > connectivity;
	int count_link = 0;

	vector<vector<double> > requests;

	vector<double> sat_capacities;
	vector<vector<double> > downlink_capacities;
	unordered_map<int, int> srcDest;


	if (choice == 0) {
		/*#######################################################
		##			Specify number of ships, sats			   ##
		##			and connectivity(adj matrix)			   ##
		#########################################################
		*/


		connectivity.resize(n_ship, vector<int>(n_sat, 0));
		requests.resize(n_ship, vector<double>(n_ship, 0));
		sat_capacities.resize(n_sat, 0);
		downlink_capacities.resize(n_sat, vector<double>(n_ship, 0));


		
		bool random_input = true;
		srand(time(NULL));
		double max_requests = req;
		double max_capacity = cap;
		double max_downlink = cap;
		double min_rand_request = 0.5, min_rand_capacity = 0.5, min_rand_downlink = 0.5;

		//matrix with m ships, n satellites, and link info


		bool regenerate = true;

		//generate topology and src-dest pairs 
		//that are connected through at least one sat
		while (regenerate) {
			regenerate = false;
			count_link = 0;
			//randomly generate connectivity
			for (int i = 0; i < connectivity.size(); i++) {
				int link_per_ship = 0;
				for (int j = 0; j < connectivity[i].size(); j++) {
					double r = (double)rand() / (RAND_MAX);
					if (r <= prob_of_conn) {
						connectivity[i][j] = 1;
						count_link++;
						link_per_ship++;
					}
				}
				//if a ship has no connection to sat, 
				// randomly connect to one
				if (link_per_ship == 0) {
					int ind = rand() % n_sat;
					connectivity[i][ind] = 1;
					count_link++;
				}
			}

			//randomly pair up source and dest	
			for (int i = 0; i < n_ship; i++) {
				double r = (double)rand() / (RAND_MAX);
				if (r <= density) {
					vector<int> candidate_dest;
					for (int j = 0; j < n_sat; j++) {
						if (connectivity[i][j]) {
							for (int k = 0; k < n_ship; k++) {
								if (k != i&&connectivity[k][j] && \
									find(candidate_dest.begin(), candidate_dest.end(), k) == candidate_dest.end()) candidate_dest.push_back(k);
							}
						}
					}
					//if there are no paths, start over
					if (candidate_dest.size() == 0) {
						regenerate = true;
						requests.clear();
						requests.resize(n_ship, vector<double>(n_ship, 0));
						connectivity.clear();
						connectivity.resize(n_ship, vector<int>(n_sat, 0));
						break;
					}
					//randomly generate requests
					int dest = rand() % candidate_dest.size();
					if (random_input) {
						double r = (1 - min_rand_request)*((double)rand() / (RAND_MAX));
						requests[i][candidate_dest[dest]] = (min_rand_request + r)*max_requests;
					}
					else
						requests[i][candidate_dest[dest]] = max_requests;
					srcDest[i] = candidate_dest[dest];
				}
			}

			if (regenerate) continue;

		}

		//randomly generate sat capacity
		for (int i = 0; i < n_sat; i++) {
			if (random_input) {
				double r = (1 - min_rand_capacity)*((double)rand() / (RAND_MAX));
				sat_capacities[i] = (min_rand_capacity + r)*max_capacity;
			}
			else
				sat_capacities[i] = max_capacity;
		}

		//randomly generate downlink capacity
		for (int i = 0; i < n_sat; i++) {
			for (int j = 0; j < n_ship; j++) {
				if (connectivity[j][i]) {
					if (random_input) {
						double r = (1 - min_rand_downlink)*((double)rand() / (RAND_MAX));
						downlink_capacities[i][j] = (min_rand_downlink + r)*max_downlink;
					}
					else
						downlink_capacities[i][j] = max_downlink;
				}
			}

		}



		//out put all the generated inputs to file
		//can either use the generated inputs
		//or use the configuration files

		ofstream config;
		config.open("config.txt");
		config << n_ship << " " << n_sat << endl;
		for (int i = 0; i < n_ship; i++) {
			for (int j = 0; j < n_sat; j++) {
				config << connectivity[i][j] << " ";
			}
			config << endl;
		}
		for (int i = 0; i < n_ship; i++) {
			for (int j = 0; j < n_ship; j++) {
				config << requests[i][j] << " ";
			}
			config << endl;
		}
		for (int i = 0; i < n_sat; i++) {
			config << sat_capacities[i] << " ";
		}
		config << endl;
		for (int i = 0; i < n_sat; i++) {
			for (int j = 0; j < n_ship; j++) {
				config << downlink_capacities[i][j] << " ";
			}
			config << endl;
		}

		config.close();
	}
	else {
		/*#######################################################
		##				Read topology from file				   ##
		#########################################################
		*/

//		string configFile;
//		cout << "config filename: ";
//		getline(cin, configFile);
//		ifstream config(configFile);
		ifstream inputFile;
		inputFile.open("/Users/haotianxu/Desktop/GSR/beginner_material/MCF_software_package/config_random.txt");
		if (!inputFile.is_open()) {
			cout << "file cannot be opened" << endl;
			pair<bool, double> nothing(false, 0);
			return nothing;
		}

		inputFile >> n_ship >> n_sat;

		connectivity.resize(n_ship, vector<int>(n_sat, 0));
		requests.resize(n_ship, vector<double>(n_ship, 0));
		sat_capacities.resize(n_sat, 0);
		downlink_capacities.resize(n_sat, vector<double>(n_ship, 0));

		for (int i = 0; i < n_ship; i++) {
			for (int j = 0; j < n_sat; j++) {
				inputFile >> connectivity[i][j];
				if (connectivity[i][j])
					count_link++;
			}
		}
		bool turb = true;
		for (int i = 0; i < n_ship; i++) {
			for (int j = 0; j < n_ship; j++) {
				inputFile >> requests[i][j];
				if (requests[i][j] > 0) {
					srcDest[i] = j;
					//small disturbance
					if (turb) {
						requests[i][j] -= requests[i][j] * 0.1;
						turb = false;
					}
				}
			}
		}
		for (int i = 0; i < n_sat; i++) {
			inputFile >> sat_capacities[i];
		}
		for (int i = 0; i < n_sat; i++) {
			for (int j = 0; j < n_ship; j++) {
				inputFile >> downlink_capacities[i][j];
			}
		}

	}


	/*#######################################################
	##					Topology Builder				   ##
	#########################################################
	*/
	int nn = n_ship + 2 * n_sat;
	int nl = 2 * count_link + n_sat;
	ofstream output;
	if (cc == 1)
		output.open("allocation_rerun.txt");
	else
		output.open("allocation.txt");


	vector<int> ships(n_ship, 0);
	vector<int> sats(n_sat, 0);
	for (int i = 0; i < n_ship; i++) {
		ships[i] = i;
	}
	for (int i = 0; i < n_sat; i++) {
		sats[i] = n_ship + i;
	}

	vector<pair<pair<int, int>, double> > v_pairs(nl);
	//adding uplinks
	int help_count = 0;
	for (int i = 0; i < n_ship; i++) {
		output << "link between ship " << i << " and sat ";
		for (int j = 0; j < n_sat; j++) {
			if (connectivity[i][j]) {
				v_pairs[help_count] = {{ i,n_ship + j },-1 };
				output << j << " ";
				help_count++;
			}
		}
		output << '\t' << '\t' << '\t' << "destination is ship " << srcDest[i] << endl;
	}
	output << endl;
	//adding dummy nodes and link capacities
	for (int i = 0; i < n_sat; i++) {
		v_pairs[help_count] = {{ n_ship + i,n_ship + n_sat + i },sat_capacities[i] };
		output << "capacity of sat " << i << " " << sat_capacities[i] << endl;
		help_count++;
	}

	//adding downlinks
	for (int j = 0; j < n_sat; j++) {
		for (int i = 0; i < n_ship; i++) {
			if (connectivity[i][j]) {
				v_pairs[help_count] = {{ n_ship + n_sat + j,i },downlink_capacities[j][i] };
				help_count++;
			}
		}
	}

	//adj matrix
	vector<vector<int> > adj(nn, vector<int>());

	int link = max(n_sat, n_ship);
	int *End1 = new int[nl], *End2 = new int[nl], **Adj = new int *[nn];
	for (int i = 0; i < nn; i++) {
		Adj[i] = new int[link];
	}

	//initialize Adj with -1
	for (int i = 0; i < nn; i++)
		for (int j = 0; j < link; j++)
			Adj[i][j] = -1;

	double* Cap = new double[nl];

	//set end1, end2, cap and adj
	int link_count = 0;
	for (auto it : v_pairs) {
		auto end_points = it.first;
		double capacity = it.second;
		End1[link_count] = end_points.first;
		End2[link_count] = end_points.second;
		adj[end_points.first].push_back(link_count);
		Cap[link_count] = capacity == -1 ? INFINITY : capacity;
		link_count++;
	}
	//set Adj from adj
	int node_count = 0;
	for (auto v : adj) {
		for (int i = 0; i < v.size(); i++)
			Adj[node_count][i] = v[i];
		node_count++;
	}

	double** Req = new double*[nn], **MM_Req = new double*[nn], ** SPdist = new double*[nn];
	for (int i = 0; i < nn; i++) {
		Req[i] = new double[nn];
		MM_Req[i] = new double[nn];
		SPdist[i] = new double[nn];
	}

	double CurrentDelay;
	double *Gflow = new double[nl];
	double *Eflow = new double[nl];
	double *Pflow = new double[nl];


	int **SPpred = new int*[nn];
	for (int i = 0; i < nn; i++) {
		SPpred[i] = new int[nn];
	}

	double* FDlen = new double[nl];
	for (int i = 0; i < nl; i++) {
		FDlen[i] = 0;
	}
	double* NewCap = new double[nl];
	//double Cost[NL] = { 0,0,0,0,0,0,0,2,10,1,0,0,0,0,0,0,0 };
	double* Cost = new double[nl];
	for (int i = 0; i < nl; i++) {
		Cost[i] = 0;
	}
	double Aresult;
	int Aflag;
	int MsgLen = 1;
	double PreviousDelay = INFINITY;
	double TotReq = 0.0;


	int print = 1;

	//MM_Req is for Max-Min when infeasible
	for (int i = 0; i < nn; i++) {
		for (int n = 0; n < nn; n++) {
			Req[i][n] = 0;
			MM_Req[i][n] = 0;
		}
	}


	for (int i = 0; i < n_ship; i++) {
		for (int j = 0; j < n_ship; j++) {
			if (requests[i][j] > 0) {
				Req[i][j] = requests[i][j];
			}

		}
	}


	for (int i = 0; i < nn; i++) {
		for (int n = 0; n < nn; n++) {
			TotReq += Req[i][n];
		}
	}
	for (int i = 0; i < nl; i++) {
		Gflow[i] = 0;
	}


	/*#######################################################
	##					FDM algorithm					   ##
	#########################################################
	*/

	//time start
	const clock_t start = clock();
	//feasible symbol
	bool feasible;

	SetLinkLens(nl, Gflow, Cap, MsgLen, FDlen, Cost);
	SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
	LoadLinks(nn, nl, Req, SPpred, End1, Gflow);
	Aresult = AdjustCaps(nl, Gflow, Cap, NewCap);
	if (Aresult == 1)
		Aflag = 0;
	else
		Aflag = 1;
	CurrentDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);

	int count = 0;
	//start to run FDM
	while (Aflag || (CurrentDelay < PreviousDelay*(1 - EPSILON))) {
		SetLinkLens(nl, Gflow, NewCap, MsgLen, FDlen, Cost);
		SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
		LoadLinks(nn, nl, Req, SPpred, End1, Eflow);
		//previous delay based on current NewCap
		PreviousDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);
		Superpose(nl, Eflow, Gflow, NewCap, TotReq, MsgLen, Cost);
		//current delay after superposition
		CurrentDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);

		//PreviousDelay = CurrentDelay;
		//CurrentDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);


		if (Aflag) {
			Aresult = AdjustCaps(nl, Gflow, Cap, NewCap);
			if (Aresult == 1)
				Aflag = 0;
			else
				Aflag = 1;
		}

		//judge whether the problem is feasible 
		//double max_FD_len = 0, min_FD_len = INFINITY;
		//for (int i = 0; i < nl; i++) {
		//	if (FDlen[i] > 0) {
		//		max_FD_len = max(max_FD_len, FDlen[i]);
		//		min_FD_len = min(min_FD_len, FDlen[i]);
		//	}
		//}
		if ((Aflag == 1 && (CurrentDelay >= PreviousDelay*(1 - EPSILON))) || count >= 100000) {
			//if ((Aflag == 1 && (max_FD_len - min_FD_len)<EPSILON)||count==100) {
			output << ("The problem is infeasible. Now reduce the request.\n");
			print = 0;
			break;
		}

		//for(i = 0; i < nl; i ++) {
		//	printf("Gflow[%d] in iteration is %f\n", i,Gflow[i]);
		//}

		//printf("%f\n", PreviousDelay);
		count++;
	}
	if (print) {

        // File recording the input of python testbed
        ofstream testbedInput;
        testbedInput.open("TestbedInput.txt");
        testbedInput << n_ship << "," << n_sat << endl;
		for(int i = 0; i < nl; i ++) {
            testbedInput << i  << "," << End1[i]  << "," << End2[i] << "," << Gflow[i] << endl;
		}

		//count  traffic at each ship
		for (auto u : ships) {
			double sum = 0;
			output << "Ship " << u << ":\n";
			for (auto l : adj[u]) {
				if (Gflow[l] > 0) {
					output << "Usage at sat " << End2[l] - n_ship << " is " << Gflow[l] << "\n";
					sum += Gflow[l];
				}
			}
			output << "Total out going flow at ship " << u << " is " << sum << endl;

			sum = 0;
			int dest = srcDest[u];
			for (int i = 0; i < nl; i++) {
				if (End2[i] == dest) {
					output << "Downlink at sat " << End1[i] - n_ship - n_sat << " is " << Gflow[i] << "\n";
					sum += Gflow[i];
				}
			}
			output << "Total in comming flow at ship " << dest << " is " << sum << endl << endl;
		}
		//count traffic at each satellite
		for (auto s : sats) {
			for (auto l : adj[s]) {
				output << "Total load at sat " << s << " is " << Gflow[l] << "\n";
			}
		}

		if (cc == 0){
			for (auto u : ships) {
				for (auto l : adj[u]) {
					Result[l] = Gflow[l];
				}
			}
		}
		else {
			ofstream file;
			file.open("performance_error_new.txt");
			for (auto u : ships) {
				//cout << "Ship " << u << " diff:";
				double sum = 0, req=0;
				for (auto l : adj[u]) {
					sum += abs(Result[l] - Gflow[l]);
					req += Result[l];
				}
				file << sum / req << endl;
			}
			file.close();
		}
		//cout << "Problem is feasible. Results in allocation.txt!\n";
		
		//printf("current delay is %f\n", CurrentDelay);
		//printf("current count is %d\n", count);
	}
	
	// time end
	double seconds_since_start = float(clock() - start) / CLOCKS_PER_SEC;
	//output << "Computing time is " << seconds_since_start << "\n";
	pair<bool, double> res(print, seconds_since_start);
	output.close();


	//recycle
	delete[] End1, End2, Cap, Gflow, Eflow, Pflow, FDlen, NewCap, Cost;
	for (int i = 0; i < nn; i++) {
		delete[] Adj[i], Req[i], MM_Req[i], SPdist[i], SPpred[i];
	}
	delete[] Adj, Req, MM_Req, SPdist, SPpred;



	return res;
}

#endif