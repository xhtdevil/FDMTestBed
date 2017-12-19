#include "fdm_um.h"
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <algorithm>

int main() {

	int choice=1,loop;
	//cout << "Choose 1 to provide config file\n choose 0 to randomly generate\n";
	cout << "choose 1 for loop\n choose 0 for no loop\n";
	/* ### Debug */
	//cin >> loop;
	//cin.ignore();
	cout << "default: loop = 0\n";
	loop = 0;
	

	int n_ship, n_sat, n_host, n_src_host, n_mptcp_host, n_udp_host;
	vector<vector<int>> host_ship_connect;
	vector<vector<int>> connectivity;
	vector<string> host_type;
	int count_link = 0;
	vector<int> ship_sat;
	vector<double> requests;
	vector<double> mptcp_requests;
	vector<double> udp_requests;
	vector<vector<double>> uplink_capacities;
	vector<double> sat_capacities;
	vector<double> downlink_capacities;
	unordered_map<int, int> srcDest;

	/*#######################################################
	##				Read topology from file				   ##
	#########################################################
	*/

    string configFile;
    

    /* ### Debug ### */
    cout << "config filename: ";
    getline(cin,configFile);
    //configFile = "testcase_um_hub_0.txt";

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

    ship_sat.resize(n_ship, 0);
    host_type.resize(n_host + 1, "");

    for (int i = 0; i < n_ship; i++) {
        for (int j = 0; j < n_sat; j++) {
            config >> connectivity[i][j];
            if (connectivity[i][j]) {
                ship_sat[i]++;
                count_link++;
            }
        }
    }

    n_src_host = n_host++;
    int hub_host = n_host - 1;

    // Auxiliary variables for requests input
    vector<int> tab(n_ship, 0);
    int srcship;
    double demand;

    // Input MPTCP requests
    config >> n_mptcp_host;
    mptcp_requests.resize(n_host, 0);
    for (int i = 0; i < n_mptcp_host; i++) {
        config >> srcship >> demand;
        int srcid = tab[srcship];
        host_type[host_ship_connect[srcship][srcid]] = "MPTCP";
        tab[srcship]++;
        mptcp_requests[host_ship_connect[srcship][srcid]] = demand;
        srcDest[host_ship_connect[srcship][srcid]] = hub_host;
    }

    // Input UDP requests
    config >> n_udp_host;
    udp_requests.resize(n_host, 0);
    for (int i = 0; i < n_udp_host; i++) {
        config >> srcship >> demand;
        int srcid = tab[srcship];
        host_type[host_ship_connect[srcship][srcid]] = "UDP";
        tab[srcship]++;
        udp_requests[host_ship_connect[srcship][srcid]] = demand;
        srcDest[host_ship_connect[srcship][srcid]] = hub_host;
    }

    host_type[hub_host] = "HUB";

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

	/* ### Debug - Read topology from file ### 
	cout << n_ship << " " << n_sat << " " << n_host << " " << n_src_host << " " << n_mptcp_host << " " << n_udp_host << endl;
	cout << "host_ship_connect" << endl;
	for (auto &r : host_ship_connect) {
		for (auto &c : r) cout << c << " ";
		cout << endl;
	}
	cout << "connectivity" << endl;
	for (auto &r : connectivity) {
		for (auto &c : r) cout << c << " ";
		cout << endl;
	}
	cout << "ship_sat" << endl;
	for (auto &c : ship_sat) cout << c << endl;
	cout << "mptcp_requests" << endl;
	for (auto &c : mptcp_requests) cout << c << endl;
	cout << "udp_requests" << endl;
	for (auto &c : udp_requests) cout << c << endl;
	cout << "host_type" << endl;
	for (auto &c : host_type) cout << c << endl;
	cout << "uplink_capacities" << endl;
	for (auto &r : uplink_capacities) {
		for (auto &c : r) cout << c << " ";
		cout << endl;
	}
	cout << "sat_capacities" << endl;
	for (auto &c : sat_capacities) cout << c << endl;
	cout << "downlink_capacities" << endl;
	for (auto &c : downlink_capacities) cout << c << endl;
	cout << "srcDest" << endl;
	for (auto &c : srcDest) cout << c.first << " " << c.second << endl;
	return 0;
	*/

	/*#######################################################
	##					Topology Builder				   ##
	#########################################################
	*/
	int nn = n_host + n_ship + 2 * n_sat + 1;
	/*if(loop)
		nn = n_host+n_ship + 3 * n_sat;
	//using mirror topology to prevent loop
	else
		nn = n_host + 2 * n_ship + 3 * n_sat;*/

    //int nl = n_host+2 * count_link + 2*n_sat;
    int nl = n_host + count_link + 2 * n_sat;

	string outputFile;
	

    /* ### Debug ### */
    cout<<"output filename: ";
	getline(cin,outputFile);
	//outputFile = "allocation.txt";

	ofstream output;
	output.open(outputFile);

	set<int> ships;
	for (int i = 0; i < n_ship; i++) {
		ships.insert(i + n_host);
    }
    int hub = nn - 1;


	vector<int> ports(nn);
	vector<string> names(nn);
	for (int i = 0,cnt=1; i < nn; i++) {
		if (i < n_host) {
			ports[i] = 0;
			names[i] = "host" + to_string(i);
		}
		else {
			ports[i] = 1;
			names[i] = "s" + to_string(cnt++);
		}
	}

	vector<pair<pair<pair<int,int>,pair<int,int>>,double>> v_pairs(nl);
	//adding uplinks
	int help_count = 0;
	//adding links between host and ship
	for (int i = 0; i < n_ship; i++) {
		//output << "link between ship " << i << " and host " << endl;
		for (int j = 0; j < host_ship_connect[i].size(); j++) {
			int host = host_ship_connect[i][j], ship = n_host + i;
			v_pairs[help_count] = { {{host,ports[host]},{ship,ports[ship]}},-1 };
			//output << host << ", ship port: " << ports[ship] << " host port: " << ports[host]<<endl;
			//output << "\t\t\tdestination host is " << srcDest[host] << endl;
			ports[host]++; ports[ship]+=ship_sat[i]; help_count++;
		}
	}
	//adding links between ship and sat
	for (int i = 0; i < n_ship; i++) {
		//output << "link between ship " << i << " and sat " << endl;
		for (int j = 0; j < n_sat; j++) {
			if (connectivity[i][j]) {
				int ship = n_host + i, sat = n_host + n_ship + j;
				v_pairs[help_count] = { {{ship,ports[ship]}, {sat,ports[sat]} },uplink_capacities[i][j] };
				//output << j<<" , ship port:"<<ports[ship]<<" sat prot:"<<ports[sat]<<endl;
				ports[ship]++; ports[sat]++; help_count++;
			}
		}
    }
    
    //adding links between sat and mirror sat
	for (int i = 0; i < n_sat; i++) {
		int sat = n_host + n_ship + i, mirror_sat = sat + n_sat;
		v_pairs[help_count] = { {{sat,ports[sat]}, {mirror_sat,ports[mirror_sat]}},sat_capacities[i] };
		//output << "capacity of sat " << i << " " << sat_capacities[i] << endl;
		//output << "sat port:" << ports[sat] << " , hub port:" << ports[hub] << endl;
		ports[sat]++; ports[mirror_sat]++; help_count++;
	}

	//adding links between mirror sat and hub
	for (int i = 0; i < n_sat; i++) {
		int mirror_sat = n_host + n_ship + n_sat + i;
		v_pairs[help_count] = { {{mirror_sat,ports[mirror_sat]}, {hub,ports[hub]}}, downlink_capacities[i] };
		//output << "capacity of sat " << i << " " << sat_capacities[i] << endl;
		//output << "sat port:" << ports[sat] << " , hub port:" << ports[hub] << endl;
		ports[mirror_sat]++; ports[hub]++; help_count++;
    }

    v_pairs[help_count++] = { {{hub,ports[hub]}, {hub_host,ports[hub_host]}}, -1};
    ports[hub]++;
    ports[hub_host]++;    
    
	//adj matrix
	vector<vector<int>> adj(nn, vector<int>());

	//add 1 to consider the ship interface
	int link = max(n_sat,n_ship + 1)+1;
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
	vector<unordered_map<string, double>> mptcp_Gtable(nl), mptcp_Etable(nl);
	vector<unordered_map<string, double>> udp_Gtable(nl);

	for (int i = 0; i < n_ship; i++) {
		int ship = n_host + i;
		for (int j = 0; j < host_ship_connect[i].size(); j++) {
			int host = host_ship_connect[i][j];
			for (int k = 0; k < ship_sat[i]; k++) {
				int sat = End2[adj[ship][k]];
				string key = to_string(host) + " " + to_string(sat);
				string value = "10.0." + to_string(host+1) + "." + to_string(k);
				IPtable[key] = value;
			}
		}
    }
    

	double** Req=new double* [nn], **MM_Req=new double* [nn], ** SPdist=new double* [nn];
	double** mptcp_Req = new double* [nn], **mptcp_MM_Req = new double* [nn];
	double** udp_Req = new double* [nn], **udp_MM_Req = new double* [nn];
	
	for (int i = 0; i < nn; i++) {
		mptcp_Req[i] = new double[nn];
		mptcp_MM_Req[i] = new double[nn];
		udp_Req[i] = new double[nn];
		udp_MM_Req[i] = new double[nn];
		SPdist[i] = new double[nn];
	}

	double CurrentDelay;
	double *Gflow=new double[nl];
	double *Eflow=new double[nl];
	double *Pflow=new double[nl];

	double *mptcp_Gflow = new double[nl];
	double *mptcp_Eflow = new double[nl];
	double *udp_Gflow = new double[nl];

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
			mptcp_Req[i][n] = 0;
			mptcp_MM_Req[i][n] = 0;
			udp_Req[i][n] = 0;
			udp_MM_Req[i][n] = 0;
		}
	}

	for (int i = 0; i < n_host - 1; i++) {
		if (mptcp_requests[i] > 0) {
			mptcp_Req[i][n_host - 1] = mptcp_requests[i];
		}
		if (udp_requests[i] > 0) {
			udp_Req[i][n_host - 1] = udp_requests[i];
		}
	}


	for(int i = 0; i < nn; i++) {
		TotReq += (mptcp_Req[i][n_host - 1] + udp_Req[i][n_host - 1]);
	}
	for(int i = 0; i < nl; i ++) {
		Gflow[i] = 0;
		mptcp_Gflow[i] = 0;
		udp_Gflow[i] = 0;
	}

    /* ### Debug - Topology Builder ### 
    cout << nn << " " << nl << " " << link << endl;
    cout << "End1 : End2" <<endl;
    for (int i = 0; i < nl; i++) cout << End1[i] << " : " << End2[i] << endl;
	cout << "mptcp_Req" << endl;
	for (int i = 0; i < nn; i++) {
		for (int j = 0; j < nn; j++) {
			if (mptcp_Req[i][j] > 0) cout << i << " " << j << " " << mptcp_Req[i][j] << endl;
		}
	}
	cout << "udp_Req" << endl;
	for (int i = 0; i < nn; i++) {
		for (int j = 0; j < nn; j++) {
			if (udp_Req[i][j] > 0) cout << i << " " << j << " " << udp_Req[i][j] << endl;
		}
	}
	cout << TotReq << endl;
	return 0;
    */

	/*#######################################################
	##					FDM_UM algorithm				   ##
	#########################################################
	*/


	//cout << "FDM_UM start" << endl;

	LoadUDP(nl, udp_Req, SPpred, End1, Gflow, mptcp_Gflow, udp_Gflow, udp_Gtable, nn, link, End2, FDlen, Adj, SPdist, Cap, MsgLen, Cost);
	SetLinkLens(nl, Gflow, Cap, MsgLen, FDlen, Cost);
	SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
	//LoadLinks(nn, nl, Req, SPpred, End1, Gflow, Gtable);
	LoadMPTCP(nn, nl, mptcp_Req, SPpred, End1, Gflow, mptcp_Gflow, mptcp_Gtable);
	Aresult = AdjustCaps(nl, Gflow, Cap, NewCap);
	if (Aresult == 1)
		Aflag = 0;
	else
		Aflag = 1;
	CurrentDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);

	cout << "First CurrentDelay: " << CurrentDelay << endl;

	int count = 0;
	//start to run FDM
	while(Aflag || (CurrentDelay < PreviousDelay*(1-EPSILON))) {
		//cout << count << " CurrentDelay: " << CurrentDelay << endl;
		//cout << "222" << endl;
		LoadUDP(nl, udp_Req, SPpred, End1, Gflow, mptcp_Gflow, udp_Gflow, udp_Gtable, nn, link, End2, FDlen, Adj, SPdist, Cap, MsgLen, Cost);
		SetLinkLens(nl, Gflow, NewCap, MsgLen, FDlen, Cost);
		SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
		//LoadLinks(nn, nl, mptcp_Req, SPpred, End1, mptcp_Eflow, mptcp_Etable);
		LoadMPTCP(nn, nl, mptcp_Req, SPpred, End1, Eflow, mptcp_Eflow, mptcp_Etable);
		//previous delay based on current NewCap
		PreviousDelay = CalcDelay(nl, Gflow, NewCap, MsgLen, TotReq, Cost);
		Superpose(nl, mptcp_Eflow, mptcp_Gflow, udp_Gflow, Gflow, NewCap, TotReq, MsgLen, Cost, mptcp_Gtable, mptcp_Etable);
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
		//cout << count << endl;
		if((Aflag == 1 && (CurrentDelay >= PreviousDelay*(1-EPSILON)))||count>=200) {
		//if ((Aflag == 1 && (max_FD_len - min_FD_len)<EPSILON)||count==100) {
			if (Aflag == 1) {
				output<<("The problem is infeasible. Now reduce the request.\n");
				print = 0;
			}
			else {
				print = 1;
			}
			break;
		}

		//for(i = 0; i < nl; i ++) {
		//	printf("Gflow[%d] in iteration is %f\n", i,Gflow[i]);
		//}

	 	//printf("%f\n", PreviousDelay);
		count++;
	}

	// Integrate mptcp_Gtable and udp_Gtable
	for (int i = 0; i < nl; i++) {
		Gtable[i].clear();
		for (auto it : mptcp_Gtable[i]) Gtable[i][it.first] = it.second;
		for (auto it : udp_Gtable[i]) Gtable[i][it.first] = it.second;
	}

	//cout << "FDM_UM finished!" << endl;

	/* ### Debug - FDM_UM algorithm ### */
	cout << "Gflow:" << endl;
    for (int i = 0; i < nl; i++) {
        if (Cap[i] < INFINITY) {
            cout << "Link " << i << ": " << Gflow[i] << "/" << Cap[i] << endl;
        }
	}
	cout << "TotReq: " << TotReq << endl;
    cout << "Final CurrentDelay: " << CurrentDelay << endl;
	/*cout << "Gtable" << endl;
	for (int i = 0; i < nl; i++) {
		cout << "Link " << i << endl;
		for (auto it : Gtable[i])  cout << it.first << " " << it.second << endl;
	}*/
	//return 0;

	
	/*#######################################################
	##					Output result					   ##
	#########################################################
	*/


	if(print) {
		//output<<("\n");
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
        int i = 0;
        for (; i < n_host; i++) output << "add host: " << names[i] << endl;
        for (; i < n_host + n_ship; i++) output << "add ship: " << names[i] << endl;
        for (; i < nn - 1; i++) output << "add sat: " << names[i] << endl;
        output << "add hub: " << names[i] << endl;
		output << "End" << endl;
		for (int i = 0; i < nl; i++) {
			if (i < n_src_host) {
				for (int j = 0; j<ship_sat[End2[i]-n_host]; j++) {
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
			output << key << " num_of_ip: " << usedIP[key].size() << " " << host_type[i] << endl;
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
				if (it.second < 1.0e-5){
					to_delete.insert(it.first);
				}
			}
			for (auto it : to_delete)
				Gtable[link].erase(it);
			if (Gtable[link].size()==0) continue;
			output << names[src_node] + "-eth" + to_string(src_port) << " " << names[dst_node] + "-eth" + to_string(dst_port) << "\tnum_of_flow:" << Gtable[link].size() << endl;
			for (auto it : Gtable[link]) {
				output << "\t\t" << it.first << " " << it.second << endl;
			}
		}


		cout << "Problem is feasible. Results in " << outputFile << endl;
	 	//printf("current delay is %f\n", CurrentDelay);
		//printf("current count is %d\n", count);
	}
	/* Currently not covering infeasible solution */
	else {
		cout << "Problem is infeasible.\n";
	}

	output.close();
	//recycle
	delete[] End1, End2, Cap, Gflow, Eflow, Pflow, mptcp_Gflow, mptcp_Eflow, udp_Gflow, FDlen, NewCap, Cost;
	for (int i = 0; i < nn; i++) {
		delete[] Adj[i], Req[i], MM_Req[i], mptcp_Req[i], mptcp_MM_Req[i], udp_Req[i], udp_MM_Req[i], SPdist[i], SPpred[i];
	}
	delete[] Adj, Req, MM_Req, SPdist, SPpred;



	return 0;
}
