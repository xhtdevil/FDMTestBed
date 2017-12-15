#ifndef FDM_H
#define FDM_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<time.h>
#include <stack>
#include<vector>
#include<unordered_map>
#include <string>
#include <algorithm>

#include <iostream>
using namespace std;

#define EPSILON 0.0001
#define DELTA 0.002
#define STEP 0.05

#define LOAD_UDP

extern unordered_map<string, string> IPtable;

void SetLinkLens(int nl, double Flow[], double Cap[], int MsgLen, double Len[], double Cost[]);
void SetSP(int nn, int link, int *End2, double* Len, int** Adj, double** SPdist, int** SPpred);
void Bellman(int nn, int link, int root, int* End2, double* LinkLength, int** Adj, int* Pred, double* Dist);
void LoadLinksMPTCP(int nn, int nl, double** Req, int** SPpred, int* End1, double* Flow, vector<unordered_map<string,double>>& Table);
double AdjustCaps(int nl, double Flow[], double Cap[], double NewCap[]);
double CalcDelay(int nl, double* Flow, double Cap[], int MsgLen, double TotReq, double Cost[]);
void Superpose(int nl, double Eflow[], double Gflow[], double Cap[], double TotReq, int MsgLen, double Cost[], vector<unordered_map<string, double>>& Gtable, vector<unordered_map<string, double>>& Etable);
double FindX(int nl, double Gflow[], double Eflow[], double Cap[], double TotReq, int MsgLen, double Cost[]);
double LinkDelay(double Flow, double Cap, int MsgLen, double Cost);
double DerivDelay(double Flow, double Cap, int MsgLen, double Cost);
double Deriv2Delay(double Flow, double Cap, int MsgLen);
double DelayF(double x, int nl, double Eflow[], double Gflow[], double Cap[], int MsgLen, double TotReq, double Cost[]);

#ifdef LOAD_UDP
void LoadLinksUDP(int nn, int nl, double** udp_req, int** SPpred, int* End1, double* udp_flow,vector<unordered_map<string, double>>& Table, double Gflow[], double NewCap[], int MsgLen, double FDlen[], double Cost[], int link, int *End2, int** Adj, double** SPdist, double* mptcp_flow);
#endif

#endif
