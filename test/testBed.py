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
    input = open(inputFile, "r")
    """ Node names """
    max_outgoing = []
    hosts = []
    switches = []  # satellite and dummy satellite
    links = []

    flows = []
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

    num_src = num_host / num_ship - 1
    # Add links
    line = input.readline()
    while line.strip() != "End":
        action, type_node, end1, end2 = line.strip().split()
        if end1[0] == "s" and int(end1[1:]) <= num_ship and end2[0] == "s":
            max_outgoing[int(end1[1:]) - 1] += 1
        links.append([end1, end2])
        line = input.readline()

    # connection numbers of each ship
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
    num_input_from_host = []
    for i in range(0, len(max_outgoing)):
        output_num = ""
        for j in range(1, num_src * max_outgoing[i]):
            output_num = output_num + str(j) + ","
        output_num = output_num + str(num_src * max_outgoing[i])
        num_input_from_host.append(output_num)

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

            if index_switch <= num_ship and "host" != end2[0:4]:
                # uplink to ship, need to configure both flowtable and queue
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
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " arp,nw_src=" + ipaddrs[i] + "/32,actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=output:" + num_input_from_host[index_switch - 1]
                flowTableConfig.write(commandFlowTable + "\n")

            elif index_switch <= num_ship:
                # ship to host downlink
                # port forwarding
                for i in range(0, int(num_flow)):
                    input.readline()
                total_input = ""
                for i in range(0, int(max_outgoing[index_switch - 1])):
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + str(int(index_intf) - i - 1) + ",actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                    if i != int(max_outgoing[index_switch - 1]) - 1:
                        total_input = total_input + str(int(index_intf) - i - 1) + ","
                    else :
                        total_input = total_input + str(int(index_intf) - i - 1)
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " in_port=" + index_intf + ",actions=" + total_input
                flowTableConfig.write(commandFlowTable + "\n")

            elif index_switch <= num_ship + num_sat * 2:
                # sat-hub-dummy_sat
                # port forwarding
                for i in range(0, int(num_flow)):
                    input.readline()
                for i in range(1, int(index_intf)):
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
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " ip,in_port="  + index_intf + ",nw_dst=" + ipaddr + "/32,actions=output:1"
                    flowTableConfig.write(commandFlowTable + "\n")
                    commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " arp,nw_src=" + ipaddr + "/32,actions=output:" + index_intf
                    flowTableConfig.write(commandFlowTable + "\n")
                commandFlowTable = "sudo ovs-ofctl add-flow " + switch + " arp,in_port=" + index_intf + ",actions=output:1"
                flowTableConfig.write(commandFlowTable + "\n")

        line = input.readline()

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

    time.sleep(3)

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
    time.sleep(5)

    CLI(net)

    net.stop()
    info('*** net.stop()\n')


if __name__ == '__main__':
    setLogLevel('info')
    WifiNet("allocation_old.txt")
