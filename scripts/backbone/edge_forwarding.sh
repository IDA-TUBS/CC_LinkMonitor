#!/bin/bash

# ---------------------------------------------------------------------------- #
#   Example configuration for the forwarding rules needed at the edge node
# ---------------------------------------------------------------------------- #

# Jetson as mobile Node
ip route add 192.168.20.6 via 192.168.2.102 dev enp0s31f6
ip route add 192.168.30.6 via 192.168.2.103 dev enp0s31f6

# wireless2 as mobile Node
ip route add 192.168.20.2 via 192.168.2.102 dev enp0s31f6
ip route add 192.168.30.2 via 192.168.2.111 dev enp0s31f6
