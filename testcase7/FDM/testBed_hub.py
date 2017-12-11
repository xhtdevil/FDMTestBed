#!/usr/bin/python

from subprocess import call, check_call, check_output
from mininet.net import Mininet
from mininet.node import Node, OVSKernelSwitch, Host, RemoteController, UserSwitch, Controller
from mininet.link import Link, Intf, TCLink, TCULink
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from functools import partial
from threading import Thread
import sys, time
flush=sys.stdout.flush
import os.path, string

class ServerSetup(Thread):
    def __init__(self, node, name):
        "Constructor"
        Thread.__init__(self)
        self.node = node
        self.name = name

    def run(self):
        print("Server set up for iperf " + self.name)
        self.node.cmdPrint("iperf -s")

    def close(self, i):
        self.running = False
        print("exit " + self.name)

def WifiNet(inputFile):
    input = open(inputFile, "r")
    """ Node names """
    max_outgoing = []
    hosts = []
    switches = []  # satellite and dummy satellite
    links = []
    requirements = []

    queueConfig = open("FDMQueueConfig.sh", "w")
    flowTableConfig = open("FDMFlowTableConfig.sh", "w")
    queueConfig.write("#!/bin/bash\n\n")
    flowTableConfig.write("#!/bin/bash\n\n")
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

    # Add nodes
    # Mirror ships are switches
    line = input.readline()
    while line.strip() != "End":
        action, type_node, target = line.strip().split()
        if type_node == "host:":
            hosts.append(target)
            num_host += 1
        else:
            if type_node == "ship:":
                num_ship += 1
                max_outgoing.append(0)
            elif type_node == "sat:":
                num_sat += 1
            switches.append(target)
        line = input.readline()
    num_sat = num_sat / 2

    # Add links
    line = input.readline()
    while line.strip() != "End":
        action, type_node, end1, end2, bw, delay = line.strip().split()
        if end1[0] == "s" and int(end1[1:]) <= num_ship and end2[0] == "s":
            max_outgoing[int(end1[1:]) - 1] += 1
        links.append([end1, end2, bw, delay])
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
            requirement = 0
            for i in range(0, int(num_flow)):
                ipaddr, rate = input.readline().strip().split()
                requirement += float(rate)
            requirements.append(requirement)
        else:
            switch, intf = end1.split("-")
            index_switch = int(switch[1:])
            index_intf = intf[3:]

            # desired requirements of infeasible testcase

            if index_switch <= num_ship and "host" != end2[0:4]:
                # uplink to ship, need to configure both flowtable and queue
                # put the queues for one port on one line in definition
                commandQueue = "sudo ifconfig " + end1 + " txqueuelen 50"
                queueConfig.write(commandQueue + "\n")

                commandQueue = "sudo ovs-vsctl -- set Port " + end1 + " qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 "
                queue_nums = []
                rates = []
                ipaddrs = []
                for i in range(0, int(num_flow) - 1):
                    ipaddr, rate = input.readline().strip().split()
                    rates.append(rate)
                    ipaddrs.append(ipaddr)
                    queue_nums.append(queue_num)
                    commandQueue += "queues:" + str(queue_num) + "=@q" + str(queue_num) + " "
                    queue_num += 1
                ipaddr, rate = input.readline().strip().split()
                rates.append(rate)
                ipaddrs.append(ipaddr)
                queue_nums.append(queue_num)
                commandQueue += "queues:" + str(queue_num) + "=@q" + str(queue_num) + " -- "
                queue_num += 1

                for i in range(0, int(num_flow) - 1):
                    commandQueue += " --id=@q" + str(queue_nums[i]) + " create Queue other-config:min-rate=" + str(int(float(rates[i]) * 1000000)) + " other-config:max-rate=" + str(int(float(rates[i]) * 1000000)) + " -- "
                commandQueue += " --id=@q" + str(queue_nums[len(queue_nums) - 1]) + " create Queue other-config:min-rate=" + str(int(float(rates[len(queue_nums) - 1]) * 1000000)) + " other-config:max-rate=" + str(int(float(rates[len(queue_nums) - 1]) * 1000000))
                queueConfig.write(commandQueue + "\n")

                for i in range(0, int(num_flow)):
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddrs[i] + "/32,actions=set_queue:" + str(queue_nums[i]) + ",output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")

                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
            # elif index_switch <= num_ship:
            #     # ship to host downlink
            #     # port forwarding
            #     for i in range(0, int(num_flow)):
            #         input.readline()
            #     for i in range(0, int(max_outgoing[index_switch - 1])):
            #         # ipaddr, rate = input.readline().strip().split()
            #         commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + str(int(index_intf) - i - 1) + ",actions=output:" + index_intf
            #         flowTableConfig.write(commandFlowTable + "\n")
            #     commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
            #     flowTableConfig.write(commandFlowTable + "\n")

            else:
                # sat-dummy_sat
                # port forwarding
                for i in range(0, int(num_flow)):
                    input.readline()
                for i in range(1, int(index_intf)):
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + str(i) + ",actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
            # elif index_switch <= num_ship + num_sat * 3:
            #     # dummy to mirror ship, ip forwarding
            #     for i in range(0, int(num_flow)):
            #         ipaddr, rate = input.readline().strip().split()
            #         commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddr + "/32,actions=output:" + index_intf
            #         flowTableConfig.write(commandFlowTable + "\n")
            #     commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
            #     flowTableConfig.write(commandFlowTable + "\n")
            # else:
            #     # mirror ship to host, port forwarding
            #     for i in range(0, int(num_flow)):
            #         input.readline()
            #     for i in range(1, int(index_intf)):
            #         commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + str(i) + ",actions=output:" + index_intf
            #         flowTableConfig.write(commandFlowTable + "\n")
            #     commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
            #     flowTableConfig.write(commandFlowTable + "\n")
        line = input.readline()
    for i in range(0, num_ship):
        queueConfig.write("sudo ovs-ofctl -O Openflow13 queue-stats s" + str(i + 1) + "\n")

    for i in range(0, num_ship + 2 * num_sat + 1):
        flowTableConfig.write("sudo ovs-ofctl add-flow s" + str(i + 1) + " priority=100,actions=normal\n")

    flowTableConfig.close()
    queueConfig.close()
    call(["sudo", "chmod", "777", "FDMQueueConfig.sh"])
    call(["sudo", "chmod", "777", "FDMFlowTableConfig.sh"])

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
        if(d != '0'):
            net.addLink(node1, node2,  delay=d+'ms')
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

    time.sleep(0.3)
    info('*** set queues ***\n')
    call(["sudo", "bash","FDMQueueConfig.sh"])

    time.sleep(0.3)
    info('*** set flow tables ***\n')
    call(["sudo", "bash","FDMFlowTableConfig.sh"])

    # time.sleep(3)
    # info('*** set flow tables ***\n')
    # call(["sudo", "bash","MPTCPFlowTable.sh"])

    # start D-ITG Servers
    for i in [num_host - 1]: # last host is receiver
        srv = nodes[hosts[i]]
        info("starting D-ITG servers...\n")
        srv.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
        srv.cmdPrint("./ITGRecv &")

    time.sleep(1)

    # start D-ITG application
    # set simulation time
    sTime = 30000  # default 120,000ms
    # bwReq = [12,12,12,12,12]
    # bwReq = [18,18,15,12,12]
    bwReq = requirements
    print(bwReq)
    for i in range(0, num_host - 1):
        sender = i
        receiver = num_host - 1
        ITGTest(sender, receiver, hosts, nodes, bwReq[i]*125, sTime)
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
    testTimes = 1
    for i in range(0, testTimes):
        WifiNet("all_7.txt")
