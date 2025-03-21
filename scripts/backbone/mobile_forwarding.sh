#!/bin/bash

# ---------------------------------------------------------------------------- #
#   Example configuration for the forwarding rules needed at the mobile node
# ---------------------------------------------------------------------------- #

# Define the interfaces 
WIFI_INTERFACE_1='wlan0'
WIFI_INTERFACE_2='wlan1'
WIFI_IP_1='192.168.20.6'
WIFI_IP_2='192.168.30.6'

AP_IP_1='192.168.20.4'
AP_IP_2='192.168.30.5'

DATA_PORT=55000



#------------------------ IP Route Config ------------------------------------------------------------------------------------
sudo sysctl -w net.ipv4.ip_forward=1

#----------------------- IP Tables Config ------------------------------------------------------------------------------------
# Flush existing iptables rules
sudo iptables -F
sudo iptables -t nat -F
sudo iptables -t mangle -F

#### ----------------------- Data Forwarding ------------------------------------------------------------------------------------
# Add a rule to change the source address of data stream to the mobile node
sudo iptables -t nat -A POSTROUTING -o $WIFI_INTERFACE_2 -d $AP_IP_2 -p udp --dport $DATA_PORT -j SNAT --to-source $WIFI_IP_2

