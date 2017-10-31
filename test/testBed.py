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
class Flow:
    def __init__(self):
        self.ip = []
        self.links = []
        self.rate = []
class Link:
    def __init__(self):
        self.src = []
        self.dst = []

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
    while line.strip() != "End of adding node":
        action, type_node, target = line.strip().split()
        if type_node == "host:":
            hosts.append(target)
            num_host += 1
        else:
            if type_node == "ship:":
                num_ship += 1
            elif type_node == "hub:":
                num_sat += 1
            switches.append(target)
        line = input.readline()

    # Add links
    line = input.readline()
    while line.strip() != "End of adding link":
        action, type_node, end1, end2 = line.strip().split()
        links.append([end1, end2])
        line = input.readline()

    # Routing table of hosts
    line = input.readline()
    while line.strip() != "End of configuring routing table of hosts":
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

            if int(index_switch) <= num_ship and "host" != end2[0:4]:
                print("index_switch: " + str(index_switch) + "   num_ship: "+ str(num_ship)+"   end1: "+ end1+ "  end2:  " + end2)
                # uplink to ship, need to configure both flowtable and queue
                for i in range(0, int(num_flow)):
                    ipaddr, rate = input.readline().strip().split()
                    commandQueue = "sudo ovs-vsctl -- set Port " + end1 + " qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:" + str(queue_num) + "=@q" + str(queue_num) + " -- --id=@q" + str(queue_num) + " create Queue other-config:min-rate=" + str(int(float(rate) * 1000000)) + " other-config:max-rate=" + str(int(float(rate) * 1000000))
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddr + "/32,actions=set_queue:" + str(queue_num) + ",output:" + index_intf
                    queueConfig.write(commandQueue + "\n")
                    flowTableConfig.write(commandFlowTable + "\n")
                    queue_num = queue_num + 1

                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
                flowTableConfig.write("sudo ovs-ofctl add-flow " + switch + " priority=100,actions=normal\n")

            elif index_switch <= num_ship + num_sat * 2:
                # sat-hub-dummy_sat
                for i in range(0, int(num_flow)):
                    ipaddr, rate = input.readline().strip().split()
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddr + "/32,actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                flowTableConfig.write(commandFlowTable + "\n")
                flowTableConfig.write("sudo ovs-ofctl add-flow " + switch + " priority=100,actions=normal\n")
            else:
                for i in range(0, int(num_flow)):
                    ipaddr, rate = input.readline().strip().split()
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,nw_src=" + ipaddr + "/32,actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=normal"
                    flowTableConfig.write(commandFlowTable + "\n")
                flowTableConfig.write("sudo ovs-ofctl add-flow " + switch + " priority=100,actions=normal\n")
        line = input.readline()
    # for l in input.readlines():
    #     link_num, end1, end2, usage = l.strip().split(",")
    #     end1 = int(end1)
    #     end2 = int(end2)
    #     usage = float(usage)
    #     if usage != 0 and (end1 < n or end1 >= n + m):
    #         flow = Flow()
    #         if end1 < n:
    #             if end1 >= len(linkConfig):
    #                 linkConfig.append([])
    #             linkConfig[end1].append(end2 - n)
    #             flow.src = 'h' + str(end1)
    #             flow.dst = 's' + str(end2 - n)
    #         if end1 >= n + m:
    #             flow.src = 'dummy_s' + str(end1 - n - m)
    #             flow.dst = 'h' + str(end2)
    #         flow.srceth = 'eth' + str(portCount[flow.src])
    #         flow.dsteth = 'eth' + str(portCount[flow.dst])
    #         flow.usage = usage
    #         flow.ip = "10.0." + str(end1) + '.' + str(portMap[flow.src])
    #         print("ip: " + flow.ip + " src: " + flow.src + " dst: " + flow.dst + " srceth:" + flow.srceth +  " dsteth:" + flow.dsteth +" usage:" + str(flow.usage))
    #         portCount[flow.src] += 1
    #         portCount[flow.dst] += 1
    #         if end1 < n:
    #             commandQueue = "sudo ovs-vsctl -- set Port " + flow.dst + "-" + flow.dsteth + " qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:" + str(queue_Num) + "=@q" + str(queue_Num) + " -- --id=@q" + str(queue_Num) + " create Queue other-config:min-rate=" + str(int(flow.usage * 1000000)) + " other-config:max-rate=" + str(int(flow.usage * 1000000))
    #             commandFlowTable = "sudo ovs-ofctl add-flow " + flow.dst + " ip,nw_src=" + flow.ip + "/32,actions=set_queue:" + str(queue_Num) + ",output:" + flow.dsteth[-1:]
    #         if end1 >= n + m:
    #             commandQueue = "sudo ovs-vsctl -- set Port " + flow.src + "-" + flow.srceth + " qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:" + str(queue_Num) + "=@q" + str(queue_Num) + " -- --id=@q" + str(queue_Num) + " create Queue other-config:min-rate=" + str(int(flow.usage * 1000000)) + " other-config:max-rate=" + str(int(flow.usage * 1000000))
    #             commandFlowTable = "sudo ovs-ofctl add-flow " + flow.src + " ip,nw_src=" + flow.ip + "/32,actions=set_queue:" + str(queue_Num) + ",output:" + flow.srceth[-1:]
    #
    #         queueConfig.write(commandQueue + "\n")
    #         flowTableConfig.write(commandFlowTable + "\n")
    #         if end1 < n:
    #             commandRouting = "ifconfig " + flow.src + "-" + flow.srceth + " " + flow.ip + " netmask 255.255.255.255"
    #             routingConfig[end1].write(commandRouting + "\n")
    #             commandRouting = "ip rule add from " + flow.ip + " table " + str(int(flow.srceth[-1:]) + 1)
    #             routingConfig[end1].write(commandRouting + "\n")
    #             commandRouting = "ip route add default via " + flow.ip + " dev " + flow.src + "-" + flow.srceth + " table " + str(int(flow.srceth[-1:]) + 1)
    #             routingConfig[end1].write(commandRouting + "\n")
    #             if flow.srceth == "eth0":
    #                 commandRouting = "ip route add default scope global nexthop via " + flow.ip + " dev " + flow.src + "-" + flow.srceth
    #                 routingConfig[end1].write(commandRouting + "\n")
    #         queue_Num += 1
    #         flows.append(flow)

    # for i in range(0, n):
    #     routingConfig[i].close()
    #     call(["sudo", "chmod", "777", hosts[i] + ".sh"])
    for i in range(0, num_ship):
        queueConfig.write("sudo ovs-ofctl -O Openflow13 queue-stats s" + str(i + 1) + "\n")

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
        name1, name2 = link[0], link[1]
        node1, node2 = nodes[name1], nodes[name2]
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
    call(["sudo", "bash","FDMQueueConfig.sh"])

    time.sleep(3)
    info('*** set flow tables ***\n')
    call(["sudo", "bash","FDMFlowTableConfig.sh"])

    # for i in range(0,n):
    #     src=nodes[hosts[i]]
    #     for j in linkConfig[i]:
    #         dst = nodes[hosts[j]]
    #         info("testing",src.name,"<->",dst.name,'\n')
    #         src.cmdPrint('ping -c 2 10.0.' + str(j) + '.0' + ' &')
    #         time.sleep(0.2)
    #         src.cmdPrint('iperf -c 10.0.' + str(j) + '.0' + ' -t 60 -i 2 &')
    #         time.sleep(0.2)
    #
    # time.sleep(5)
    #
    CLI(net)

    net.stop()
    info('*** net.stop()\n')


if __name__ == '__main__':
    # setLogLevel('info')
    WifiNet("allocation.txt")
    #WifiNet(30,3,"127.0.0.1",[2.0]*30,[30,20,30],120)
