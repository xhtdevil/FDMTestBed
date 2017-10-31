#!/bin/bash

ifconfig host0-eth0 10.0.0.0 netmask 255.255.255.255
ip rule add from 10.0.0.0 table 1
ip route add 10.0.0.0/32 dev host0-eth0 scope link table 1
ip route add default via 10.0.0.0 dev host0-eth0 table 1
ip route add default scope global nexthop via 10.0.0.0 dev host0-eth0
ifconfig host0-eth1 10.0.0.2 netmask 255.255.255.255
ip rule add from 10.0.0.2 table 2
ip route add 10.0.0.2/32 dev host0-eth1 scope link table 2
ip route add default via 10.0.0.2 dev host0-eth1 table 2
ifconfig host0-eth2 10.0.0.1 netmask 255.255.255.255
ip rule add from 10.0.0.1 table 3
ip route add 10.0.0.1/32 dev host0-eth2 scope link table 3
ip route add default via 10.0.0.1 dev host0-eth2 table 3
