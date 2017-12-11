#!/bin/bash

sudo ifconfig s1-eth3 txqueuelen 100
sudo ovs-vsctl -- set Port s1-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:1=@q1 --  --id=@q1 create Queue other-config:min-rate=5480300 other-config:max-rate=5480300
sudo ifconfig s1-eth4 txqueuelen 100
sudo ovs-vsctl -- set Port s1-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:2=@q2 --  --id=@q2 create Queue other-config:min-rate=4519700 other-config:max-rate=4519700
sudo ifconfig s2-eth3 txqueuelen 100
sudo ovs-vsctl -- set Port s2-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:3=@q3 --  --id=@q3 create Queue other-config:min-rate=6390100 other-config:max-rate=6390100
sudo ifconfig s2-eth4 txqueuelen 100
sudo ovs-vsctl -- set Port s2-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:4=@q4 --  --id=@q4 create Queue other-config:min-rate=3609900 other-config:max-rate=3609900
sudo ifconfig s3-eth2 txqueuelen 100
sudo ovs-vsctl -- set Port s3-eth2 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:5=@q5 --  --id=@q5 create Queue other-config:min-rate=8000000 other-config:max-rate=8000000
sudo ifconfig s4-eth3 txqueuelen 100
sudo ovs-vsctl -- set Port s4-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:6=@q6 --  --id=@q6 create Queue other-config:min-rate=3888000 other-config:max-rate=3888000
sudo ifconfig s4-eth4 txqueuelen 100
sudo ovs-vsctl -- set Port s4-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:7=@q7 --  --id=@q7 create Queue other-config:min-rate=2112000 other-config:max-rate=2112000
sudo ifconfig s5-eth3 txqueuelen 100
sudo ovs-vsctl -- set Port s5-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:8=@q8 --  --id=@q8 create Queue other-config:min-rate=2866720 other-config:max-rate=2866720
sudo ifconfig s5-eth4 txqueuelen 100
sudo ovs-vsctl -- set Port s5-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:9=@q9 --  --id=@q9 create Queue other-config:min-rate=3133280 other-config:max-rate=3133280
sudo ovs-ofctl -O Openflow13 queue-stats s1
sudo ovs-ofctl -O Openflow13 queue-stats s2
sudo ovs-ofctl -O Openflow13 queue-stats s3
sudo ovs-ofctl -O Openflow13 queue-stats s4
sudo ovs-ofctl -O Openflow13 queue-stats s5
