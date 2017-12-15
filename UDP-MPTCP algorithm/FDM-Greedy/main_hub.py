import fdm_hub
from pprint import pprint

class Pair(object):
	"""docstring for Pair"""
	def __init__(self, start, end, sPort, ePort, cap):
		self.cap = cap
		self.start = start
		self.end = end
		self.startPort = sPort
		self.endPort = ePort
	def sprint(self):
		print "cap"+str(self.cap)+"\tstart" + str(self.start) + "\tend" + str(self.end) + "\tsport" + str(self.startPort) + "\teport" + str(self.endPort)


class Topo(object):
	"""docstring for Topo"""
	def __init__(self, nSat, nShip, hostShipConnect, nHost):
		self.nSat = nSat		#number of sat
		self.nShip = nShip		#number of ship
		self.nHost = nHost	#number of host
		self.hostShipConnect = hostShipConnect
		self.connectivity = []
		self.shipSat = []
		self.countLink = 0
		self.nSrcHost = 0
		self.requests =[]
		self.udpList = []
		self.mptcpList = []
		for x in xrange(nHost):			#init the nHost*nHost requests  
			self.requests.append([0]*nHost)	#fill with 0
			self.udpList.append([0]*nHost)
			self.mptcpList.append([0]*nHost)
		self.uplinkCap = []
		self.downlinkCap = []
		self.satCap = []
		self.srcDest = {}
		
class fdmData(object):
	"""data prepare for FDM handling"""
	def __init__(self,nL,nN,cap,startList,endList,link,adj,ipTable,names):
		self.nL = nL
		self.nN = nN
		self.cap = cap
		self.startList = startList
		self.endList = endList
		self.link = link
		self.adj = adj
		self.ipTable = ipTable
		self.names = names
		self.ships = set()
		self.hubs = set()
		self.mirrorShips = set()
		self.vPairs = []


def readFromText(filename):
	f = open(filename,"r")
	line = ""
	#get nShip and nSat
	list1 = getlineList(f)
	nShip = int(list1[0])
	nSat = int(list1[1])
	#nShip parameter, total number of host connect ot each ship
	hostShipConnect = getlineList(f)[0:nShip]

	nHost = 0
	for i in range(len(hostShipConnect)):
		host = int(hostShipConnect[i])
		hostShipConnect[i] = range(nHost, nHost + host)
		nHost += host
	desHub = nHost
	nHost += 1
	#fit in class Topo
	topology = Topo(nSat, nShip, hostShipConnect, nHost)
	topology.connectivity = getMatrix(f, nSat, nShip)
	countLink = 0
	shipSat = []
	for item in topology.connectivity:
		shipSat.append(item.count(1))
		countLink += item.count(1)
	topology.countLink = countLink
	topology.shipSat = shipSat
	topology.nSrcHost = nHost - nShip


	#mptcp
	reqNum = getlineList(f)[0]
	tab = [0]*nShip
	for x in xrange(reqNum):
		line = getlineList(f)
		scr = int(line[0])
		des = desHub
		cap = int(line[1])
		scrid = tab[scr]
		tab[scr] =tab[scr] + 1
		scr = hostShipConnect[scr][scrid]
		topology.requests[scr][des] = cap
		topology.mptcpList[scr][des] = cap
		topology.srcDest[scr] = des

	#udp
	reqNum = getlineList(f)[0]
	for x in xrange(reqNum):
		line = getlineList(f)
		scr = int(line[0])
		des = desHub
		cap = int(line[1])
		scrid = tab[scr]
		tab[scr] =tab[scr] + 1
		scr = hostShipConnect[scr][scrid]
		topology.requests[scr][des] = cap
		topology.udpList[scr][des] = cap
		topology.srcDest[scr] = des


	#uplink cap
	topology.uplinkCap = getMatrix(f, nSat, nShip)

	#sat cap
	topology.satCap = getlineList(f)

	#uplink cap
	topology.downlinkCap = getlineList(f)
#	topology.downlinkCap = getMatrix(f, nShip, nSat)
	return topology


def topologyBuilder(topology, loop):
	nHost = topology.nHost
	nShip = topology.nShip
	nSat = topology.nSat
	countLink = topology.countLink
	hostShipConnect = topology.hostShipConnect
	shipSat =topology.shipSat
	connectivity = topology.connectivity
	uplinkCap = topology.uplinkCap
	downlinkCap =topology.downlinkCap
	satCap =topology.satCap
	desHub = nHost - 1
	nN = 0
	nN = nHost + nShip + 2 * nSat + 1
	nL = nHost +  countLink + 2 * nSat

	ships = set()
	hubs = set()
	mirrorShips = set()
	for i in range(nShip):
		ships.add(i + nHost)
	hubs.add(nN - 1)

	ports = []
	names = []
	cnt = 1
	for i in range(nN):
		if i < nHost:
			ports.append(0)
			names.append("host" + str(i))
		else:
			ports.append(1)
			names.append("s" + str(cnt))
			cnt += 1

	vPairs = []
	helpCounter = 0
	#uplink
	for i in range(nShip):
		for j in range(len(hostShipConnect[i]) ):
			host = hostShipConnect[i][j]
			ship = nHost +i
			p = Pair(host, ship, ports[host], ports[ship], -1)
			vPairs.append(p)
			ports[host] = ports[host] + 1
			ports[ship] = ports[ship] + shipSat[i]
			helpCounter +=1

	#adding links between ship and sat
	for i in range(nShip):
		for j in range(nSat):
			if connectivity[i][j]:
				ship = nHost + i
				sat = nHost + nShip + j
				p = Pair( ship, sat,  ports[ship],ports[sat], uplinkCap[i][j])
				vPairs.append(p)
				ports[ship] = ports[ship] + 1
				ports[sat] = ports[sat] + 1
				helpCounter += 1

	#sat and dumb sat
	for i in range(nSat):
		sat = nHost + nShip + i
		dumbSat = nHost + nShip + nSat + i
		p = Pair(sat, dumbSat, ports[sat],ports[dumbSat], satCap[i])
		vPairs.append(p)
		ports[sat] = ports[sat] + 1
		ports[dumbSat] = ports[dumbSat] + 1
		helpCounter +=1

	#add downlinks to hub switch
	for i in range(nSat):
		sat = nHost + nShip + nSat + i
		hub = nN - 1
		p = Pair( sat,hub, ports[sat],ports[hub], downlinkCap[i])
		vPairs.append(p)
		ports[sat] = ports[sat] + 1
		ports[hub] = ports[hub] + 1
		helpCounter +=1
	
	#adding hub to dest_hub
	hub = nN -1 
	p = Pair(hub, desHub, ports[hub], ports[desHub], -1)
	vPairs.append(p)
	ports[hub] += 1
	ports[desHub] += 1
	helpCounter += 1


	link = max(nShip, nSat) + 1
	end1 = []
	end2 = []
	adj = []
	Adj = []
	cap=[]
	#initialize Adj with -1
	for i in range(nN):
		Adj.append([-1]*link)
		adj.append([])

	#set end1, end2, cap and adj
	link_count = 0
	for item in vPairs:
		end1.append(item.start)
		end2.append(item.end)
		adj[item.start].append(link_count)
		if item.cap == -1:
			cap.append(float("inf"))
		else:
			cap.append(item.cap)
		link_count+=1
	#set adj from adj
	node_count = 0
	for item in adj:
		for i in range(len(item)):
			Adj[node_count][i] = item[i]
		node_count +=1

	ipTable = {}
	for i in range(nShip):
		ship = topology.nHost + i
		for j in range(len(hostShipConnect[i])):
			host = hostShipConnect[i][j]
			for k in range(shipSat[i]):
				sat = end2[adj[ship][k]]
				key = str(host) + " " + str(sat)
				value = "10.0." + str(host+1) + "." + str(k)
				ipTable[key] = value

	fd = fdmData(nL,nN,cap,end1,end2,link,Adj, ipTable,names)
	fd.ships = ships
	fd.hubs = hubs
	fd.mirrorShips = mirrorShips
	fd.vPairs = vPairs
	return fd


'''get line from text file, ignore empty line and  with "#"'''
def getlineList(f):
	line = ''
	while (not line) or line =="\n" or line.startswith("#"):
		line = f.readline() 
	return map(int,line.split())


'''convert a matrix in text file into n*m double list'''
'''n:width of matrix 	m:height of matrix'''
def getMatrix(f,n,m):
	mat = []
	for i in xrange(m):
		line = getlineList(f)
		lineList = line[0:n]
		mat.append(map(int,lineList))
	return mat


'''=====================MAIN================================='''
loop = False

configFile = "7.txt"
outputFile = "b.txt"
output = open(outputFile,"w")
topology = readFromText(configFile)

#topology builder
fdmData = topologyBuilder(topology, loop)
#udpAssign()
#run FDM
gTable, uTable= fdm_hub.fdm(fdmData,topology)





srcDest = topology.srcDest
nHost = topology.nHost
shipSat =topology.shipSat


ships = fdmData.ships
hubs = fdmData.hubs
mirrorShips = fdmData.mirrorShips


nN = fdmData.nN
nL = fdmData.nL
names = fdmData.names
end1 = fdmData.startList
end2 = fdmData.endList
vPairs = fdmData.vPairs
nSrcHost = topology.nSrcHost

for item in srcDest:
	output.write(names[item]+" "+names[srcDest[item]]+" 10.0.0." + str(srcDest[item]+1)+"\n")
output.write("End\n")
mptcpUsedIP = {}
udpUsedIP = {}

for link in range(nSrcHost):
	srcNode = end1[link]
	for item in gTable[link]:
		if gTable[link][item] > 1.0e-5:
			name = names[srcNode]
			if name in mptcpUsedIP:
				mptcpUsedIP[name].append(item)
			else:
				mptcpUsedIP[name] = [item]
	for item in uTable[link]:
		if uTable[link][item] > 1.0e-5:
			name = names[srcNode]
			if name in udpUsedIP:
				udpUsedIP[name].append(item)
			else:
				udpUsedIP[name] = [item]



#output nodes and links in order
for i in range(nN):
	if i < nHost:
		output.write("add host: " + names[i] + "\n")
	elif i in ships:
		output.write("add ship: " + names[i] + "\n")
	elif i in hubs:
		output.write("add hub: " + names[i] + "\n")
	elif i in mirrorShips:
		output.write("add mirror_ship: " + names[i] + "\n")
	else:
		output.write("add sat: " + names[i] + "\n")
output.write("End\n")
for i in range(nL):
	if i < nSrcHost:
		for j in range(shipSat[end2[i]-nHost]):
			src = end1[i]
			des = end2[i]
			output.write("add link: " + names[src] + " " + names[des] + "\n")
	else:
		src = end1[i]
		des = end2[i]
		output.write("add link: " + names[src] + " " + names[des] + "\n")
output.write("End\n")


for i in range(nHost):
	key = names[i]
	if key in mptcpUsedIP:
		output.write(str(key) + " num_of_ip: " + str(len(mptcpUsedIP[key])) + " MPTCP\n")
		for ip in mptcpUsedIP[key]:
			output.write(ip + "\n")
	elif key in udpUsedIP:
		output.write(str(key) + " num_of_ip: " + str(len(udpUsedIP[key])) + " UDP\n")
		for ip in udpUsedIP[key]:
			output.write(ip + "\n")
	else:
		output.write(str(key) + " num_of_ip: 0\n")

#print link flow table
for link in range(nL):
	pair = vPairs[link]
	toDelete = set()
	for item in gTable[link]:
		if float(gTable[link][item]) < 1.0e-5:
			toDelete.add(item)

	for item in toDelete:
		if item in gTable[link]:
			gTable[link].pop(item)
		if item in uTable[link]:
			uTable[link].pop(item)
	if len(gTable[link]) == 0 and len(uTable[link]) == 0:
		continue
	output.write(names[pair.start] + "-eth" + str(pair.startPort) + " " )
	output.write(names[pair.end] + "-eth" + str(pair.endPort) + "\t")
	output.write("num_of_flow:" + str(len(gTable[link])+len(uTable[link]))+"\n")

	#raw_input()
	for item in gTable[link]:
		output.write("\t\t" + str(item) + " MPTCP " + str(round(gTable[link][item],4)) + "\n")
	for item in uTable[link]:
		output.write("\t\t" + str(item) + " UDP " + str(round(uTable[link][item],4)) + "\n")


output.write("End\n")


output.close()