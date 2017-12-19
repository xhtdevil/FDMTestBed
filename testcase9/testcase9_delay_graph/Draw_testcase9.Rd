single<-data.frame(read.csv("singletcp.csv"))
single<-single[5,-1]
rownames(single)=c("SingleTCP")

mptcp<-data.frame(read.csv("mptcp.csv"))
mptcp<-mptcp[5,-1]
rownames(mptcp)=c("MPTCP")

mu<-data.frame(read.csv("mu.csv"))
mu<-mu[5,-1]
rownames(mu)=c("MPTCP+FDM")

alldelay<-rbind(single,mptcp,mu)
mptcphost<-seq(1,9,2)
udphost<-seq(2,10,2)
mptcp<-alldelay[,mptcphost]
udp<-alldelay[,udphost]
udp<-apply(udp,1,mean)
mptcp<-apply(mptcp,1,mean)

single2<-data.frame(read.csv("singletcp_modi.csv"))
single2<-single2[5,-1]
rownames(single2)=c("SingleTCP")

mptcp2<-data.frame(read.csv("mptcp_modi.csv"))
mptcp2<-mptcp2[5,-1]
rownames(mptcp2)=c("MPTCP")

mu2<-data.frame(read.csv("mu_modi_q10.csv"))
mu2<-mu2[5,-1]
rownames(mu2)=c("MPTCP+FDM")

alldelay2<-rbind(single2,mptcp2,mu2)
mptcp2<-alldelay2[,mptcphost]
udp2<-alldelay2[,udphost]
udp2<-apply(udp2,1,mean)
mptcp2<-apply(mptcp2,1,mean)

udp<-data.frame(udp)
algorithm<-rownames(udp)
udp<-data.frame(algorithm,udp)
testcase<-rep("testcase9",3)
data<-cbind(udp,testcase)
colnames(data)[2]<-c("delay_value") 
udp2<-data.frame(udp2)
udp2<-data.frame(algorithm,udp2)
testcase<-rep("testcase9_modified",3)
data2<-cbind(udp2,testcase)
colnames(data2)[2]<-c("delay_value") 
data<-rbind(data,data2)

library("ggplot2")
r <- ggplot(data=data, aes(x = algorithm, y=delay_value, fill =testcase))+ geom_bar(stat = "identity",position="dodge")
r <- r + labs(title = "Average UDP Packet Delay in Testcase9 and its Modification", x = "Algorithm", y = "Average UDP Delay /s")
r
ggsave("UDP Delay Comparison.png")

mptcp<-data.frame(mptcp)
algorithm<-rownames(mptcp)
mptcp<-data.frame(algorithm,mptcp)
testcase<-rep("testcase9",3)
data<-cbind(mptcp,testcase)
colnames(data)[2]<-c("delay_value") 
mptcp2<-data.frame(mptcp2)
mptcp2<-data.frame(algorithm,mptcp2)
testcase<-rep("testcase9_modified",3)
data2<-cbind(mptcp2,testcase)
colnames(data2)[2]<-c("delay_value") 
data<-rbind(data,data2)

g <- ggplot(data=data, aes(x = algorithm, y=delay_value, fill =testcase))+ geom_bar(stat = "identity",position="dodge")
g <- g + labs(title = "Average TCP Packet Delay in Testcase9 and its Modification", x = "Algorithm", y = "Average TCP Delay /s")
g
ggsave("MPTCP Delay Comparison.png")