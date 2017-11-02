#!/bin/bash

ifconfig host4-eth0 10.0.4.2 netmask 255.255.255.255
ip rule add from 10.0.4.2 table 1
ip route add 10.0.4.2/32 dev host4-eth0 scope link table 1
ip route add default via 10.0.4.2 dev host4-eth0 table 1
ip route add default scope global nexthop via 10.0.4.2 dev host4-eth0
ifconfig host4-eth1 10.0.4.1 netmask 255.255.255.255
ip rule add from 10.0.4.1 table 2
ip route add 10.0.4.1/32 dev host4-eth1 scope link table 2
ip route add default via 10.0.4.1 dev host4-eth1 table 2
