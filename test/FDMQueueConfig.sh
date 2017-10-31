#!/bin/bash

sudo ovs-vsctl -- set Port s1-eth7 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:1=@q1 -- --id=@q1 create Queue other-config:min-rate=4618000 other-config:max-rate=4618000
sudo ovs-vsctl -- set Port s1-eth7 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:2=@q2 -- --id=@q2 create Queue other-config:min-rate=3102310 other-config:max-rate=3102310
sudo ovs-vsctl -- set Port s1-eth8 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:3=@q3 -- --id=@q3 create Queue other-config:min-rate=238970 other-config:max-rate=238970
sudo ovs-vsctl -- set Port s1-eth9 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:4=@q4 -- --id=@q4 create Queue other-config:min-rate=1143030 other-config:max-rate=1143030
sudo ovs-vsctl -- set Port s1-eth9 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:5=@q5 -- --id=@q5 create Queue other-config:min-rate=897687 other-config:max-rate=897687
sudo ovs-vsctl -- set Port s2-eth7 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:6=@q6 -- --id=@q6 create Queue other-config:min-rate=5387670 other-config:max-rate=5387670
sudo ovs-vsctl -- set Port s2-eth8 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:7=@q7 -- --id=@q7 create Queue other-config:min-rate=2416220 other-config:max-rate=2416220
sudo ovs-vsctl -- set Port s2-eth8 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:8=@q8 -- --id=@q8 create Queue other-config:min-rate=278798 other-config:max-rate=278798
sudo ovs-vsctl -- set Port s2-eth9 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:9=@q9 -- --id=@q9 create Queue other-config:min-rate=1333540 other-config:max-rate=1333540
sudo ovs-vsctl -- set Port s2-eth9 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:10=@q10 -- --id=@q10 create Queue other-config:min-rate=583783 other-config:max-rate=583783
sudo ovs-vsctl -- set Port s3-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:11=@q11 -- --id=@q11 create Queue other-config:min-rate=3221620 other-config:max-rate=3221620
sudo ovs-vsctl -- set Port s3-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:12=@q12 -- --id=@q12 create Queue other-config:min-rate=3221620 other-config:max-rate=3221620
sudo ovs-vsctl -- set Port s3-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:13=@q13 -- --id=@q13 create Queue other-config:min-rate=778377 other-config:max-rate=778377
sudo ovs-vsctl -- set Port s3-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:14=@q14 -- --id=@q14 create Queue other-config:min-rate=778377 other-config:max-rate=778377
sudo ovs-vsctl -- set Port s4-eth7 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:15=@q15 -- --id=@q15 create Queue other-config:min-rate=2309000 other-config:max-rate=2309000
sudo ovs-vsctl -- set Port s4-eth8 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:16=@q16 -- --id=@q16 create Queue other-config:min-rate=2416220 other-config:max-rate=2416220
sudo ovs-vsctl -- set Port s4-eth8 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:17=@q17 -- --id=@q17 create Queue other-config:min-rate=119485 other-config:max-rate=119485
sudo ovs-vsctl -- set Port s4-eth9 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:18=@q18 -- --id=@q18 create Queue other-config:min-rate=571515 other-config:max-rate=571515
sudo ovs-vsctl -- set Port s4-eth9 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:19=@q19 -- --id=@q19 create Queue other-config:min-rate=583783 other-config:max-rate=583783
sudo ovs-vsctl -- set Port s5-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:20=@q20 -- --id=@q20 create Queue other-config:min-rate=2326730 other-config:max-rate=2326730
sudo ovs-vsctl -- set Port s5-eth5 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:21=@q21 -- --id=@q21 create Queue other-config:min-rate=2326730 other-config:max-rate=2326730
sudo ovs-vsctl -- set Port s5-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:22=@q22 -- --id=@q22 create Queue other-config:min-rate=673265 other-config:max-rate=673265
sudo ovs-vsctl -- set Port s5-eth6 qos=@newqos -- --id=@newqos create QoS type=linux-htb other-config:max-rate=100000000 queues:23=@q23 -- --id=@q23 create Queue other-config:min-rate=673265 other-config:max-rate=673265
sudo ovs-ofctl -O Openflow13 queue-stats s1
sudo ovs-ofctl -O Openflow13 queue-stats s2
sudo ovs-ofctl -O Openflow13 queue-stats s3
sudo ovs-ofctl -O Openflow13 queue-stats s4
sudo ovs-ofctl -O Openflow13 queue-stats s5
