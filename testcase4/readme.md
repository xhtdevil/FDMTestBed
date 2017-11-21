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
It's time to modify the orginal testbed and run some testings! But how should we do?

### Add D-ITG commandlines to testbed
From the original Python testbed, we can add code shown below:

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
Here, we use bandwidth with ```bwReq[i]*125``` to convert Mbits to kbytes.

* the ```ITGTest()``` function, we have:
```
def ITGTest(srcNo, dstNo, hosts, nodes, bw, sTime):
    src = nodes[hosts[srcNo]] # source node
    dst = nodes[hosts[dstNo]] # destination node
    info("Sending message from ",src.name,"<->",dst.name,"...",'\n')
    src.cmdPrint("cd ~/D-ITG-2.8.1-r1023/bin")
    src.cmdPrint("./ITGSend -T TCP  -a 10.0.0."+str(dstNo+1)+" -c 1000 -C "+str(bw)+" -t "+str(sTime)+" -l sender"+str(srcNo)+".log -x receiver"+str(srcNo)+"ss"+str(dstNo)+".log &")
```
With ```ITGTest()```, we can have log files named like ```receiver*ss*.log```, where the first ```*``` stands for sender's node number and the second for receiver's. And we can note that those logs files are located at ```~/D-ITG-2.8.1-r1023/bin```.

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

We can use ```mininet > host0 watch -n 1 'netstat -n'``` to monitor host status. Below is a capture of netstat of host0 when running MPTCP. With bandwidth constraint, we can see the host is sending TCP messages using multi IPs. 
![](https://github.com/rockwellcollins/FDMTestBed/blob/rockwellcollins-patch-3/testcase4/raw/cap.PNG?raw=true)

## Analysis after Testings
### Decode Logs and Aggregate Information
Now we have run the testbed and get a btunch of log files in ```~/D-ITG-2.8.1-r1023/bin```. It's time to run analysis to decode those logs on receivers and aggregate the information that we need.

The analysis script is on ```/testcase4/analysis/analysis.py```, and should be placed at ```~/D-ITG-2.8.1-r1023/bin```. The Python script takes the logs on receivers as input (why receivers: sender's logs do not have delay analysis, obviously), and decoded txts and ```result.csv``` as output. 

If you take a quick look at the ```analysis.py```, you can find two functions: ```extractStat(sndList, recvList)``` and ```statAnalysis(sndList, recvList)```. The first one decodes the logs and the second aggregate information of various sender-receiver pairs.

Result.csv contains information of the D-ITG testing. Although we only used average bitrate, average delay and delay sd as criteria, we still leave some other parameters that may be useful for further analysis. 
For reference, every row in the csv file stands for : ```"total_time","total_packets","min_delay","max_delay","avg_delay","avg_jitter","sd_delay","avg_bit_rate","avg_pkt_rate"```.

### Plot with R
```analysis1109.R``` is an R file to plot the satisfaction and delay result. If you do not have R on your computer, you can use Excel to plot similar ones. When you run R on your computer, remember to change working directory to folder that contains ```result.csv```.

### Draw Histogram with R
```analysis1116.R``` draws Histogram of staisfacroty and delay distribution according results of mass testing (in ```\result```).

## Anyway, if I want to run the performance testing as soon as possible, what should I do?
That's a very good question! In fact, all the information above is not necessary for doing performance testing, but very useful when you want to modify the code or debug.

First we should know what is useful and where to place those files.

Below three folers contains three scenarios of MPTCP, Single TCP and FDM
* ```/multi``` contains files for MPTCP testings
* ```/single``` contains files for Single TCP testings
* ```/FDM``` contains files for FDM testings

Below folder contains analysis codes
* ```/analysis```
* the ```analysis.py``` should be placed at ```~/D-ITG-2.8.1-r1023/bin```
* the working directory of ```analysis1109.R``` should be changed to folder that contains ```result.csv``` before running.

Now we do Single TCP Testings:
* ```$ sudo python ~/FOO/single/SingleTCPtestbed.py```
* wait for 2 minutes until ```mininet > ``` appears
* change to ```~/D-ITG-2.8.1-r1023/bin```
* run ```python analysis.py```
* move the ```result.csv``` to ```~/FOO```
* open ```analysis1109.R```, ```setwd("~/FOO")```
* run ```analysis1109.R```, the result should appear on CLI and plots should be in ```~/FOO```

You can also run the rest two cases similarly : )

## Result and Analysis
![](https://github.com/xhtdevil/FDMTestBed/blob/master/testcase4/raw/Topology%20of%20Testcase%204.PNG?raw=true)
According to the topology of testcase 4(shown above), we ran network simulation 10 times with each routing method (Single TCP, MPTCP and MPTCP with FDM), separately. Using D-ITG as a traffic generator to make host send 1k byte TCP packets to the destinations with rate defined by requirements. We use satisfactory, which is defined by (bw in real network)/(requirement bw), to describe how the network fulfill sending requirements (why not throughput: bw requirements of hosts differs), and delay to evaluate the quality of connections.
After testing, we aggregate the criterias of each host and draw Histogram(why not average value: It's better to use Histogram than average value to evaluate the performance of network, since the data distribution is not normal.) of both parameters to see the perfrmance difference of routing method. The result is shown as follows. (In this project, I used R to run analysis. The result code is in ```\result``` and you can run ```\analysis\analysis1116.R``` to get the plots. You can also use similar statistic tools if you like.)
![](https://github.com/xhtdevil/FDMTestBed/blob/master/testcase4/Satisfactory%20Distribution.jpg?raw=true)
![](https://github.com/xhtdevil/FDMTestBed/blob/master/testcase4/Delay%20Distribution.jpg?raw=true)
From the plot, we can see that although FDM uses single path, FDM still achieves better performance.

## Discussion: Lessons Learned(前车之鉴 in Chinese)
//TODO
