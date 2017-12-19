#!/bin/bash

ifconfig host6-eth0 10.0.7.1 netmask 255.255.255.255
ip rule add from 10.0.7.1 table 1
ip route add 10.0.7.1/32 dev host6-eth0 scope link table 1
ip route add default via 10.0.7.1 dev host6-eth0 table 1
ip route add default scope global nexthop via 10.0.7.1 dev host6-eth0
ifconfig host6-eth1 10.0.7.0 netmask 255.255.255.255
ip rule add from 10.0.7.0 table 2
ip route add 10.0.7.0/32 dev host6-eth1 scope link table 2
ip route add default via 10.0.7.0 dev host6-eth1 table 2
