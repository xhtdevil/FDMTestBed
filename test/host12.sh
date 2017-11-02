#!/bin/bash

ifconfig host12-eth0 10.0.12.1 netmask 255.255.255.255
ip rule add from 10.0.12.1 table 1
ip route add 10.0.12.1/32 dev host12-eth0 scope link table 1
ip route add default via 10.0.12.1 dev host12-eth0 table 1
ip route add default scope global nexthop via 10.0.12.1 dev host12-eth0
ifconfig host12-eth1 10.0.12.0 netmask 255.255.255.255
ip rule add from 10.0.12.0 table 2
ip route add 10.0.12.0/32 dev host12-eth1 scope link table 2
ip route add default via 10.0.12.0 dev host12-eth1 table 2
