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
