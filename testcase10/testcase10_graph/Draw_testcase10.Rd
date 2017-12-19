single<-data.frame(read.csv("single_10.csv"))
single<-single[5,-1]
rownames(single)=c("SingleTCP")

mptcp<-data.frame(read.csv("mptcp_10.csv"))
mptcp<-mptcp[5,-1]
rownames(mptcp)=c("MPTCP")

mu<-data.frame(read.csv("mu_10.csv"))
mu<-mu[5,-1]
rownames(mu)=c("MPTCP+FDM")

alldelay<-rbind(single,mptcp,mu)
mptcphost<-seq(1,9,2)
udphost<-seq(2,10,2)
mptcp<-alldelay[,mptcphost]
udp<-alldelay[,udphost]
udp<-apply(udp,1,mean)
mptcp<-apply(mptcp,1,mean)


udp<-data.frame(udp)
algorithm<-rownames(udp)
udp<-data.frame(algorithm,udp)
testcase<-rep("testcase10",3)
data<-cbind(udp,testcase)
colnames(data)[2]<-c("delay_value") 

library("ggplot2")
r <- ggplot(data=data, aes(x = algorithm, y=delay_value,fill=algorithm))+ geom_bar(stat = "identity",position="dodge")
r <- r + labs(title = "Average UDP Packet Delay in Testcase10", x = "Algorithm", y = "Average UDP Delay /s")
r
ggsave("UDP Delay Comparison.png")

mptcp<-data.frame(mptcp)
algorithm<-rownames(mptcp)
mptcp<-data.frame(algorithm,mptcp)
testcase<-rep("testcase10",3)
data<-cbind(mptcp,testcase)
colnames(data)[2]<-c("delay_value") 

g <- ggplot(data=data, aes(x = algorithm, y=delay_value,fill=algorithm))+ geom_bar(stat = "identity",position="dodge")
g <- g + labs(title = "Average TCP Packet Delay in Testcase10", x = "Algorithm", y = "Average TCP Delay /s")
g
ggsave("MPTCP Delay Comparison.png")