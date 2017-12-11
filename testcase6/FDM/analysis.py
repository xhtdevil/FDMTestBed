# !/bin/env/python

from subprocess import call, check_call, check_output
import sys
import time
import os.path
import string
import datetime
os.chdir("/home/osboxes/D-ITG-2.8.1-r1023/bin/")

def extractStat(sndList, recvList):
    '''
        Extract statistic information from log files
    '''
    # decode of sender files
    for i in range(0, len(sndList)):
        initialFileName = "sender" + str(sndList[i]) + ".log"
        decodedSenderFile = "sender" + str(sndList[i]) + ".txt"
        os.system("./ITGDec " + str(initialFileName) + " > " + str(decodedSenderFile))

    # decode of receiver files
    for i in range(0, len(sndList)):
        initialFileName = "receiver" + str(sndList[i]) + "ss" + str(recvList[i]) + ".log"
        decodedRecvFile = "receiver" + str(sndList[i]) + "ss" + str(recvList[i]) + ".txt"
        os.system("./ITGDec " + str(initialFileName) + " > " + str(decodedRecvFile))


def statAnalysis(sndList, recvList):
    '''
        Analysis the decoded files
    '''
    time = datetime.datetime.now()
    fileName = "result_" + str(time) + ".csv"
    g = open("/home/osboxes/FDMTestBed/testcase6/FDM/" + fileName, "w")
    str_ini = 'host,'

    # sender host list
    for i in range(0,len(sndList)):
        str_ini = str_ini + "host" + str(sndList[i])
        if(i < len(sndList) - 1):
            str_ini = str_ini + ','
        else:
            str_ini = str_ini + '\n'

    # initial values
    total_time = 'total_time,'
    total_packets = 'total_packets,'
    min_delay = 'min_delay,'
    max_delay = 'max_delay,'
    avg_delay = 'avg_delay,'
    avg_jitter = 'avg_jitter,'
    sd_delay = 'sd_delay,'
    avg_bit_rate = 'avg_bit_rate,'
    avg_pkt_rate = 'avg_pkt_rate,'

    for i in range(0, len(sndList)):
        decodedRecvFile = "receiver" + str(sndList[i]) + "ss" + str(recvList[i]) + ".txt"
        flag = 0
        f = open(decodedRecvFile,"r")
        if(i < len(sndList) - 1):
            str_split = ","
        else:
            str_split = '\n'

        # read separate files
        while 1:
            line = f.readline()

            if not line:
                break

            if(line.find("TOTAL RESULTS") != -1):
                flag = 1

            total_time = findnSeek(flag, line, "Total time", total_time, str_split)
            total_packets = findnSeek(flag, line,"Total packets", total_packets, str_split)
            min_delay = findnSeek(flag, line, "Minimum delay", min_delay, str_split)
            max_delay = findnSeek(flag, line, "Maximum delay", max_delay, str_split)
            avg_delay = findnSeek(flag, line, "Average delay", avg_delay, str_split)
            avg_jitter = findnSeek(flag, line, "Average jitter", avg_jitter, str_split)
            sd_delay = findnSeek(flag, line, "Delay standard", sd_delay, str_split)
            avg_bit_rate = findnSeek(flag, line, "Average bitrate", avg_bit_rate, str_split)
            avg_pkt_rate = findnSeek(flag, line, "Average packet", avg_pkt_rate, str_split)

    # write results to files
    g.write(str_ini)
    g.write(total_time)
    g.write(total_packets)
    g.write(min_delay)
    g.write(max_delay)
    g.write(avg_delay)
    g.write(avg_jitter)
    g.write(sd_delay)
    g.write(avg_bit_rate)
    g.write(avg_pkt_rate)
    g.close()

def findnSeek(flag, line, findStr,result, str_split):
    if (flag == 1 and line.find(findStr) != -1):
        result = result + line.split('=')[1].strip().split(" ")[0]
        result = result + str_split
        return(result)
    else:
        result = result + '' # do nothing
        return(result)

if __name__ == '__main__':
    # sender list
    sndList = [0, 1, 3, 4, 6, 7, 9, 10, 12, 13]
    # receiver list
    recvList = [11, 14, 2, 8, 5, 11, 5, 8, 2, 11]

    extractStat(sndList, recvList)
    statAnalysis(sndList, recvList)
