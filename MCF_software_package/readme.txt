Instructions on FDM.cpp and FDM_hub.cpp:

FDM.cpp is for testcase 4-6, FDM_hub.cpp is for other 7 testcases.
To compile the source file, use g++ -std=c++11  FDM.cpp -o <output_file>, or g++ -std=c++11  FDM_hub.cpp -o <output_file>

The program reads in config file to build the network topology and request matrix.

1. Two options for FDM.cpp, loop or no loop (using mirror ship for destination host):
	the two options should be passed to different .py file, i.e. testBed.py or testBed_noloop.py.
	FDM_hub.cpp has no loop in the topology, so using no loop option by default.
2. Provide the file name of config file, e.g. testcase_4.txt.
3. Specify the output file and path, e.g. ../testcase4-6_loop/allocation.txt or ../test/testcase4-6_noloop/allocation.txt.

For FDM.cpp, the format of config file is as follows:

	The first line has 2 parameters, number of ship (nship) and number of satellite (nsat)
	The second line has nship parameters, each parameter is the total number of hosts connected to each ship:
		The hosts connected to each ship contain source hosts (which generate traffic) and a single sink host;
		The sink host represents the destination of traffic coming to this ship from other ships;
		Denoting the total number of hosts nhost, and number of source hosts n_srchost=nhost-nship.

	Next is a nship x nsat matrix with 1 and 0. 1 means there is a link between the ship (row) and satellite (col).
	Next is a n_srchost x 3 matrix of non-negative numbers, specifying the request from source ship to destination ship:
		The first parameter in each row represents the source ship;
		The second parameter represents destination ship;
		The third parameter represents demand/requirement;
		Since there are in total n_srchost rows, we make sure every source host requests demand to some destination ship;
		In the requirement matrix (check variable Req in source code), each line is converted to Req[srchost][sink_of_dest_ship]=demand.

	Next is a nship x nsat matrix of non-negative numbers, specifying the uplink capacities between ship (row) and satellite (col)
		0 means there is no uplink between a ship and satellite

	Next line is 1 x nsat vector of non-negative numbers, specifying the satellite capacities.
	Final lines are nsat x nship matrix of non-negative numbers, specifying the downlink capacities between satellite (row) and ship (col),
		0 means there is no downlink between a satellite and ship.

	An example of config file is provided: "testcase_4.txt".

The format of config file for FDM_hub.cpp is similar, with the following differences:
	Since the destination is a common hub, the n_srchost x 3 request matrix is now n_srchost x 2 matrix, fixing destination as the hub
	THe downlink capacities are no longer nsat x nship matrix. It is now a 1x nsat vector.

	Results are output to e.g. "allocation.txt". The format of "allocation.txt" is as follows:
		First print source host , destination host, destination host ip;
		Then add host, ship, sat, hub, sat
		Then add link between nodes
		Then print the IP usage of each source host
		Finally print the flows on each link:
			link is represented as endnode-eth_port endnode-eth_port
			flows are identified by IP, associated by the amount of flow given from FDM

Notice:
	When providing your own config file, make sure it follows the format.
	Also, we have been focusing on the correctness of FDM algorithm, thus have not throughly tested some corner cases.
	Our program cannot handle these cases:
		1. negative numbers
		2. source and destination ships are not connected (there is no common satellites connecting them)
		3. mismatched parameters, e.g. nship and nsat do not match the size of request matrix or connectivity matrix.
