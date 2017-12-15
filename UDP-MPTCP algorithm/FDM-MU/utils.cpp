#include "utils.h"

unordered_map<string, string> IPtable;


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
                //  cout << "dd\n";
                Pred[node2] = curlink;
                Hop[node2] = Hop[node] + 1;
                //setting hop limit to restrict ship relays
                if (Hop[node2]<6)
                    scanqueue.push(node2);
            }
        }

    }
}

void LoadLinksMPTCP(int nn, int nl, double** Req, int** SPpred, int* End1, double* Flow,vector<unordered_map<string, double>>& Table) {
    int m;
    int p;
    int link;
    for (int i = 0; i < nl; i++) {
        Flow[i] = 0;
        Table[i].clear();
    }
    for (int s = 0; s < nn; s++) {
        for (int d = 0; d < nn; d++) {
            if (Req[s][d] > 0) {
                vector<int> path_link, path_node;
                m = d; path_node.push_back(m);
                while (m != s) {
                    link = SPpred[s][m];
                    p = End1[link];
                    path_node.push_back(p); path_link.push_back(link);
                    //Flow[link] += Req[s][d];
                    m = p;
                }
                //get the host, sat combination to retrieve IP for this flow
                string key = to_string(s) + " " + to_string(path_node[path_node.size() - 3]);
                string ip = IPtable[key];
                for (int k = path_link.size() - 1; k >= 0; k--) {
                    Flow[path_link[k]] += Req[s][d];
                    Table[path_link[k]][ip] = Req[s][d];
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
void Superpose(int nl, double Eflow[], double MPTCP_flow[], double Cap[], double TotReq, int MsgLen, double Cost[], vector<unordered_map<string, double>>& Gtable, vector<unordered_map<string, double>>& Etable) {
    double x = FindX(nl, MPTCP_flow, Eflow, Cap, TotReq, MsgLen, Cost);

    for (int l = 0; l < nl; l++) {
        MPTCP_flow[l] = x*Eflow[l] + (1 - x)*MPTCP_flow[l];
        for (auto it : Gtable[l]) {
            Gtable[l][it.first] *= (1 - x);
        }
        for(auto it:Etable[l]){
            string key = it.first;
            double eflow = it.second;
            Gtable[l][key] += x*eflow;
        }
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

#ifdef LOAD_UDP
void LoadLinksUDP(int nn, int nl, double** udp_req, int** SPpred, int* End1, double* udp_flow, vector<unordered_map<string, double> > &Table, double Gflow[], double NewCap[], int MsgLen, double FDlen[], double Cost[], int link, int *End2, int** Adj, double** SPdist, double* mptcp_flow)
{
    int m;
    int p;
    int link_udp;

    for (int i = 0; i < nl; i++) {
        udp_flow[i] = 0;
        Table[i].clear();
    }

    for (int s = 0; s < nn; s++) {
        for (int d = 0; d < nn; d++) {
            if (udp_req[s][d] > 0) {
                // update gflow 
                for(int i = 0; i < nl; i ++) {
                    Gflow[i] = mptcp_flow[i] + udp_flow[i];
                }

                //recalculate the link length
                SetLinkLens(nl, Gflow, NewCap, MsgLen, FDlen, Cost);

                // refind shortest path
                SetSP(nn, link, End2, FDlen, Adj, SPdist, SPpred);
                
                vector<int> path_link, path_node;
                m = d; path_node.push_back(m);
                while (m != s) {
                    link_udp = SPpred[s][m];
                    p = End1[link_udp];
                    path_node.push_back(p); path_link.push_back(link_udp);
                    //udp_flow[link_udp] += udp_req[s][d];
                    m = p;
                }

                //get the host, sat combination to retrieve IP for this flow
                string key = to_string(s) + " " + to_string(path_node[path_node.size() - 3]);
                string ip = IPtable[key];
                for (int k = path_link.size() - 1; k >= 0; k--) {
                    udp_flow[path_link[k]] += udp_req[s][d];
                    Table[path_link[k]][ip] = udp_req[s][d];
                }
            }
        }
    }
}
#endif