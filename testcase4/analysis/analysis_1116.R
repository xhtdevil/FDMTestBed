# R Analysis of SAT-NAV Networks
# Network performance analysis
# 2017-Nov-03

# set path

rm( list = ls (all = TRUE));
#data <- read.csv("result.csv", header = T, stringsAsFactors = FALSE, sep=",")
ana <- function(){
nameLst <- paste("result", 0:9, sep = "");
nameLst <- paste(nameLst, ".csv", sep = "");

data <- rep(0,9);
for (i in seq(1,length(nameLst))){
  data_tmp <- read.csv(nameLst[i], header = T, stringsAsFactors = FALSE, sep=",");
  data <- cbind(data, data_tmp);
}
data <- data[,-1];
rownames(data) <- c("total_time","total_packets","min_delay","max_delay","avg_delay",
                    "avg_jitter","sd_delay","avg_bit_rate","avg_pkt_rate");
data <- t(data)
req <- c(6,4,7,3,4,4,3,3,3,3);
reqss <- rep(req,10);

data <- as.data.frame(data)
sat <- data$avg_bit_rate * 1024 / 1000 / 1000 / reqss;
data <- cbind(data, sat);

# analysis
#delay <- data$avg_delay * data$total_packets / sum(data$total_packets);


return(rbind(sat, data$avg_delay));
}

setwd("D://SHARE2//single");
single <- ana();
setwd("D://SHARE2//multi");
multi <- ana();
setwd("D://SHARE2//fdm");
fdm <- ana();

# single 2.15ms 0.774
# multi 1.6ms 0.768
# fdm 2.4ms 0.765

single <- t(single);
single <- data.frame(single, "single");
colnames(single) <- c("sat","delay","type");
multi <- t(multi);
multi <- data.frame(multi, "multi");
colnames(multi) <- c("sat","delay","type");
fdm <- t(fdm);
fdm <- data.frame(fdm, "fdm");
colnames(fdm) <- c("sat","delay","type");

sat_sum <- rbind(single[,c(1,3)],multi[,c(1,3)],fdm[,c(1,3)])
delay_sum <- rbind(single[,c(2,3)],multi[,c(2,3)],fdm[,c(2,3)]);
delay_sum <- as.data.frame(delay_sum);
delay_sum$delay <- round(delay_sum$delay * 1000,2);

library(ggplot2)
p <- ggplot(sat_sum, aes(x=sat, group = factor(type), colour = factor(type)));
p <- p + geom_density();
p <- p + labs(x = "Satisfactory",y = "Density");
p <- p + scale_x_continuous(limits = c(0.6,0.9))
p <- p + ggtitle("Satisfactory Distribution");
ggsave("ss.jpg")

library(ggplot2)
p <- ggplot(delay_sum, aes(x=delay, group = factor(type), colour = factor(type)));
p <- p + geom_density();
p <- p + labs(x = "Delay/ms",y = "Density");
p <- p + scale_x_continuous(limits = c(0,5));
p <- p + ggtitle("Delay Distribution");
ggsave("ss2.jpg")