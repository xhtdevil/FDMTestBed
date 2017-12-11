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

# Flow: IP, Links, rate
# Link: src, dst, srceth, dsteth,
# class Flow:
#     def __init__(self):
#         self.ip = []
#         self.links = []
#         self.rate = []
# class Link:
#     def __init__(self):
#         self.src = []
#         self.dst = []

# class cd:
#     """Context manager for changing the current working directory"""
#     def __init__(self, newPath):
#         self.newPath = os.path.expanduser(newPath)
#
#     def _x_enter__(self):
#         self.savedPath = os.getcwd()
#         os.chdir(self.newPath)
#
#     def __exit__(self, etype, value, traceback):
#         os.chdir(self.savedPath)


def WifiNet(inputFile):
    input = open(inputFile, "r")
    # l = input.readline()
    # linkConfig = []
    # arg = l.strip().split(",")
    # n = int(arg[0])
    # m = int(arg[1])
    """ Node names """
    max_outgoing = []
    hosts = []
    switches = []  # satellite and dummy satellite
    links = []
    sw_name = []
    portCount = {} # count the current eth usage of a host/switch
    linkToPort = {} # map link to certain ethernets of src and dst

    # for i in range(1, 16):
    #     sw_name.append('0'+hex(i)[-1])
    # for i in range(16, 60):
    #     sw_name.append(hex(i)[-2:])
    # for i in range(0, n):
    #     hosts.append('h' + str(i))
    #     portCount['h' + str(i)] = 0
    # for i in range(0, m):  # satellite
    #     switches.append('s'+str(i))
    #     portCount['s' + str(i)] = 1
    # for i in range(0, m):  # dummy satellite
    #     switches.append('dummy_s'+str(i))
    #     portCount['dummy_s' + str(i)] = 2  # one port for satellite-dummy_satellite
    # for i in range(0, m):  # hubs between uplink & downlink
    #     switches.append('hub' + str(i))


    flows = []
    routingConfig = []
    # for i in range(0, n):
    #     file = open("h" + str(i) + ".sh", "w")
    #     file.write("#!/bin/bash\n\n")
    #     routingConfig.append(file)
    queueConfig = open("FDMQueueConfig.sh", "w")
    flowTableConfig = open("FDMFlowTableConfig.sh", "w")
    queueConfig.write("#!/bin/bash\n\n")
    flowTableConfig.write("#!/bin/bash\n\n")
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

    # Flow table and queue
    queue_num = 1
    line = input.readline()
    while line:
        # print(line)
        end1, end2, num_flow = line.strip().split()
        num_flow = num_flow.strip().split(":")[1]


        # Routing tables have been configured
        if "host" in end1:
            for i in range(0, int(num_flow)):
                line = input.readline()
        else:
            switch, intf = end1.split("-")
            index_switch = int(switch[1:])
            index_intf = intf[3:]

            if index_switch <= num_ship and "host" != end2[0:4]:

                # uplink to ship, need to configure both flowtable and queue
                commandQueue = "sudo ifconfig " + end1 + " txqueuelen 1"
                queueConfig.write(commandQueue + "\n")

                for i in range(0, int(num_flow)):
                    ipaddr, rate = input.readline().strip().split()
                    commandQueue = "sudo ovs-vsctl -- set Port " + end1 + " qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:" + str(queue_num) + "=@q" + str(queue_num) + " -- --id=@q" + str(queue_num) + " create Queue other-config:min-rate=" + str(int(float(rate) * 1000000)) + " other-config:max-rate=" + str(int(float(rate) * 1000000))
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddr + "/32,actions=set_queue:" + str(queue_num) + ",output:" + index_intf
                    queueConfig.write(commandQueue + "\n")
                    flowTableConfig.write(commandFlowTable + "\n")
                    queue_num = queue_num + 1

                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
            elif index_switch <= num_ship:
                # ship to host downlink
                # port forwarding
                for i in range(0, int(num_flow)):
                    input.readline()
                for i in range(0, int(max_outgoing[index_switch - 1])):
                    # ipaddr, rate = input.readline().strip().split()
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + str(int(index_intf) - i - 1) + ",actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")

            elif index_switch <= num_ship + num_sat * 2:
                # sat-hub-dummy_sat
                # port forwarding
                for i in range(0, int(num_flow)):
                    input.readline()
                for i in range(1, int(index_intf)):
                    # ipaddr, rate = input.readline().strip().split()
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + str(i) + ",actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
            else:
                # dummy to ship, ip forwarding
                for i in range(0, int(num_flow)):
                    ipaddr, rate = input.readline().strip().split()
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddr + "/32,actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
        line = input.readline()

    for i in range(0, num_ship):
        queueConfig.write("sudo ovs-ofctl -O Openflow13 queue-stats s" + str(i + 1) + "\n")

    for i in range(0, num_ship + 3 * num_sat):
        flowTableConfig.write("sudo ovs-ofctl add-flow s" + str(i + 1) + " priority=100,actions=normal\n")

    flowTableConfig.close()
    queueConfig.close()

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
    info('*** set queues ***\n')
    #call(["sudo", "bash","FDMQueueConfig.sh"])
    call(["sudo", "bash","testcase_4_ft.sh"])
    time.sleep(3)
    info('*** set flow tables ***\n')
    #call(["sudo", "bash","FDMFlowTableConfig.sh"])
    call(["sudo","bash","testcase_4_q.sh"])

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
        senderList = [0,1,3,4,6,7,9,10,12,13]
        recvList = [11,14,2,8,5,11,5,8,2,11]
        bwReq = [6,4,7,3,4,4,3,3,3,3]
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
    WifiNet("allocation_legacy.txt")
    #WifiNet(30,3,"127.0.0.1",[2.0]*30,[30,20,30],120)
