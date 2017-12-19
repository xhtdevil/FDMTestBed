import numpy as numpy
import Queue
from pprint import pprint
from collections import OrderedDict


EPS = 0.0001
DELTA = 0.002
STEP = 0.05
#helper
def calcDelay(nL, gFlow,uFlow, cap, msgLen, totalReq, cost):
	sum = 0.0
	tFlow = nL * [0]
	for i in range(nL):
		tFlow[i] = gFlow[i] + uFlow[i]
	for i in range(nL):
		#print "tFlow["+str(i)+"]" + str(tFlow[i])
		#print gFlow[i],uFlow[i], cap[i]
		sum = sum + tFlow[i] * linkDelayOf(tFlow[i],cap[i]+ uFlow[i],msgLen,cost[i])
		#print "sum" + str(i) +":" + str(sum)

	return sum/totalReq

def linkDelayOf(gFlow, cap, msgLen,cost):
	#print "-----------------------"
	#print gFlow, cap
	#print  1.0-float(gFlow)/cap
	if gFlow == cap:
		return float("inf")
	return (float(msgLen)/cap)/(1.0-float(gFlow)/(cap)+cost)

def adjustCaps(nL, gFlow, cap, newCap):
	factor = 1.0;
	for i in range(nL):
		factor = max(factor,(1+DELTA)*gFlow[i]/cap[i])
	for x in range(nL):
		newCap[x] = factor*cap[x]
	return factor

def setLinkLens(nL,gFlow,uFlow, cap,msgLen,fdLen, cost):
	for i in range(0,nL):
		flow = gFlow[i]+uFlow[i]
		cap1 = cap[i] + uFlow[i]
		cost1 = cost[i]
		f = 1 - float(flow)/(cap1)
		if f == 0:
			return float("inf")
		#print msgLen,cap1
		fdLen[i] = (float(msgLen)/(cap1))/(f*f) + cost1

def setSP(nN, link, end2, fdLen,adj,spDist,spPred):
	for i in range(nN):
		Bellman(nN,link,i,end2,fdLen,adj,spDist[i],spPred[i])

def Bellman(nN,link,root,end2,linkLength,adj,dist,pred):
	hop = [0]*nN
	for i in range(nN):
		dist[i] = float("inf")
	dist[root] = 0
	pred[root] = root

	scanQueue = []
	scanQueue.append(root)
	while scanQueue:
		node = scanQueue.pop()
		for i in range(link):
			curLink = adj[node][i]
			if curLink == -1:
				break
			node2 = end2[curLink]

			d = float(dist[node]+linkLength[curLink])
			if dist[node2] > d:
				dist[node2] = d
				pred[node2] = curLink
				hop[node2] = hop[node] + 1
				if hop[node2] < 6:
					scanQueue.append(node2)

def loadLinks(nN, nL, req, spPred, end1, ipTable):
	flow = [0]*nL
	table =[]
	for i in range(nL):
		table.append({})
	for src in xrange(nN):
		for dest in xrange(nN):
			if req[src][dest] > 0:
				pathLink = []
				pathNode = []
				m = dest
				pathNode.append(m)
				while (m != src):
					link = int(spPred[src][m])
					p = end1[link]
					pathNode.append(p)
					pathLink.append(link)
					m = p
				key = str(src) + " " + str(pathNode[-3])
				ip = ipTable[key]
				for k in list(reversed(range(len(pathLink)))):
					a = pathLink[k]
					flow[a] += req[src][dest]
					table[a][ip] = req[src][dest]

	return flow,table

def superpose(nL, eFlow,gFlow,uFlow,cap, totalReq, msgLen,gTable,eTable, cost):
	x = findX(nL,gFlow,eFlow, uFlow,cap,totalReq,msgLen,cost)
	
	for i in range(nL):
		gFlow[i] = x*eFlow[i] + (1 - x)*gFlow[i]
		tmp = 0.0

		for item in gTable[i]:
			gTable[i][item] *= (1- x)
			tmp += gTable[i][item]
		for item in eTable[i]:
			if gTable[i].has_key(item):
				gTable[i][item] += eTable[i][item] * x
			else:
				gTable[i][item] = eTable[i][item] * x
			tmp +=  eTable[i][item] * x

def findX(nL,gFlow,eFlow,uFlow,cap,totalReq,msgLen,cost):
	xLimit = 0.0
	st = 0.0
	end = 1.0
	flow = [0.0]*nL
	while end -st > 0.0001:		#binary search 
		xLimit = st + (end - st)/2
		exceed = False
		for i in range(nL):
			flow[i] = xLimit * eFlow[i] + (1-xLimit) * gFlow[i]
			if flow[i] > cap[i]:	#flow overload the cap
				exceed = True
				break
		if exceed is True:
			end = xLimit 			#rerun binary search right
		else:
			st = xLimit 			#binary search left
	#end of finding xlimit, use st for xlimit
	xLimit = st
	x0 = 0.0
	x2 = numpy.mean([x0,xLimit])
	x4 = xLimit
	f0 = delayF(x0, nL, eFlow,gFlow,uFlow, cap, msgLen, totalReq, cost)
	f2 = delayF(x2, nL, eFlow,gFlow,uFlow, cap, msgLen, totalReq, cost)
	f4 = delayF(x4, nL, eFlow,gFlow,uFlow, cap, msgLen, totalReq, cost)

	while (x4 - x0) > EPS:
		x1 = numpy.mean([x0,x2])
		x3 = numpy.mean([x2,x4])
		f1 = delayF(x1, nL, eFlow,gFlow, uFlow, cap, msgLen, totalReq, cost)
		f3 = delayF(x3, nL, eFlow,gFlow, uFlow, cap, msgLen, totalReq, cost)
		if  (f0 <= f1) or (f1 <= f2):
			x4 = x2
			x2 = x1
			f4 = f2
			f2 = f1
		elif f2 <= f3:
			x0 = x1
			x4 = x3
			f0 = f1
			f4 = f3
		else:
			x0 = x2
			x2 = x3
			f0 = f2
			f2 = f3
	if f0 <= f2 and f0 <= f4:
		return x0
	elif f2< f4:
		return x2
	else:
		return x4

def delayF(x,nL,eFlow,gFlow,uFlow,cap,msgLen,totalReq,cost):
	flow = [0]*nL
	for i in range(0,nL):
		flow[i] = x * eFlow[i] +(1.0-x)*gFlow[i]
	return calcDelay(nL, flow,uFlow, cap, msgLen, totalReq, cost)



def fdm(fdmData, topology):
	nL = fdmData.nL
	nN =fdmData.nN
	cap = fdmData.cap
	end1 = fdmData.startList
	end2 = fdmData.endList
	link = fdmData.link
	adj = fdmData.adj
	ipTable = fdmData.ipTable
	mptcpList = topology.mptcpList
	udpList = topology.udpList
	nHost = topology.nHost
	requests = topology.requests
	nShip = topology.nShip
	hostShipConnect = topology.hostShipConnect
	shipSat = topology.shipSat

	#udp
	uTable = []
	uFlow = [0] * nL
	for i in range(nL):
		uTable.append({})

	udpDict = OrderedDict()
	for i in range(nHost):
		for j in range(nHost):
			if udpList[i][j] != 0:
				#is a udp demand, i = src, j = desc
	
				udpDict[(i,j)] = udpList[i][j] 	
				#order dict by size of req bw
	for item in udpDict:
		src, des = item
		ucap = udpDict[item]

		srcShip = end2[end1.index(src)]

		#desShip = end1[end2.index(des)]
		avalSatLink = []
		for item in adj[srcShip]:
			if item != -1 and cap[item]>0:
				avalSatLink.append(item)
		greedy = {}
		for shipSatLink in avalSatLink:
			upSat = end2[shipSatLink]
			upLink = adj[upSat][0]
			if cap[upLink] < 0 or cap[upLink] < ucap:
				continue	#discard if not enough cap
			midSat = end2[upLink]
			downLink = adj[midSat][0]
			if cap[downLink] < 0 or cap[downLink] < ucap:
				continue	#discard if not enough cap
			endSat = end2[downLink]

			for lk in adj[endSat]:
				if lk != -1 and end2[lk] == des:
					greedy[(shipSatLink,upLink,downLink,lk)] = \
					min(cap[shipSatLink],cap[upLink],cap[downLink],cap[lk])
		if not greedy:
			print "no feasible"
			return []
		linkList = list(max(greedy, key=greedy.get))
		
		#put in utable
		ip = ipTable[str(src)+" "+ str(end2[linkList[0]])]
		uTable[end1.index(src)][ip] = ucap
		uTable[end2.index(des)][ip] = ucap
		uFlow[end1.index(src)] += ucap
		uFlow[end2.index(des)] += ucap
		for lk in linkList:
			uTable[lk][ip] = ucap
			uFlow[lk] += ucap
			#start decrease cap in the route link
			cap[lk] -= ucap

	req = []
	mmReq = []
	for i in xrange(nN):
		req.append([0]*nN)
		mmReq.append([0]*nN)


	totalReq = 0
	for i in range(nHost):
		for j in range(nHost):
			if requests[i][j] > 0 and udpList[i][j] == 0 :
				req[i][j] = requests[i][j]
				totalReq += requests[i][j]
	count = 0
	feasible = True
	fdLen = [0.0] * nL
	msgLen = 1

	spDist = []
	spPred = []
	for i in range(nN):
		spPred.append([0]*nN)
		spDist.append([0]*nN)
	eFlow = [0.0] * nL
	gFlow = [0.0] * nL
	pFlow = [0.0] * nL
	cost = [0.0] * nL
	gTable =[{}]*nL
	eTable = [{}]*nL
	newCap = [0.0] * nL
	#init	
	setLinkLens(nL,gFlow,uFlow,cap,msgLen,fdLen, cost)
	setSP(nN, link, end2, fdLen, adj, spDist, spPred)
	gFlow, gTable = loadLinks(nN, nL, req, spPred, end1, ipTable)
	aResult = adjustCaps(nL, gFlow, cap, newCap)
	if aResult == 1:
		aFlag = 0
	else:
		aFlag = 1
	currentDelay = calcDelay(nL, gFlow,uFlow, newCap, msgLen,totalReq, cost)
	print "currentDelay" + str(currentDelay)
	raw_input()
	previousDelay = float("inf")
	while  aFlag or (currentDelay < (1-EPS)*previousDelay)  :
		setLinkLens(nL,gFlow,uFlow,newCap,msgLen,fdLen, cost)
		setSP(nN, link, end2, fdLen, adj, spDist, spPred)
		eFlow, eTable = loadLinks(nN, nL, req, spPred, end1,ipTable)
		previousDelay = calcDelay(nL,gFlow,uFlow,newCap,msgLen,totalReq, cost)
		superpose(nL, eFlow, gFlow,uFlow, newCap, totalReq, msgLen, gTable, eTable, cost)
		currentDelay = calcDelay(nL,gFlow,uFlow,newCap,msgLen,totalReq, cost)

		#check feasiblity
		print "currentDelay" + str(currentDelay)
		raw_input()
		if aFlag is True:
			aResult = adjustCaps(nL,gFlow,cap,newCap)
			if aResult == 1:
				aFlag = 0
			else:
				aFlag = 1

		if (aFlag == 1 and currentDelay < (1-EPS)*previousDelay) or count >= 100000:
			print ("not feasible")
			feasible = False
			break
		count+=1

	if feasible:
		print "Finished in " + str(count) + " iteration(s)"
		print "currentDelay" + str(currentDelay)
		totalReqMPTCP = totalReq
		totalReqUDP = 0 
		for item in udpDict:
			totalReqUDP += udpDict[item]
		print "===================================="

		print "Total MPTCP Requiremnet:" +str(totalReqMPTCP)

		desHubLink =end2.index(nHost - 1)
		desHub = end1[desHubLink]	
		for i in range(nL):
		 	if end2[i] == desHub:
		 		endSat = end1[i]
		 		print "gFlow["+str(endSat)+"]" + str(gFlow[endSat]) 	
		print "===================================="
		print "Total UDP Requiremnet:" +str(totalReqUDP)	
		for i in range(nL):
		 	if end2[i] == desHub:
		 		endSat = end1[i]
		 		print "uFlow["+str(endSat)+"]" + str(uFlow[endSat]) 
		return gTable, uTable

	else:
		#reduce request
		#Max-Min algorithm
		desHubLink =end2.index(nHost - 1)
		desHub = end1[desHubLink]	
		totalReqUDP = 0 
		for item in udpDict:
			totalReqUDP += udpDict[item]
		print "===================================="
		print "Total UDP Requiremnet:" +str(totalReqUDP)	
		for i in range(nL):
		 	if end2[i] == desHub:
		 		endSat = end1[i]
		 		print "uFlow["+str(endSat)+"]" + str(uFlow[endSat]) 
		maxReq = max(array(req).max(axis=1))
		minReq = 0
		while (max_req > min_req + 0.1):
			feasible = True
			midReq = minReq + numpy.mean([minReq,maxReq])
			totalReq = 0
			for x in xrange(0,nN):
				 for y in xrange(0,nN):
				 	if req[x][y] > 0:
				 		minReq = min(Req[x][y],midReq)
					 	totalReq += minReq
			previousDelay = float("inf")
			gFlow = [0]*nL	
			setLinkLens(nL,gFlow,uFlow,cap,msgLen,fdLen)
			setSP(nN,link,end2,fdLen,adj,spDist,spPred)
			loadLinks(nN,nl,mmReq,spPred,end1,gFlow,eTable)
			aResult = adjustCaps(nL,gFlow,cap,newCap)
			if aResult ==1:
				aFlag = 0
			else:
				aFlag =1
			currentDelay = calcDelay(nL,gFlow,uFlow,newCap,msgLen,totalReq)
			while currentDelay < (1-EPS)*previousDelay:
				setLinkLens(nL,gFlow,uFlow,cap,msgLen,fdLen)
				setSP(nN,link,end2,fdLen,adj,spDist,spPred)
				loadLinks(nN,nl,mmReq,spPred,end1,eFlow,eTable,ipTable)

				superpose(nL,eFlow,gFlow,newCap,totalReq,msgLen,gTable,eTable)
				currentDelay = calcDelay(nL,gFlow,uFlow,newCap,msgLen,totalReq)
				if aFlag == True:
					aResult = adjustCaps(nL,gFlow,cap,newCap)
					if aResult ==1:
						aFlag = 0
					else:
						aFlag = 1

			if aFlag is True and currentDelay >= previousDelay*(1-EPS) or count >=10000:
				feasible = False
			if feasible is True:
				min_req = mid
			else:
				max_req = mid
		#caculate traffic

		for ship in ships:
			sum_ship = 0
			for link in adj[ship]:
				if gFlow[link] > 0:
					print "Usage at sat ", end2[l]-n_ship,"is",gFlow[link]
					sum_ship += gFlow[link]
			print "Total outgoing flow at ship", ship, "is", sum_ship
		for sat in sats:
			for link in sat:
				print "Total load at sat", s, "is", gFlow[link]


		if feasible:
			return gFlow
		else:
			return None