#!/bin/sh

sudo tc qdisc del dev $1-eth0 root
sudo tc qdisc add dev $1-eth0 root handle 1: cbq bandwidth 1000Mbit avpkt 1000 cell 8 mpu 64
sudo tc class add dev $1-eth0 parent 1:0 classid 1:1 cbq bandwidth 1000Mbit rate 1000Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit
sudo tc class add dev $1-eth0 parent 1:1 classid 1:2 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:3 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:4 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:5 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:6 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:7 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:8 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:9 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:10 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:11 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:12 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:13 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:14 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:15 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc class add dev $1-eth0 parent 1:1 classid 1:16 cbq bandwidth 1000Mbit rate 10Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 2 flowid 1:2
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 3 flowid 1:3
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 4 flowid 1:4
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 5 flowid 1:5
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 6 flowid 1:6
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 7 flowid 1:7
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 8 flowid 1:8
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 9 flowid 1:9
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 10 flowid 1:10
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 11 flowid 1:11
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 12 flowid 1:12
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 13 flowid 1:13
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 14 flowid 1:14
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 15 flowid 1:15
sudo tc filter add dev $1-eth0 parent 1:0 protocol ip prio 100 route to 16 flowid 1:16
