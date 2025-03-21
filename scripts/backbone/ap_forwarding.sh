#!/bin/bash

# ---------------------------------------------------------------------------- #
#   Example configuration for the forwarding rules needed at the access point
# ---------------------------------------------------------------------------- #

# Define the interfaces 
WIFI_INTERFACE='wlp1s0'
ETHERNET_INTERFACE='enp0s31f6' 
LINUX_HOST_IP='192.168.2.102' 
LINUX_AP_IP='192.168.20.4' 
SERVER_IP='192.168.2.100' 
MOBILE_NODE_IP='192.168.20.6'

HB_PORT=50000
DATA_PORT=55000
RM_PORT=40000


#------------------------ IP Route Config ------------------------------------------------------------------------------------
sudo sysctl -w net.ipv4.ip_forward=1

#----------------------- IP Tables Config ------------------------------------------------------------------------------------
# Flush existing iptables rules
sudo iptables -F
sudo iptables -t nat -F
sudo iptables -t mangle -F

#### ----------------------- HB Forwarding ------------------------------------------------------------------------------------
# Add a rule to change the destination address of broadcast heartbeats 
sudo iptables -t nat -A PREROUTING -i $WIFI_INTERFACE -d $LINUX_AP_IP -p udp --dport $HB_PORT -j DNAT --to-destination $SERVER_IP

# Add a rule to change the source address of the heartbeats
sudo iptables -t nat -A POSTROUTING -o $ETHERNET_INTERFACE -d $SERVER_IP -p udp --dport $HB_PORT -j SNAT --to-source $LINUX_HOST_IP

#### ----------------------- Data Forwarding ------------------------------------------------------------------------------------
# Add a rule to change the destination address of nackFrags from the mobile Node
sudo iptables -t nat -A PREROUTING -i $WIFI_INTERFACE -d $LINUX_AP_IP -p udp --dport $DATA_PORT -j DNAT --to-destination $SERVER_IP

# Add a rule to change the source address of data stream to the mobile node
sudo iptables -t nat -A POSTROUTING -o $WIFI_INTERFACE -d $MOBILE_NODE_IP -p udp --dport $DATA_PORT -j SNAT --to-source $LINUX_AP_IP

#### ----------------------- RM Forwarding ------------------------------------------------------------------------------------
# Add a rule to change the destination address of broadcast heartbeats 
sudo iptables -t nat -A PREROUTING -i $WIFI_INTERFACE -d $LINUX_AP_IP -p udp --dport $RM_PORT -j DNAT --to-destination $SERVER_IP

# Add a rule to change the source address of the heartbeats
sudo iptables -t nat -A POSTROUTING -o $ETHERNET_INTERFACE -d $SERVER_IP -p udp --dport $RM_PORT -j SNAT --to-source $LINUX_HOST_IP