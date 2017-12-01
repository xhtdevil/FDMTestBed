greedy<-data.frame(read.csv("result9_greedy.csv"))
greedy<-greedy[5,-1]
rownames(greedy)=c("Greedy")

mu<-data.frame(read.csv("result9_mu.csv"))
mu<-mu[5,-1]
rownames(mu)=c("MU")

um<-data.frame(read.csv("result9_um.csv"))
um<-um[5,-1]
rownames(um)=c("UM")

alldelay<-rbind(greedy,mu,um)
mptcphost<-seq(1,9,2)
udphost<-seq(2,10,2)
mptcp<-alldelay[,mptcphost]
udp<-alldelay[,udphost]
udp<-apply(udp,1,mean)
mptcp<-apply(mptcp,1,mean)

greedy2<-data.frame(read.csv("result9_greedy_modi.csv"))
greedy2<-greedy2[5,-1]
rownames(greedy2)=c("Greedy")

mu2<-data.frame(read.csv("result9_mu_modi.csv"))
mu2<-mu2[5,-1]
rownames(mu2)=c("MU")

um2<-data.frame(read.csv("result9_um_modi.csv"))
um2<-um2[5,-1]
rownames(um2)=c("UM")

alldelay2<-rbind(greedy2,mu2,um2)
mptcp2<-alldelay2[,mptcphost]
udp2<-alldelay2[,udphost]
udp2<-apply(udp2,1,mean)
mptcp2<-apply(mptcp2,1,mean)

udp<-data.frame(udp)
algorithm<-rownames(udp)
udp<-data.frame(algorithm,udp)
testcase<-rep("original",3)
data<-cbind(udp,testcase)
colnames(data)[2]<-c("delay_value") 
udp2<-data.frame(udp2)
udp2<-data.frame(algorithm,udp2)
testcase<-rep("modified",3)
data2<-cbind(udp2,testcase)
colnames(data2)[2]<-c("delay_value") 
data<-rbind(data,data2)

library("ggplot2")
r <- ggplot(data=data, aes(x = algorithm, y=delay_value, fill =testcase))+ geom_bar(stat = "identity",position="dodge")
r <- r + labs(title = "Average UDP Delay in original and modified Testcase9", x = "Algorithm", y = "Average UDP Delay /s")
r
ggsave("UDP Delay Comparison.png")



mptcp<-data.frame(mptcp)
algorithm<-rownames(mptcp)
mptcp<-data.frame(algorithm,mptcp)
testcase<-rep("original",3)
data<-cbind(mptcp,testcase)
colnames(data)[2]<-c("delay_value") 
mptcp2<-data.frame(mptcp2)
mptcp2<-data.frame(algorithm,mptcp2)
testcase<-rep("modified",3)
data2<-cbind(mptcp2,testcase)
colnames(data2)[2]<-c("delay_value") 
data<-rbind(data,data2)



g <- ggplot(data=data, aes(x = algorithm, y=delay_value, fill =testcase))+ geom_bar(stat = "identity",position="dodge")
g <- g + labs(title = "Average MPTCP Delay in original and modified Testcase9", x = "Algorithm", y = "Average MPTCP Delay /s")
g
ggsave("MPTCP Delay Comparison.png")