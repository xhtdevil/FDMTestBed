# R Analysis of SAT-NAV Networks
# Network performance analysis
# 2017-Nov-28

# set path

rm( list = ls (all = TRUE));
#data <- read.csv("result.csv", header = T, stringsAsFactors = FALSE, sep=",")
ana <- function(){
  nameLst <- paste("result", 0:9, sep = "");
  nameLst <- paste(nameLst, ".csv", sep = "");
  
  data <- rep(0,10);
  for (i in seq(1,length(nameLst))){
    data_tmp <- read.csv(nameLst[i], header = T, stringsAsFactors = FALSE, sep=",");
    data_tmp <- data_tmp[,-1];
    nn <- colnames(data_tmp);
    data_tmp <- rbind(nn,data_tmp);
    data <- cbind(data, data_tmp);
  }
  data <- data[,-1];
  rownames(data) <- c("host","total_time","total_packets","min_delay","max_delay","avg_delay",
                      "avg_jitter","sd_delay","avg_bit_rate","avg_pkt_rate");
  data <- t(data)
  req <- c(10,10,8,6,6);
  reqss <- rep(req,10);
  
  data <- as.data.frame(data)
  data$avg_bit_rate <- as.numeric(as.character(data$avg_bit_rate));
  data$avg_delay <- as.numeric(as.character(data$avg_delay));
  
  data$avg_bit_rate <- data$avg_bit_rate * 1024 / 1000 / 1000 / reqss;
  data_compr <- data[,c(1,6,9)];
  return(data_compr);
}

form_sat <- function(data){
  data <- tapply(data$sat,data$host,mean);
  data <- as.data.frame(data);
  data <- cbind(rownames(data),data);
  colnames(data) <- c("host","data");
  return(data);
}

form_delay <- function(data){
  data <- tapply(data$delay,data$host,mean);
  data <- as.data.frame(data);
  data <- cbind(rownames(data),data);
  colnames(data) <- c("host","data");
  return(data);
}

setwd("D://SHARE//tk1rss//single");
single <- ana();
single <- data.frame(single,"single");
colnames(single) <- c("host","delay","sat","type");
single_sat <- form_sat(single);
single_sat <- data.frame(single_sat,type="single");
single_delay <- form_delay(single);
single_delay <- data.frame(single_delay,type="single");

setwd("D://SHARE//tk1rss//multi");
multi <- ana();
multi <- data.frame(multi,"multi");
colnames(multi) <- c("host","delay","sat","type");
multi_sat <- form_sat(multi);
multi_sat <- data.frame(multi_sat,type="mptcp");
multi_delay <- form_delay(multi);
multi_delay <- data.frame(multi_delay,type="mptcp");

setwd("D://SHARE//tk1rss//fdm");
fdm <- ana();
fdm <- data.frame(fdm,"fdm");
colnames(fdm) <- c("host","delay","sat","type");
fdm_sat <- form_sat(fdm);
fdm_sat <- data.frame(fdm_sat,type="fdm");
fdm_delay <- form_delay(fdm);
fdm_delay <- data.frame(fdm_delay,type="fdm");

sat_sum <- rbind(single_sat,multi_sat,fdm_sat);
colnames(sat_sum) <- c("host","sat","type");

delay_sum <- rbind(single_delay,multi_delay,fdm_delay);
colnames(delay_sum) <- c("host","delay","type");

# sat plot by host
library(ggplot2);
p <- ggplot(sat_sum, aes(x = host, y = sat, fill = type));
p <- p + geom_bar(position = "dodge", stat = "identity");
p <- p + labs(y = "Satisfaction",x = "Host");
p <- p + ggtitle("Satisfaction Value over host");
ggsave("sat_vl_ov_hst.png",width = 3.5, height = 3.5);
p <- ggplotly(p)
htmlwidgets::saveWidget(p, "sat_vl_ov_hst.html")

# delay plot by host
p <- ggplot(delay_sum, aes(x = host, y = delay, fill = type));
p <- p + geom_bar(position = "dodge", stat = "identity");
p <- p + labs(y = "Delay / s",x = "Host");
p <- p + ggtitle("Delay over Host");
ggsave("dly_vl_ov_hst.png", width = 3.5, height = 3.5);
p <- ggplotly(p)
htmlwidgets::saveWidget(p, "dly_vl_ov_hst.html")