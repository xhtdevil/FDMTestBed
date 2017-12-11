#!/bin/bash

sudo ifconfig s1-eth5 txqueuelen 100
sudo ovs-vsctl -- set Port s1-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:1=@q1 queues:2=@q2 --  --id=@q1 create Queue other-config:min-rate=5117530 other-config:max-rate=5117530 --  --id=@q2 create Queue other-config:min-rate=2000000 other-config:max-rate=2000000
sudo ifconfig s1-eth6 txqueuelen 100
sudo ovs-vsctl -- set Port s1-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:3=@q3 --  --id=@q3 create Queue other-config:min-rate=6882470 other-config:max-rate=6882470
sudo ifconfig s2-eth5 txqueuelen 100
sudo ovs-vsctl -- set Port s2-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:4=@q4 --  --id=@q4 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ifconfig s2-eth6 txqueuelen 100
sudo ovs-vsctl -- set Port s2-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:5=@q5 --  --id=@q5 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ifconfig s3-eth3 txqueuelen 100
sudo ovs-vsctl -- set Port s3-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:6=@q6 queues:7=@q7 --  --id=@q6 create Queue other-config:min-rate=5000000 other-config:max-rate=5000000 --  --id=@q7 create Queue other-config:min-rate=5000000 other-config:max-rate=5000000
sudo ifconfig s4-eth5 txqueuelen 100
sudo ovs-vsctl -- set Port s4-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:8=@q8 --  --id=@q8 create Queue other-config:min-rate=9440440 other-config:max-rate=9440440
sudo ifconfig s4-eth6 txqueuelen 100
sudo ovs-vsctl -- set Port s4-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:9=@q9 queues:10=@q10 --  --id=@q9 create Queue other-config:min-rate=2559560 other-config:max-rate=2559560 --  --id=@q10 create Queue other-config:min-rate=2000000 other-config:max-rate=2000000
sudo ifconfig s5-eth5 txqueuelen 100
sudo ovs-vsctl -- set Port s5-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:11=@q11 queues:12=@q12 --  --id=@q11 create Queue other-config:min-rate=2000000 other-config:max-rate=2000000 --  --id=@q12 create Queue other-config:min-rate=5486940 other-config:max-rate=5486940
sudo ifconfig s5-eth6 txqueuelen 100
sudo ovs-vsctl -- set Port s5-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:13=@q13 --  --id=@q13 create Queue other-config:min-rate=6513060 other-config:max-rate=6513060
sudo ovs-ofctl -O Openflow13 queue-stats s1
sudo ovs-ofctl -O Openflow13 queue-stats s2
sudo ovs-ofctl -O Openflow13 queue-stats s3
sudo ovs-ofctl -O Openflow13 queue-stats s4
sudo ovs-ofctl -O Openflow13 queue-stats s5
