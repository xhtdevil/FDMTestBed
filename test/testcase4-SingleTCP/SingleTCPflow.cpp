//
//  main.cpp
//  SingleTCPFlowConfig
//
//  Created by liuxin on 11/6/17.
//  Copyright © 2017 liuxin. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;
class Request
{
public:
    int nodeid;
    int shipid;
    int dst;
    
    Request(int ship,int node,int d){
        shipid=ship;
        nodeid=node;
        dst=d;
    }
    
    string getIP(vector<int> numberhost)
    {
        int sum=0;
        for(int i=0;i<shipid;i++){
            sum+=numberhost[i]+1;
        }
        sum+=nodeid;
        return "10.0.0."+to_string(sum+1)+"/32";
    }
};

int main(int argc, const char * argv[]) {
    int n_ship=0;
    int n_sat=0;
    string file;
    cout<<"Input file name:"<<endl;
    cin>>file;
    ifstream config(file);
    if(!config){
       cout<<"Cannot open file"<<endl;
    }
    if (!config.is_open()){
        cout << "Error opening file";
        exit (1);
    }
    config >> n_ship >> n_sat;

    cout<<n_ship<<endl;
    cout<<n_sat<<endl;
  
    vector<vector<int> > connectivity(n_ship,vector<int>(n_sat,0));
    vector<int>hostnumber(n_ship,0);
    vector<int>satconnect(n_sat,0);
    vector<int>shipconnect(n_ship,0);
    for (int i = 0; i < n_ship; i++)
    {
        config >> hostnumber[i];
    }
    
    int totalhost=0;
    for (int i = 0; i < n_ship; i++)
    {
         hostnumber[i]--;
         cout<< hostnumber[i]<<" ";
         totalhost+=hostnumber[i];
    }
    totalhost=totalhost;
    cout<<totalhost<<endl;
   
    for (int i = 0; i < n_ship; i++)
    {
        for (int j = 0; j < n_sat; j++)
        {
            config >> connectivity[i][j];
        }
    }
    cout<<"Connectivity between ships and sats"<<endl;
    for (int i = 0; i < n_ship; i++)
    {
        for (int j = 0; j < n_sat; j++)
        {
            cout << connectivity[i][j] << " ";
            if(connectivity[i][j]==1)
            {
                satconnect[j]++;
                shipconnect[i]++;
            }
        }
        
        cout<<endl;
    }
    
    vector<vector<int> > requests(totalhost,vector<int>(2,0));
    vector<vector<Request> > sh(n_ship);
    int dummy=0;
    int shipid=0;
    int npers=0;
    int number=hostnumber[shipid];
    for (int i = 0; i < totalhost; i++)
    {
        config >> requests[i][0];
        config >> requests[i][1];
        config >> dummy;
        if(npers<number)
        {
            Request r(requests[i][0],npers,requests[i][1]);
            npers++;
            sh[shipid].push_back(r);
        }

        else{
            npers=1;
            Request r(requests[i][0],0,requests[i][1]);
            number=hostnumber[++shipid];
            sh[shipid].push_back(r);
        }

    }
    
    for(int i=0;i<n_ship;i++)
    {
        for(auto el:sh[i]){
            cout<<"Ship:"<<el.shipid<<" node:"<<el.nodeid<<" Des:"<<el.dst<<endl;
        }
    }
    
    
    config.close();
    cout<<"Requirements between hosts and hosts"<<endl;
    for (int i = 0; i < totalhost; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            cout << requests[i][j] << " ";
        }
        cout<<endl;
    }
    
    auto nreq=requests.size();
    vector<unordered_set<int> > satused(nreq);
    vector<unordered_set<int> > reqused(n_sat);
    for(int i=0;i<nreq;i++)
    {
        int src=requests[i][0];
        int dst=requests[i][1];
        for(int j=0;j<n_sat;j++)
        {
            if(connectivity[src][j] && connectivity[dst][j])
            {
                cout<<"src:"<<src<<" dst:"<<dst<<" "<<j<<endl;
                satused[i].insert(j);
                reqused[j].insert(i);
            }
        }
    }
    
    for(int i=0;i<nreq;i++)
    {
        for (auto itr = satused[i].begin(); itr != satused[i].end(); ++itr)
        {
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    
    ofstream out("SingleTCPFlowTable.sh");
    if (!out.is_open())
    {
        cout<<"Creating file fails"<<endl;
    }
    out<<"#!/bin/bash"<<endl<<endl;
    
    //generate s1-s5
    
    
    for(int i=0;i<nreq;i++)
    {
        int src=requests[i][0];
        int dst=requests[i][1];
        for(int j=0;j<n_sat;j++)
        {
            if(connectivity[src][j] && connectivity[dst][j])
            {
                cout<<"src:"<<src<<" dst:"<<dst<<" "<<j<<endl;
                satused[i].insert(j);
                reqused[j].insert(i);
            }
        }
    }
    
    //s1-s5
    for(int i=0;i<requests.size();i++)
    {
        int ship=requests[i][0];
        int dst=requests[i][1];
        string s="s"+to_string(ship+1);
        Request r(ship,-1,dst);
        for(int j=0;j<sh[ship].size();j++)
        {
            if(dst==sh[ship][j].dst){
                r=sh[ship][j];
            }
        }
        vector<unordered_set<int> > outportset(n_ship);
        int limit=1;
        int n=0;
        for (auto sat = satused[i].begin(); sat != satused[i].end()&& n<limit; ++sat,++n)
        {
            string inport=to_string(r.nodeid+1);
            int outport=hostnumber[ship]+1;
            for(int k=0;k<connectivity[ship].size();k++)
            {
                if(connectivity[ship][k]==1)
                {
                    if(k==*sat)
                        break;
                    else
                        outport++;
                }
            }
            string outp=to_string(outport);
            out<<"sudo ovs-ofctl add-flow "+s+" in_port="+inport+",actions=output:"+outp<<endl;
            
        }
    }
    
    out<<endl;
    for(int i=0;i<n_ship;i++){
        int nport=hostnumber[i];
        string s="s"+to_string(i+1);
        for(int j=0;j<n_sat;j++)
        {
            if(connectivity[i][j])
            {
                nport++;
                out<<"sudo ovs-ofctl add-flow "+s+" in_port="+to_string(nport)+",actions=normal"<<endl;
            }
        }
        out<<endl;
    }
    
    //s6-s8
    for(int i=0;i<n_sat;i++)
    {
        string outport=to_string(satconnect[i]+1);
        string s="s"+to_string(i+n_ship+1);
        for(int j=1;j<=satconnect[i];j++)
        {
            string inport="in_port="+to_string(j);
            out<<"sudo ovs-ofctl add-flow "+s+" "+inport+",actions="+"output:"+outport<<endl;
        }
        out<<"sudo ovs-ofctl add-flow "+s+" in_port="+outport+",actions=normal"<<endl;
        out<<endl;
    }
    
    //s9-s11
    for(int i=0;i<n_sat;i++)
    {
        string s="s"+to_string(i+n_ship+n_sat+1);
        out<<"sudo ovs-ofctl add-flow "+s+" "+"in_port=1"+",actions="+"output:2"<<endl;
        out<<"sudo ovs-ofctl add-flow "+s+" in_port=2,actions=normal"<<endl;
        out<<endl;
    }
    
    
    for(int i=0;i<n_sat;i++)
    {
        string s="s"+to_string(1+2*n_sat+n_ship+i);
        for (auto itr = reqused[i].begin(); itr != reqused[i].end(); ++itr)
        {
            int src=requests[*itr][0];
            int dst=requests[*itr][1];

            for(int z=0;z<sh[src].size();z++)
            {
                if(sh[src][z].dst==dst)
                {
                    string sid=sh[src][z].getIP(hostnumber);
                    int outport=2;
                    for(int j=0;j<n_ship;j++)
                    {
                        if(connectivity[j][i])
                        {
                            if(j==dst)
                                break;
                            else
                                outport++;
                        }
                    }
                    out<<"sudo ovs-ofctl add-flow "+s+" ip,nw_src="+sid+",actions=output:"+to_string(outport)<<endl;
                }
            }
         }
        int nport=1;
        for(int j=0;j<n_ship;j++)
        {
            if(connectivity[j][i])
            {
                nport++;
                out<<"sudo ovs-ofctl add-flow "+s+" in_port="+to_string(nport)+",actions=normal"<<endl;
            }
        }
         out<<endl;
    }
   
    
    for(int i=0;i<n_ship;i++)
    {
        int n=shipconnect[i];
        string s="s"+to_string(3*n_sat+n_ship+i+1);
        string outport=to_string(n+1);
        for(int j=1;j<=n;j++)
        {
            string inport=to_string(j);
            out<<"sudo ovs-ofctl add-flow "+s+" in_port="+inport+",actions=output:"+outport<<endl;
        }
        out<<"sudo ovs-ofctl add-flow "+s+" in_port="+outport+",actions=normal"<<endl;
        out<<endl;
    }
    
    for(int i=1;i<=3*n_sat+n_ship;i++)
    {
        string s="s"+to_string(i);
        out<<"sudo ovs-ofctl add-flow "+s+" priority=100,actions=normal"<<endl;
    }
    
    out.close();
    return 0;
}
