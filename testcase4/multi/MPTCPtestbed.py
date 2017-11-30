#!/usr/bin/python

from subprocess import call, check_call, check_output
from mininet.net import Mininet
from mininet.node import Node, OVSKernelSwitch, Host, RemoteController, UserSwitch, Controller
from mininet.link import Link, Intf, TCLink, TCULink
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from functools import partial
import sys, time
flush=sys.stdout.flush
import os.path, string

def WifiNet(inputFile):

    # enable mptcp
    call(["sudo", "sysctl","-w","net.mptcp.mptcp_enabled=1"])

    input = open(inputFile, "r")
    """ Node names """
    max_outgoing = []
    hosts = []
    switches = []  # satellite and dummy satellite
    links = []
    sw_name = []
    portCount = {} # count the current eth usage of a host/switch
    linkToPort = {} # map link to certain ethernets of src and dst

    flows = []
    routingConfig = []
    queue_num = 1
    num_host = 0
    num_ship = 0
    num_sat = 0

    line = input.readline()
    # Add nodes
    while line.strip() != "End":
        action, type_node, target = line.strip().split()
        if type_node == "host:":
            hosts.append(target)
            num_host += 1
        else:
            if type_node == "ship:":
                num_ship += 1
                max_outgoing.append(0)
            elif type_node == "hub:":
                num_sat += 1
            switches.append(target)
        line = input.readline()

    # Add links
    line = input.readline()
    while line.strip() != "End":
        action, type_node, end1, end2 = line.strip().split()
        if end1[0] == "s" and int(end1[1:]) <= num_ship and end2[0] == "s":
            max_outgoing[int(end1[1:]) - 1] += 1
        links.append([end1, end2])
        line = input.readline()


    print(max_outgoing)

    # Routing table of hosts
    line = input.readline()
    while line.strip() != "End":
        host, st, num_ip = line.strip().split()
        file = open(host + ".sh", "w")
        file.write("#!/bin/bash\n\n")
        for i in range(0, int(num_ip)):
            ipaddr = input.readline().strip()
            intf = host + "-eth" + str(i)
            file.write("ifconfig " + intf + " " + ipaddr + " netmask 255.255.255.255\n")
            file.write("ip rule add from " + ipaddr + " table " + str(i + 1) + "\n")
            file.write("ip route add " + ipaddr + "/32 dev " + intf + " scope link table " + str(i + 1) + "\n")
            file.write("ip route add default via " + ipaddr + " dev " + intf + " table " + str(i + 1) + "\n")
            if i == 0:
                file.write("ip route add default scope global nexthop via " + ipaddr + " dev " + intf + "\n")
        file.close()
        call(["sudo", "chmod", "777", host + ".sh"])
        line = input.readline()

    input.close()
    net = Mininet(link=TCLink, controller=None, autoSetMacs = True)

    nodes = {}

    """ Initialize Ships """
    for host in hosts:
        node = net.addHost(host)
        nodes[host] = node

    """ Initialize SATCOMs """
    for switch in switches:
        node = net.addSwitch(switch)
        nodes[switch] = node
    print(links)
    """ Add links """
    for link in links:
        name1, name2 = link[0], link[1]
        node1, node2 = nodes[name1], nodes[name2]
        if(name1 == 's6' and name2 == 's9'):
            net.addLink(node1, node2, bw = 30)
            info('set *************************')
        elif(name1 == 's7' and name2 == 's10'):
            net.addLink(node1, node2, bw = 20)
        elif (name1 == 's8' and name2 == 's11'):
            net.addLink(node1, node2, bw = 15)
        else:
            net.addLink(node1, node2)

    """ Start the simulation """
    info('*** Starting network ***\n')
    net.start()

    #  set all ships
    for i in range(0,num_host):
        src=nodes[hosts[i]]
        info("--configing routing table of "+hosts[i])
        if os.path.isfile(hosts[i]+'.sh'):
            src.cmdPrint('./'+hosts[i]+'.sh')


    info('*** start test ***\n')


    time.sleep(3)
    info('*** set flow tables ***\n')
    call(["sudo","bash","MPTCPFlowTable.sh"])

    # start D-ITG Servers

    for i in [2,5,8,11,14]:
        srv = nodes[hosts[i]]
        info("starting D-ITG servers...\n")
        srv.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
        srv.cmdPrint("./ITGRecv &")

    time.sleep(1)

    # start D-ITG application
    # set simulation time

    sTime = 60000# default 120,000ms
    for i in range(0,10):
        # normal requirement
        senderList = [0,1,3,4,6,7,9,10,12,13]
        recvList = [11,14,2,8,5,11,5,8,2,11]
        # bwReq = [6,4,7,3,4,4,3,3,3,3]

        # large requirement
        bwReq = [2,12,3,3,5,5,12,2,12,2]
        ITGTest(senderList[i], recvList[i], hosts, nodes, bwReq[i]*125, sTime)
        time.sleep(0.2)

    info("running simulaiton...\n")
    info("please wait...\n")

    time.sleep(sTime/1000)

    # You need to change the path here
    call(["sudo", "python","../analysis/analysis.py"])
    # CLI(net)

    net.stop()
    info('*** net.stop()\n')

def iperfTest(srcNo, dstNo, hosts,nodes):
    src = nodes[hosts[srcNo]]
    dst = nodes[hosts[dstNo]]
    info("iperfing",src.name,"<->",dst.name,"...",'\n')
    src.cmdPrint("iperf -c 10.0.0." + str(dstNo + 1) + " -t 3600 -i 2 &")
    time.sleep(0.2)
    #src.cmdPrint("sudo wireshark &")

def ITGTest(srcNo, dstNo, hosts, nodes, bw, sTime):
    src = nodes[hosts[srcNo]]
    dst = nodes[hosts[dstNo]]
    info("Sending message from ",src.name,"<->",dst.name,"...",'\n')
    src.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
    src.cmdPrint("./ITGSend -T TCP  -a 10.0.0."+str(dstNo+1)+" -c 1000 -C "+str(bw)+" -t "+str(sTime)+" -l sender"+str(srcNo)+".log -x receiver"+str(srcNo)+"ss"+str(dstNo)+".log &")

if __name__ == '__main__':
    setLogLevel('info')
    WifiNet("allocation.txt")
