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
