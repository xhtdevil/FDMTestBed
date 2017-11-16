# Running Performance Test with D-ITG

## D-ITG as a Traffic Generator
### Download and Deploy D-ITG
[D-ITG](http://traffic.comics.unina.it/software/ITG/documentation.php) is a traffic generator. I followed instructions [here](http://sdnopenflow.blogspot.com/2015/05/using-of-d-itg-traffic-generator-in.html) to build D-ITG.

Follow the instructions and D-ITG can be deployed on ```~/D-ITG-2.8.1-r1023/bin```
```sh
$ sudo apt-get install unzip
$ sudo apt-get install g++
$ wget   http://traffic.comics.unina.it/software/ITG/codice/D-ITG-2.8.1-r1023-src.zip
$ unzip  D-ITG-2.8.1-r1023-src.zip
$ cd  D-ITG-2.8.1-r1023/src
$ make
```

### Testing Network Performance with D-ITG
D-ITG has a server to "hear" from the traffic and a client to send the traffic as commanded.
But we note that, after ovs has been initiated, we should change directory ```~/D-ITG-2.8.1-r1023/bin``` to use D-ITG.
For specific usage, we can find manual [here](http://traffic.comics.unina.it/software/ITG/manual/).

e.g.
Follow those steps to run a simple bw testing between host0 and host11 in mininet
* Open host0 and host11 ```mininet > xterm host0 host11```
* Start server on host11 ```host11$ cd ~/D-ITG-2.8.1-r1023/bin```
* ```host11$ ./ITGRecv```
* Start client on host0. ```host0$ ./ITGSend -T TCP -a 10.0.0.12 -c 1000 -C 200 -t 150000 -l sender.log -x receiver.log```
* The command in STEP4 says, host0 should send TCP packets to 10.0.0.12, with 1000bit packet, 200 packets per second, and send those packets continuously for 150000ms, the sender(client)'s log is saved as ```sender.log``` while the receiver(server)'s log is saved as ```receiver.log```.
* Decode the receiver's log with ```./ITGDec receiver.log > receiver.txt``` to save the information decoded as txt files.
The content of the decoded logs is shown as follows.

![](https://github.com/rockwellcollins/FDMTestBed/blob/rockwellcollins-patch-3/testcase4/raw/logfile.png?raw=true)

### Special Reminder
OVS and the host running mininet share the same folder, so before using python API to gengeate D-ITG commands, make sure the names of different testing do not overlap.

## How to write a generator with D-ITG using Python?
### Add D-ITG commandlines to testbed
From the original Python testbed, we can add code belows:

According to testcase 4, we have:

* start the servers 
```
# start D-ITG Servers
for i in [2,5,8,11,14]:
    srv = nodes[hosts[i]]
    info("starting D-ITG servers...\n")
    srv.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
    srv.cmdPrint("./ITGRecv &")
```
* start the clients
```    
# start D-ITG application
# set simulation time
sTime = 120000 # default Simulation Time: 120,000ms
for i in range(0,10):
    senderList = [0,1,3,4,6,7,9,10,12,13] # sender list
    recvList = [11,14,2,8,5,11,5,8,2,11] # receiver list
    bwReq = [6,4,7,3,4,4,3,3,3,3] # bandwidth requirement in testcase 4
    ITGTest(senderList[i], recvList[i], hosts, nodes, bwReq[i]*125, sTime)
    time.sleep(0.2)
    info("running simulaiton...\n")
 ```
Here, we use bandwidth with ```bwReq[i]*125``` to convert MBytes to kbits.

* the ```ITGTest()``` function, we have:
```
def ITGTest(srcNo, dstNo, hosts, nodes, bw, sTime):
    src = nodes[hosts[srcNo]] # source node
    dst = nodes[hosts[dstNo]] # destination node
    info("Sending message from ",src.name,"<->",dst.name,"...",'\n')
    src.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
    src.cmdPrint("./ITGSend -T TCP  -a 10.0.0."+str(dstNo+1)+" -c 1000 -C "+str(bw)+" -t "+str(sTime)+" -l sender"+str(srcNo)+".log -x receiver"+str(srcNo)+"ss"+str(dstNo)+".log &")
```
With ```ITGTest()```, we can have log files named like ```receiver*ss*.log```, where the first ```*``` stands for sender's node number and the second for receiver's.

### With traffic generator, do we still need to add bw to links?
The answer is yes!

Without bw constraint, the D-ITG client would only send single path with one IP address in MPTCP testing. So we still need to add bw constraint to links.

According to testcase 4, we have:

```
""" Add links """
for link in links:
    name1, name2 = link[0], link[1]
    node1, node2 = nodes[name1], nodes[name2]
    if(name1 == 's6' and name2 == 's9'):
        net.addLink(node1, node2, bw = 30)
    elif(name1 == 's7' and name2 == 's10'):
        net.addLink(node1, node2, bw = 20)
    elif (name1 == 's8' and name2 == 's11'):
        net.addLink(node1, node2, bw = 15)
    else:
        net.addLink(node1, node2)
 ```

With bandwidth constraint, we can see the host is sending MPTCP.
