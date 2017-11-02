#!/bin/bash

ifconfig host3-eth0 10.0.3.1 netmask 255.255.255.255
ip rule add from 10.0.3.1 table 1
ip route add 10.0.3.1/32 dev host3-eth0 scope link table 1
ip route add default via 10.0.3.1 dev host3-eth0 table 1
ip route add default scope global nexthop via 10.0.3.1 dev host3-eth0
ifconfig host3-eth1 10.0.3.2 netmask 255.255.255.255
ip rule add from 10.0.3.2 table 2
ip route add 10.0.3.2/32 dev host3-eth1 scope link table 2
ip route add default via 10.0.3.2 dev host3-eth1 table 2
ifconfig host3-eth2 10.0.3.0 netmask 255.255.255.255
ip rule add from 10.0.3.0 table 3
ip route add 10.0.3.0/32 dev host3-eth2 scope link table 3
ip route add default via 10.0.3.0 dev host3-eth2 table 3
