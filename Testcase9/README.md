# Testcase9 
## testcase9\_original:
In order to compare the SingleTCP, MPTCP and FDM, the following directories are useful:  
 
* testcase9_single
* testcase9_mptcp
* testcase9\_mu_delay  

(testcase9\_mu is for MU algorithm without propagation delay, do not use it if you desire to include propagation delay.)

Each directory should contain at least the following files:  

* testBed_hub.py
* analysis-9.py
* allocation.txt
* Flowtable
* Queuetable (only FDM-based algorithms have)

To run the test, fist use command "cd" to go into that particular directory, and then "**sudo python testBed_hub.py**".

After the program finishes, **go to /home/osboxes/D-ITG-2.8.1-r1023/bin/ to seek generated csv files.**

## testcase9\_modi
In order to compare the SingleTCP, MPTCP and FDM, the following directoris are useful:  
 
* testcase9_single
* testcase9_mptcp
* testcase9\_mu_delay

The testing method is the **same** as what you do in testcase9_original directory.

## testcase9\_delay\_graph
This directory includes all the detailed graphs and csv files which are used to generate them.

analyis\_9.R, analysis\_9\_modi.R and Draw\_testcase9.Rd are the R files used to draw those statistical graphs.
