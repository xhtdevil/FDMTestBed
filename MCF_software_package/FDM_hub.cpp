#include "fdm.h"
#include <math.h>
#include<time.h>
#include <stack>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<iostream>
#include <string>
#include <set>
#include <algorithm>

int main() {
	int loop = 1;
	int n_ship, n_sat, n_host, n_src_host;
	vector<vector<int>> host_ship_connect;
	vector<vector<int>> connectivity;
	int count_link = 0;
	vector<int> ship_sat;
	vector<vector<double>> requests;
	vector<vector<double>> uplink_capacities;
	vector<double> sat_capacities;
	vector<double> downlink_capacities;
	unordered_map<int, int> srcDest;

	/*#######################################################
	##				Read topology from file				   ##
	#########################################################
	*/

	string configFile;
	cout << "config filename: ";
	getline(cin, configFile);
	ifstream config(configFile);
	if (!config.is_open()) {
		cout << "file cannot be opened" << endl;
		return 0;
	}


	config >> n_ship >> n_sat;

	n_host = 0;

	host_ship_connect.resize(n_ship);
	connectivity.resize(n_ship, vector<int>(n_sat, 0));
	//requests.resize(n_ship, vector<double>(n_ship, 0));
	uplink_capacities.resize(n_ship, vector<double>(n_sat, 0));
	sat_capacities.resize(n_sat, 0);
	downlink_capacities.resize(n_sat, 0);



	for (int i = 0; i < n_ship; i++) {
		int num_host = 0;
		config >> num_host;
		for (int h = 0; h < num_host; h++)
			host_ship_connect[i].push_back(h + n_host);
		n_host += num_host;
	}
	//common destination for all source hosts
	int dest_hub = n_host;
	//total number of host is n_src_host+1
	n_host++;

	ship_sat.resize(n_ship, 0);

	for (int i = 0; i < n_ship; i++) {
		for (int j = 0; j < n_sat; j++) {
			config >> connectivity[i][j];
			if (connectivity[i][j]) {
				ship_sat[i]++;
				count_link++;
			}
		}
	}

	n_src_host = n_host -1;
	requests.resize(n_host, vector<double>(n_host, 0));

	vector<int> tab(n_ship, 0);
	for (int i = 0; i < n_src_host; i++) {
		int srcship,destship=dest_hub;
		double demand;
		config >> srcship >> demand;
		int srcid = tab[srcship];
		tab[srcship]++;
		requests[host_ship_connect[srcship][srcid]][destship] = demand;
		srcDest[host_ship_connect[srcship][srcid]] = destship;
	}

	for (int i = 0; i < n_ship; i++) {
		for (int j = 0; j < n_sat; j++)
			config >> uplink_capacities[i][j];
	}

	for (int i = 0; i < n_sat; i++) {
		config >> sat_capacities[i];
	}
	for (int i = 0; i < n_sat; i++) {
		config >> downlink_capacities[i];
	}


	/*#######################################################
	##					Topology Builder				   ##
	#########################################################
	*/
	//new nn and nl for hub topology
	int nn=n_host+n_ship+2*n_sat+1;
	int nl = n_host + count_link + 2 * n_sat;

	string outputFile;
	cout << "output filename: ";
	getline(cin, outputFile);
	ofstream output;
	output.open(outputFile);

	set<int> ships, hubs;
	for (int i = 0; i < n_ship; i++) {
		ships.insert(i + n_host);
	}
	hubs.insert(nn - 1);


	vector<int> ports(nn);
	vector<string> names(nn);
	for (int i = 0, cnt = 1; i < nn; i++) {
		if (i < n_host) {
			ports[i] = 0;
			names[i] = "host" + to_string(i);
		}
		else {
			ports[i] = 1;
			names[i] = "s" + to_string(cnt++);
		}
	}

	vector<pair<pair<pair<int, int>, pair<int, int>>, double>> v_pairs(nl);
	//adding uplinks
	int help_count = 0;
	//adding links between host and ship
	for (int i = 0; i < n_ship; i++) {
		for (int j = 0; j < host_ship_connect[i].size(); j++) {
			int host = host_ship_connect[i][j], ship = n_host + i;
			v_pairs[help_count] = { { { host,ports[host] },{ ship,ports[ship] } },-1 };
			ports[host]++; ports[ship] += ship_sat[i]; help_count++;
		}
	}
	//adding links between ship and sat
	for (int i = 0; i < n_ship; i++) {
		for (int j = 0; j < n_sat; j++) {
			if (connectivity[i][j]) {
				int ship = n_host + i, sat = n_host + n_ship + j;
				v_pairs[help_count] = { { { ship,ports[ship] },{ sat,ports[sat] } },uplink_capacities[i][j] };
				ports[ship]++; ports[sat]++; help_count++;
			}
		}
	}
	//adding links between sat and dum sat
	for (int i = 0; i < n_sat; i++) {
		int sat = n_host + n_ship + i, dum_sat = n_host + n_ship + n_sat + i;
		v_pairs[help_count] = { { { sat,ports[sat] },{ dum_sat,ports[dum_sat] } },sat_capacities[i] };
		ports[sat]++; ports[dum_sat]++; help_count++;
	}
	
	//adding downlinks(to hub switch)
	for (int j = 0; j < n_sat; j++) {
		int sat = n_host + n_ship + n_sat + j, hub=nn-1;
		v_pairs[help_count] = { { { sat,ports[sat] },{ hub,ports[hub] } },downlink_capacities[j] };
		ports[sat]++; ports[hub]++; help_count++;
	}
	

	//adding hub to dest_hub
	{
		int hub = nn - 1;
		v_pairs[help_count] = { { { hub,ports[hub] },{ dest_hub,ports[dest_hub] } },-1 };
		ports[hub]++; ports[dest_hub]++; help_count++;
	}

	//adj matrix
	vector<vector<int>> adj(nn, vector<int>());


	int link = max(n_sat, n_ship) + 1;
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
		End1[link_count] = end_points.first.first;
		End2[link_count] = end_points.second.first;
		adj[end_points.first.first].push_back(link_count);
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

	//define per-link hashtable to store flows
	vector<unordered_map<string, double>> Gtable(nl), Etable(nl);

	//define IPs for each source host
	for (int i = 0; i < n_ship; i++) {
		int ship = n_host + i;
		for (int j = 0; j < host_ship_connect[i].size(); j++) {
			int host = host_ship_connect[i][j];
			for (int k = 0; k < ship_sat[i]; k++) {
				int sat = End2[adj[ship][k]];
				string key = to_string(host) + " " + to_string(sat);
				string value = "10.0." + to_string(host + 1) + "." + to_string(k);
				IPtable[key] = value;
			}
		}
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


	for (int i = 0; i < n_host; i++) {
		for (int j = 0; j < n_host; j++) {
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


	SetLinkLens(nl, Gflow, Cap, MsgLen, FDlen, Cost);
	SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
	LoadLinks(nn, nl, Req, SPpred, End1, Gflow, Gtable);
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
		LoadLinks(nn, nl, Req, SPpred, End1, Eflow, Etable);
		//previous delay based on current NewCap
		PreviousDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);
		Superpose(nl, Eflow, Gflow, NewCap, TotReq, MsgLen, Cost, Gtable, Etable);
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
		//add IP
		//print src and dest
		for (auto it : srcDest) {
			output << names[it.first] << " " << names[it.second] << " 10.0.0." + to_string(it.second + 1) << endl;
		}
		output << "End" << endl;
		unordered_map<string, vector<string>> usedIP;
		for (int link = 0; link < n_src_host; link++) {
			int src_node = End1[link];
			for (auto it : Gtable[link]) {
				if (it.second > 1.0e-5) {
					usedIP[names[src_node]].push_back(it.first);
				}
			}
		}


		//output nodes and links in order
		for (int i = 0; i < nn; i++) {
			if (i < n_host)
				output << "add host: " << names[i] << endl;
			else if (ships.find(i) != ships.end())
				output << "add ship: " << names[i] << endl;
			else if (hubs.find(i) != hubs.end())
				output << "add hub: " << names[i] << endl;
			else
				output << "add sat: " << names[i] << endl;
		}
		output << "End" << endl;
		for (int i = 0; i < nl; i++) {
			//repeat num of links between source host and ship
			if (i < n_src_host) {
				for (int j = 0; j<ship_sat[End2[i] - n_host]; j++) {
					output << "add link: " << names[End1[i]] << " " << names[End2[i]] << endl;
				}
			}
			else
				output << "add link: " << names[End1[i]] << " " << names[End2[i]] << endl;
		}
		output << "End" << endl;
		//printing ip
		for (int i = 0; i < n_host; i++) {
			string key = names[i];
			output << key << " num_of_ip: " << usedIP[key].size() << endl;
			for (auto ip : usedIP[key])
				output << ip << endl;
		}
		output << "End" << endl;
		//printing link flow table
		for (int link = 0; link < nl; link++) {
			auto endpoints = v_pairs[link].first;
			auto node1 = endpoints.first, node2 = endpoints.second;
			int src_node = node1.first, src_port = node1.second, dst_node = node2.first, dst_port = node2.second;
			set<string> to_delete;
			for (auto it : Gtable[link]) {
				if (it.second < 1.0e-5) {
					to_delete.insert(it.first);
				}
			}
			for (auto it : to_delete)
				Gtable[link].erase(it);
			if (Gtable[link].size() == 0) continue;
			output << names[src_node] + "-eth" + to_string(src_port) << " " << names[dst_node] + "-eth" + to_string(dst_port) << "\tnum_of_flow:" << Gtable[link].size() << endl;
			for (auto it : Gtable[link]) {
				output << "\t\t" << it.first << " " << it.second << endl;
			}
		}





		////count  traffic at each ship
		//for (auto u : ships) {
		//	double sum = 0;
		//	output << "Ship " << u << ":\n";
		//	for (auto l : adj[u]) {
		//		if (Gflow[l] > 0) {
		//			output << "Usage at sat " << End2[l]-n_ship << " is " << Gflow[l] << "\n";
		//			sum += Gflow[l];
		//		}
		//	}
		//	output << "Total out going flow at ship " << u << " is " << sum << endl;

		//	sum = 0;
		//	int dest = srcDest[u];
		//	for (int i = 0; i < nl; i++) {
		//		if (End2[i] == dest) {
		//			output << "Downlink at sat " << End1[i]-n_ship-n_sat << " is " << Gflow[i] << "\n";
		//			sum += Gflow[i];
		//		}
		//	}
		//	output << "Total in comming flow at ship " << dest << " is " << sum << endl<<endl;
		//}


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
			LoadLinks(nn, nl, MM_Req, SPpred, End1, Gflow, Etable);
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
				LoadLinks(nn, nl, MM_Req, SPpred, End1, Eflow, Etable);
				PreviousDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);
				Superpose(nl, Eflow, Gflow, NewCap, TotReq, MsgLen, Cost, Gtable, Etable);
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
				if ((Aflag == 1 && (CurrentDelay >= PreviousDelay*(1 - EPSILON))) || count >= 100000) {
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
		//for (auto s : sats) {
		//	for (auto l : adj[s]) {
		//		output << "Total load at sat " << s << " is " << Pflow[l] << "\n";
		//	}
		//}

		cout << "Problem is infeasible. Max-Min solution in allocation.txt!\n";
	}

	output.close();
	//recycle
	delete[] End1, End2, Cap, Gflow, Eflow, Pflow, FDlen, NewCap, Cost;
	for (int i = 0; i < nn; i++) {
		delete[] Adj[i], Req[i], MM_Req[i], SPdist[i], SPpred[i];
	}
	delete[] Adj, Req, MM_Req, SPdist, SPpred;



	return 0;
}