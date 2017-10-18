#include "fdm.h"



int original_main() {

	int choice;
	cout << "Choose 1 to provide config file\n choose 0 to randomly generate\n";
	cin >> choice;
	cin.ignore();
	
	int n_ship, n_sat;

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
		cout << "provide num of ships: ";
		cin >> n_ship;
		cout << "provide num of satellites: ";
		cin >> n_sat;

		connectivity.resize(n_ship, vector<int>(n_sat, 0));
		requests.resize(n_ship, vector<double>(n_ship, 0));
		sat_capacities.resize(n_sat, 0);
		downlink_capacities.resize(n_sat, vector<double>(n_ship, 0));

		
		double prob_of_conn = 0.7;
		bool random_input = true;
		srand(time(NULL));
		double max_requests = 16;
		double max_capacity = 100;
		double max_downlink = 100;
		double min_rand_request = 0.5, min_rand_capacity = 0.5, min_rand_downlink = 0.3;

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

		string configFile;
		cout << "config filename: ";
		getline(cin,configFile);
		ifstream config(configFile);
		if (!config.is_open()) {
			cout << "file cannot be opened" << endl;
			return 0;
		}

		config >> n_ship >> n_sat;

		connectivity.resize(n_ship, vector<int>(n_sat, 0));
		requests.resize(n_ship, vector<double>(n_ship, 0));
		sat_capacities.resize(n_sat, 0);
		downlink_capacities.resize(n_sat, vector<double>(n_ship, 0));

		for (int i = 0; i < n_ship; i++) {
			for (int j = 0; j < n_sat; j++) {
				config >> connectivity[i][j];
				if (connectivity[i][j])
					count_link++;
			}
		}
		for (int i = 0; i < n_ship; i++) {
			for (int j = 0; j < n_ship; j++) {
				config >> requests[i][j];
				if (requests[i][j] > 0)
					srcDest[i] = j;
			}
		}
		for (int i = 0; i < n_sat; i++) {
			config >> sat_capacities[i];
		}
		for (int i = 0; i < n_sat; i++) {
			for (int j = 0; j < n_ship; j++) {
				config >> downlink_capacities[i][j];
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
	output.open("allocation.txt");


	vector<int> ships(n_ship,0);
	vector<int> sats(n_sat,0);
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
				v_pairs[help_count] = {{ i,n_ship + j },-1};
				output << j<<" ";
				help_count++;
			}
		}
		output <<'\t'<<'\t'<<'\t'<<"destination is ship "<<srcDest[i]<< endl;
	}
	output << endl;
	//adding dummy nodes and link capacities
	for (int i = 0; i < n_sat; i++) {
		v_pairs[help_count] = {{n_ship+i,n_ship+n_sat+i},sat_capacities[i]};
		output << "capacity of sat " << i << " " << sat_capacities[i] << endl;
		help_count++;
	}

	//adding downlinks
	for (int j = 0; j < n_sat; j++) {
		for (int i = 0; i < n_ship; i++) {
			if (connectivity[i][j]) {
				v_pairs[help_count] = {{ n_ship+n_sat+j,i },downlink_capacities[j][i]};
				help_count++;
			}
		}
	}

	//adj matrix
	vector<vector<int>> adj(nn, vector<int>());

	int link = max(n_sat,n_ship);
	int *End1 = new int[nl], *End2 = new int[nl], **Adj = new int *[nn];
	for (int i = 0; i < nn; i++) {
		Adj[i] = new int[link];
	}

	//initialize Adj with -1
	for (int i = 0; i < nn; i++)
		for (int j = 0; j < link; j++)
			Adj[i][j] = -1;

	double* Cap=new double[nl];

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

	double** Req=new double* [nn], **MM_Req=new double* [nn], ** SPdist=new double* [nn];
	for (int i = 0; i < nn; i++) {
		Req[i] = new double[nn];
		MM_Req[i] = new double[nn];
		SPdist[i] = new double[nn];
	}

	double CurrentDelay;
	double *Gflow=new double[nl];
	double *Eflow=new double[nl];
	double *Pflow=new double[nl];

	int **SPpred = new int*[nn];
	for (int i = 0; i < nn; i++) {
		SPpred[i] = new int[nn];
	}

	double* FDlen = new double[nl];
	for (int i = 0; i < nl; i++) {
		FDlen[i] = 0;
	}
	double* NewCap=new double[nl];
	//double Cost[NL] = { 0,0,0,0,0,0,0,2,10,1,0,0,0,0,0,0,0 };
	double* Cost=new double[nl];
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
	for(int i = 0; i < nn; i++) {
		for(int n = 0; n < nn; n ++) {
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

	
	for(int i = 0; i < nn; i++) {
		for(int n = 0; n < nn; n ++) {
			TotReq += Req[i][n];
		}
	}
	for(int i = 0; i < nl; i ++) {
		Gflow[i] = 0;
	}


	/*#######################################################
	##					FDM algorithm					   ## 
	#########################################################
	*/

	//time start
	const clock_t start = clock();

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
	while(Aflag || (CurrentDelay < PreviousDelay*(1-EPSILON))) {
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
		
		
		if(Aflag) {
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
		if((Aflag == 1 && (CurrentDelay >= PreviousDelay*(1-EPSILON)))||count>=100000) {
		//if ((Aflag == 1 && (max_FD_len - min_FD_len)<EPSILON)||count==100) {
			output<<("The problem is infeasible. Now reduce the request.\n");
			print = 0;
			break;
		}
		
		//for(i = 0; i < nl; i ++) {
		//	printf("Gflow[%d] in iteration is %f\n", i,Gflow[i]);
		//}
		
	 	//printf("%f\n", PreviousDelay);
		count++;
	}
	if(print) {
		output<<("\n");
	 	/*for(int i = 0; i < nl; i ++) {
			printf("Gflow[%d] is %f\n", i,Gflow[i]);
			printf("fd_length[%d] is %f\n", i, FDlen[i]);
		}*/

		//count  traffic at each ship
		for (auto u : ships) {
			double sum = 0;
			output << "Ship " << u << ":\n";
			for (auto l : adj[u]) {
				if (Gflow[l] > 0) {
					output << "Usage at sat " << End2[l]-n_ship << " is " << Gflow[l] << "\n";
					sum += Gflow[l];
				}
			}
			output << "Total out going flow at ship " << u << " is " << sum << endl;

			sum = 0;
			int dest = srcDest[u];
			for (int i = 0; i < nl; i++) {
				if (End2[i] == dest) {
					output << "Downlink at sat " << End1[i]-n_ship-n_sat << " is " << Gflow[i] << "\n";
					sum += Gflow[i];
				}
			}
			output << "Total in comming flow at ship " << dest << " is " << sum << endl<<endl;
		}
		//count traffic at each satellite
		for (auto s : sats) {
			for (auto l : adj[s]) {
				output << "Total load at sat " << s << " is " << Gflow[l] << "\n";
			}
		}

		cout << "Problem is feasible. Results in allocation.txt!\n";
	 	//printf("current delay is %f\n", CurrentDelay);
		//printf("current count is %d\n", count);
	}
	else {
		//Run Max-Min algorithm, binary search for feasible solution

		double max_request = 0, min_request = 0, mid;
		//initialize request for infeasible problem
		for (int i = 0; i < nn; i++) {
			for (int n = 0; n < nn; n++) {
				max_request = max(max_request, Req[i][n]);
			}
		}
		while ((max_request - min_request) > 0.1) {
			print = 1;
			mid = min_request + (max_request - min_request) / 2;
			for (int i = 0; i < nn; i++) {
				for (int n = 0; n < nn; n++) {
					if (Req[i][n] > 0) {
						MM_Req[i][n] = min(Req[i][n], mid);
					}
				}
			}

			TotReq = 0;
			PreviousDelay = INFINITY;

			for (int i = 0; i < nn; i++) {
				for (int n = 0; n < nn; n++) {
					TotReq += MM_Req[i][n];
				}
			}
			for (int i = 0; i < nl; i++) {
				Gflow[i] = 0;
			}
			SetLinkLens(nl, Gflow, Cap, MsgLen, FDlen, Cost);
			SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
			LoadLinks(nn, nl, MM_Req, SPpred, End1, Gflow);
			Aresult = AdjustCaps(nl, Gflow, Cap, NewCap);
			if (Aresult == 1)
				Aflag = 0;
			else
				Aflag = 1;
			CurrentDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);
			count = 0;
			while (Aflag || (CurrentDelay < PreviousDelay*(1 - EPSILON))) {
				SetLinkLens(nl, Gflow, NewCap, MsgLen, FDlen, Cost);
				SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
				LoadLinks(nn, nl, MM_Req, SPpred, End1, Eflow);
				PreviousDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);
				Superpose(nl, Eflow, Gflow, NewCap, TotReq, MsgLen, Cost);
				CurrentDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);

				if (Aflag) {
					Aresult = AdjustCaps(nl, Gflow, Cap, NewCap);
					if (Aresult == 1)
						Aflag = 0;
					else
						Aflag = 1;
				}
				//judge whether the problem is feasible 
				/*double max_FD_len = 0, min_FD_len = INFINITY;
				for (int i = 0; i < nl; i++) {
					if (FDlen[i] > 0) {
						max_FD_len = max(max_FD_len, FDlen[i]);
						min_FD_len = min(min_FD_len, FDlen[i]);
					}
				}*/
				if ((Aflag == 1 && (CurrentDelay >= PreviousDelay*(1 - EPSILON)))||count>=100000) {
					//if ((Aflag == 1 && (max_FD_len - min_FD_len)<EPSILON) || count == 100) {
						//printf("The problem becomes infeasible.\n");
					print = 0;
					break;
				}
				count++;
			}
			output << "mid is " << mid << endl;
			if (print) {
				//feasible

				min_request = mid;
			}
			else
				max_request = mid;

			//increase the MM_Req
			/*for(i = 0; i < nn; i++) {
				for(n = 0; n < nn; n++) {
					MM_Req[i][n] = min(Req[i][n], MM_Req[i][n] + STEP);

				}
			}
			if(print == 0) {
				for(i = 0; i < nl; i++) {
					printf("When the problem is feasible Gflow[%d] = %f\n", i, Pflow[i]);
				}
			}	*/
			if (print) {
				for (int i = 0; i < nl; i++) {
					Pflow[i] = Gflow[i];
				}
			}
		}

		//count  traffic at each ship
		for (auto u : ships) {
			double sum = 0;
			output << "Ship " << u << ":\n";
			for (auto l : adj[u]) {
				if (Pflow[l] > 0) {
					output << "Usage at sat " << End2[l] - n_ship << " is " << Pflow[l] << "\n";
					sum += Pflow[l];
				}
			}
			output << "Total out going flow at ship " << u << " is " << sum << endl;

			sum = 0;
			int dest = srcDest[u];
			for (int i = 0; i < nl; i++) {
				if (End2[i] == dest) {
					output << "Downlink at sat " << End1[i] - n_ship - n_sat << " is " << Pflow[i] << "\n";
					sum += Pflow[i];
				}
			}
			output << "Total in comming flow at ship " << dest << " is " << sum << endl << endl;
		}
		//count traffic at each satellite
		for (auto s : sats) {
			for (auto l : adj[s]) {
				output << "Total load at sat " << s << " is " << Pflow[l] << "\n";
			}
		}

		cout << "Problem is infeasible. Max-Min solution in allocation.txt!\n";
	}
	// time end
	double seconds_since_start = float(clock()-start)/CLOCKS_PER_SEC;
	output << "Computing time is " << seconds_since_start << "\n";
	output.close();


	//recycle
	delete[] End1, End2, Cap, Gflow, Eflow, Pflow, FDlen, NewCap, Cost;
	for (int i = 0; i < nn; i++) {
		delete[] Adj[i], Req[i], MM_Req[i], SPdist[i], SPpred[i];
	}
	delete[] Adj, Req, MM_Req, SPdist, SPpred;



	return 0;
}

double avg(vector<double> v) {
	double sum = 0;
	for (int i = 0; i < v.size(); i++) {
		sum += v[i];
	}
	return sum / v.size();
}

double stand_dev(vector<double> v, double avg) {
	double sum = 0;
	for (auto ve : v) {
		sum += (ve - avg)*(ve - avg);
	}
	return sqrt(sum / v.size());
}

int main() {

	int round = 20;
	double saturate = 0.5;
	//------ case 1 ------------
	/*
	Keep saturation ratio=total req/total cap a  constant, therefore 
	time to converge has only to do with network size
	*/
	//ofstream file;
	//file.open("performance_new.txt");
	
	//int satLimit = 10, shipLimit = 210;
	
	//double cap = 100;
	//vector<int> num_ship;

	//for (int i = 10; i <= shipLimit; i += 25)
	//	num_ship.push_back(i);
	//for (int sat = 2; sat <= satLimit; sat+=2) {
	//	for (auto ship: num_ship) {
	//		double demand = cap*sat*saturate / ship;
	//		cout << "ship " << ship << " sat " << sat << endl;
	//		int j = 0;
	//		double time = 0;
	//		vector<double> times;
	//		while (j < round) {
	//			auto res = fdm(ship, sat, cap, demand, 1, 1);
	//			if (res.first) {
	//				time += res.second; j++;
	//				times.push_back(res.second);
	//			}
	//		}
	//		double average = avg(times);
	//		file << ship << '\t' << sat<<'\t'<< average<<'\t'<<stand_dev(times,average)<< '\n';
	//	}
	//}
	//file.close();
	//cout << "done" << endl;

	//---------case 2-----------
	/*density/sparcity of transmission pairs
	keep the total load constant, the influencial factor becomes
	only the number of flows*/

	//ofstream file;
	//file.open("performance_density_new.txt");
	//int sat = 10, ship = 100;
	//double cap = 100;
	//for (double density = 0.1; density <= 1; density += 0.1) {
	//	double demand = sat*cap*saturate / (ship*density);
	//	vector<double> times;
	//	int j = 0;
	//	while (j < round) {
	//		auto res = fdm(ship, sat, cap, demand, 1, density);
	//		if (res.first) {
	//		times.push_back( res.second); j++;
	//		}
	//	}
	//	double average = avg(times);
	//	file << density << '\t' << average <<'\t'<<stand_dev(times,average)<< endl;
	//	cout << density << endl;
	//}
	//file.close();
	//cout << "done" << endl;
	//-------case 3---------
	//connectivity between ship and SATCOM
	//keep demand constant, factor becomes path diversity

	//ofstream file;
	//file.open("performance_connect_new.txt");
	//int sat = 10, ship = 100;
	//double cap = 100, demand;
	//for (double conn = 0.1; conn <= 1; conn += 0.1) {
	//	vector<double> times;
	//	demand = cap*sat*saturate / ship;
	//	int j = 0;
	//	while (j < round) {
	//		auto res = fdm(ship, sat, cap, demand, conn, 1);
	//		if (res.first) {
	//			times.push_back(res.second); j++;
	//		}
	//	}
	//	double average = avg(times);
	//	file << conn << '\t' << average <<'\t'<<stand_dev(times,average)<< endl;
	//	cout << conn << endl;
	//}
	//file.close();
	//cout << "done" << endl;

	//---------case 4------------
	//bool a;
	//do {
	//	auto res = fdm(1, 1, 1, 1, 1, 1);
	//	a = res.first;
	//} while (!a);


	int sat = 10, ship = 100;
	double cap = 100, demand;
	round = 1;
	double* result=new double[3000];
	for (double conn = 0.3; conn <= 0.3; conn += 0.1) {
		vector<double> times;
		demand = cap*sat*saturate / ship;
		int j = 0;
		while (j < round) {
			auto res = fdm(ship, sat, cap, demand, conn, 1, result, 0);
			if (res.first) {
				times.push_back(res.second); j++;
			}
		}
		//double average = avg(times);
		//file << conn << '\t' << average <<'\t'<<stand_dev(times,average)<< endl;
		//cout << conn << endl;
		j = 0;
		while (j < round) {
			auto res = fdm(ship, sat, cap, demand, conn, 1, result, 1);
			if (res.first) {
				times.push_back(res.second); j++;
			}
		}
	}

	delete[] result;
	cout << "done" << endl;

}


