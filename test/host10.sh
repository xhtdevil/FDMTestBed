#!/bin/bash

ifconfig host10-eth0 10.0.10.2 netmask 255.255.255.255
ip rule add from 10.0.10.2 table 1
ip route add 10.0.10.2/32 dev host10-eth0 scope link table 1
ip route add default via 10.0.10.2 dev host10-eth0 table 1
ip route add default scope global nexthop via 10.0.10.2 dev host10-eth0
ifconfig host10-eth1 10.0.10.1 netmask 255.255.255.255
ip rule add from 10.0.10.1 table 2
ip route add 10.0.10.1/32 dev host10-eth1 scope link table 2
ip route add default via 10.0.10.1 dev host10-eth1 table 2
