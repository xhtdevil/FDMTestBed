Instructions on FDM.cpp:

The FDM.cpp and fdm.h files are created and tested in Visual Studio 2015.

The program provides two ways of input:
	1. automatically generates random SDN-SAT network
	2. reads in config file

1.	For automatically generated input, choose 0 in the console. Then provide
	input for the number of ships and satellites. The program will randomly connect each
	ship to each satellite with probability 0.7 (can be changed in program through variable prob_of_conn).

	The maximum amount of request, satellite capacity and downlink capacity can be configured through vairables: 
	max_requests, max_capacity, and max_downlink. 

	The minimum amount of request, satellite capacity and downlink capacity can be configured through: 
	min_rand_request, min_rand_capacity, and min_rand_downlink.

	For example, the range of requests is [max_requests*min_rand_request, max_requests]. Similarly for capacities.
	
	For source and destination pairs, every ship will randomly choose a destination that it is able to reach
	via satellite links. If some ship is isolated, the program will generate a new network.
	
	Once the network is generated and parameters are configured, FDM is run and results are produced.
	
	The program will eventually output the network topology, random requests and capacities to a config file  "config.txt", 
	and allocation results are in "allocation.txt". The generated "config.txt" can also be fed to the program using input option 2.
	
2.  To feed the program with a config file, one must choose 1 in the console and provide the file name of config file.
	The format of config file is as follows:
	
	The first line has 2 parameters, number of ship (nship) and number of satellite (nsat)
	Next is a nship x nsat matrix with 1 and 0. 1 means there is a link between the ship (row) and satellite (col).
	Next is a nship x nship matrix of non-negative numbers, specifying the request from source ship (row) to destination ship (col).
	Next line is 1 x nsat vector of non-negative numbers, specifying the satellite capacities.
	Next is a nsat x nship matrix of non-negative numbers, specifying the downlink capacities between satellite (row) and ship (col),
		0 means there is no downlink between a satellite and ship.
	
	An example of config file is provided: "config_random.txt".
	
	Again, results are output to "allocation.txt". The format of "allocation.txt" is as follows:
		First n_ship lines: 
			Accessible satellites of each ship, and destination of each ship 
			We currently randomly choose one destination for every ship, i.e. n_ship S-D pairs total

		Next n_sat lines:
			Capacity of each satellite

		Next n_ship blocks:
			Each block first prints the out going traffic from each source ship
				The flows between ship and the connected satellites are printed
			Then we print the incoming traffic to the destination of this ship
				The flows on the downlinks of the destination ship are printed

		Last n_sat lines print the total incoming flows at each satellite (from the connected source ships)
	
Notice:
	When providing your own config file, make sure it follows the format.
	Also, we have been focusing on the correctness of FDM algorithm, thus have not throughly tested some corner cases.
	Our program cannot handle these cases:
		1. negative numbers
		2. source and destination ships are not connected (there is no common satellites connecting them)
		3. mismatched parameters, e.g. nship and nsat do not match the size of request matrix or connectivity matrix.
	
	Finally, you may test the correctness of our program by providing small inputs, and test it with input as large as 
	100 ships and 20 satellites.
