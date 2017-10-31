Instructions on FDM.cpp:

To compile the source file, use g++ -std=c++11  FDM.cpp -o <output_file>

The program provides two ways of input:
	1. automatically generates random SDN-SAT network (currently not supported)
	2. reads in config file

2.  To feed the program with a config file, one must choose 1 in the console and provide the file name of config file.
	The format of config file is as follows:

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

	An example of config file is provided: "config_new.txt".

	Results are output to "allocation.txt". The format of "allocation.txt" is as follows:
		First add host, ship, sat, hub, sat
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
