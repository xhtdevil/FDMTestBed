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

    # disable mptcp
    call(["sudo", "sysctl","-w","net.mptcp.mptcp_enabled=0"])

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

    # Read src-des pair for testing
    src_hosts = []
    des_hosts = []
    des_ip = []
    line = input.readline()
    while line.strip() != "End":
        src, des, ip = line.strip().split()
        src_hosts.append(src)
        des_hosts.append(des)
        des_ip.append(ip)
        line = input.readline()


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
    lastline=""
    while line.strip()!="End":
    	line.strip()
    	if line!=lastline:
        	action, type_node, end1, end2, bw, delay = line.strip().split()
        	if end1[0] == "s" and int(end1[1:]) <= num_ship and end2[0] == "s":
            		max_outgoing[int(end1[1:]) - 1] += 1
        	links.append([end1, end2, bw, delay])
        lastline=line
        line= input.readline()
    input.close()
    print(max_outgoing)

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

    """ Add links """
    for link in links:
        name1, name2, b, d = link[0], link[1], link[2], link[3]
        node1, node2 = nodes[name1], nodes[name2]
        if(b!='inf' and d != '0'):
            net.addLink(node1, node2, bw=float(b), delay=d+'ms')
        elif(b!='inf'):
            net.addLink(node1, node2,bw=float(b))
        elif(d!='0'):
            net.addLink(node1, node2, delay=d+'ms')
        else:
            net.addLink(node1,node2)

    """ Start the simulation """
    info('*** Starting network ***\n')
    net.start()

    time.sleep(3)
    info('*** set flow tables ***\n')
    call(["sudo","bash","SingleTCPFlowTable.sh"])

    for i in range(0,5):
        for i in [2,5,8,11,14]:
            srv = nodes[hosts[i]]
            info("starting D-ITG servers...\n")
            srv.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
            srv.cmdPrint("./ITGRecv &")
            srv.cmdPrint("PID=$!")

        time.sleep(1)
        sTime = 10000# default 120,000ms
        for i in range(0,10):
            # normal requirement
            senderList = [0,1,3,4,6,7,9,10,12,13]
            recvList = [11,14,2,8,5,11,5,8,2,11]
            bwReq = [6,4,7,3,4,4,3,3,3,3]

            # large requirement
            #bwReq = [2,12,3,3,5,5,12,2,12,2]
            ITGTest(senderList[i], recvList[i], hosts, nodes, bwReq[i]*125, sTime)
            time.sleep(0.2)
        info("running simulaiton...\n")
        info("please wait...\n")

        time.sleep(sTime/1000+10)
        for i in [2,5,8,11,14]:
            srv=nodes[hosts[i]]
            info("killing D-ITG servers...\n")

            srv.cmdPrint("kill $PID")

        # You need to change the path here
        call(["sudo", "python","analysis.py"])
    # CLI(net)

    net.stop()
    info('*** net.stop()\n')

    # enable mptcp
    call(["sudo", "sysctl","-w","net.mptcp.mptcp_enabled=1"])

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
    WifiNet("all_5.txt")
