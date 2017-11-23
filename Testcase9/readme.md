* testcase_um_9.text is the file defining the topology and requirements of Testcase9, which is the input of FDM+UDP algorithm
* Alloc_hub.txt is the file specifying hosts, links, flows (UDP/MPTCP), the number of flows at each interface, which is the output of FDM+UDP algorithm
* Coincidentally, each MPTCP host only has one sub-TCPflow again in the setting of Testcase9.
* Each ship is connected to two hosts. Specifically, one host only sends UDP packets, and the other host sends MPTCP packets.The flow requirements of them are equal distributed, which both are the half of total requirement of the ship that they belong to.
