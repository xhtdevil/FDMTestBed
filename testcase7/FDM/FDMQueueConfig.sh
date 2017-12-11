#!/bin/bash

sudo ifconfig s1-eth3 txqueuelen 50
sudo ovs-vsctl -- set Port s1-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:1=@q1 --  --id=@q1 create Queue other-config:min-rate=5652440 other-config:max-rate=5652440
sudo ifconfig s1-eth4 txqueuelen 50
sudo ovs-vsctl -- set Port s1-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:2=@q2 --  --id=@q2 create Queue other-config:min-rate=7917870 other-config:max-rate=7917870
sudo ifconfig s2-eth3 txqueuelen 50
sudo ovs-vsctl -- set Port s2-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:3=@q3 --  --id=@q3 create Queue other-config:min-rate=10207800 other-config:max-rate=10207800
sudo ifconfig s2-eth4 txqueuelen 50
sudo ovs-vsctl -- set Port s2-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:4=@q4 --  --id=@q4 create Queue other-config:min-rate=3362550 other-config:max-rate=3362550
sudo ifconfig s3-eth2 txqueuelen 50
sudo ovs-vsctl -- set Port s3-eth2 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:5=@q5 --  --id=@q5 create Queue other-config:min-rate=13570300 other-config:max-rate=13570300
sudo ifconfig s4-eth3 txqueuelen 50
sudo ovs-vsctl -- set Port s4-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:6=@q6 --  --id=@q6 create Queue other-config:min-rate=9026550 other-config:max-rate=9026550
sudo ifconfig s4-eth4 txqueuelen 50
sudo ovs-vsctl -- set Port s4-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:7=@q7 --  --id=@q7 create Queue other-config:min-rate=2973450 other-config:max-rate=2973450
sudo ifconfig s5-eth3 txqueuelen 50
sudo ovs-vsctl -- set Port s5-eth3 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:8=@q8 --  --id=@q8 create Queue other-config:min-rate=4998360 other-config:max-rate=4998360
sudo ifconfig s5-eth4 txqueuelen 50
sudo ovs-vsctl -- set Port s5-eth4 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:9=@q9 --  --id=@q9 create Queue other-config:min-rate=7001640 other-config:max-rate=7001640
sudo ovs-ofctl -O Openflow13 queue-stats s1
sudo ovs-ofctl -O Openflow13 queue-stats s2
sudo ovs-ofctl -O Openflow13 queue-stats s3
sudo ovs-ofctl -O Openflow13 queue-stats s4
sudo ovs-ofctl -O Openflow13 queue-stats s5
