#!/bin/bash

sudo ovs-vsctl -- set Port s1-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:1=@q1 --  --id=@q1 create Queue other-config:min-rate=6000000 other-config:max-rate=6000000
sudo ovs-vsctl -- set Port s1-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:2=@q2 --  --id=@q2 create Queue other-config:min-rate=4000000 other-config:max-rate=4000000
sudo ovs-vsctl -- set Port s2-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:3=@q3 --  --id=@q3 create Queue other-config:min-rate=7000000 other-config:max-rate=7000000
sudo ovs-vsctl -- set Port s2-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:4=@q4 --  --id=@q4 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ovs-vsctl -- set Port s3-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:5=@q5 queues:6=@q6 --  --id=@q5 create Queue other-config:min-rate=4000000 other-config:max-rate=4000000 --  --id=@q6 create Queue other-config:min-rate=4000000 other-config:max-rate=4000000
sudo ovs-vsctl -- set Port s4-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:7=@q7 --  --id=@q7 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ovs-vsctl -- set Port s4-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:8=@q8 --  --id=@q8 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ovs-vsctl -- set Port s5-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:9=@q9 --  --id=@q9 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ovs-vsctl -- set Port s5-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:10=@q10 --  --id=@q10 create Queue other-config:min-rate=3000000 other-config:max-rate=3000000
sudo ovs-ofctl -O Openflow13 queue-stats s1
sudo ovs-ofctl -O Openflow13 queue-stats s2
sudo ovs-ofctl -O Openflow13 queue-stats s3
sudo ovs-ofctl -O Openflow13 queue-stats s4
sudo ovs-ofctl -O Openflow13 queue-stats s5
