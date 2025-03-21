#!/usr/bin/env python
# Script to configure client/station mode

import os
import subprocess
import argparse
import json

import wireless_node as ran
from ap_mode.helper import *

NETWORK_FILE = 'ap_mode/network.json'
WPA_FILE = 'ap_mode/temp_cfg/wpa_supplicant.conf'

# Define launch arguments
parser = argparse.ArgumentParser(description='Configure ad hoc mode for wireless interfaces', formatter_class=argparse.RawTextHelpFormatter)
parser.add_argument('-node', dest='node', default=1, choices=range(1, len(ran.system_list)+1), type=int,
                    help="""specifies on which node the setup script is executed\nAvailable systems:\n\t""" +
                            '\n\t'.join(str(system.index) + ": " +str(system.label) for system in ran.system_list))
parser.add_argument('-m', '--mode', dest='mode', default="COHDA", choices=["COHDA", "CTRL"], type=str,
                    help="""specifies which RAN is configured. \nAvailable options:\n\t""" +
                            "COHDA, CTRL")
launch_options = parser.parse_args()

# Decrement launch argument since it is used as list index
launch_options.node -= 1

# Construct config file paths
script_dir = os.path.dirname(__file__)
network_file_path = os.path.join(script_dir, NETWORK_FILE)
wpa_file_path = os.path.join(script_dir, WPA_FILE)
ensures_directories_exists(wpa_file_path)

# Load config: network.json
# Open and read the JSON file
with open(network_file_path, 'r') as file:
    network_cfg = json.load(file)

# Assign IP and interface\ based on specified RAN
node_if = ''
node_ip = ''
if(launch_options.mode == 'COHDA'):
    node_if = ran.system_list[launch_options.node].wifi_interface
    node_ip = ran.system_list[launch_options.node].wifi_ip
elif(launch_options.mode == 'CTRL'):
    node_if = ran.system_list[launch_options.node].wifi_ctrl_interface
    node_ip = ran.system_list[launch_options.node].wifi_ctrl_ip

# Configure Station/Client
sta_parameters = {
    'interface': node_if,
    'ssid': network_cfg[launch_options.mode]['SSID'],
    'wpa_passphrase': network_cfg[launch_options.mode]['PASSWORD']
}

sta_setup_commands = [
    "ip link set " + node_if + " down",
    "ip a add " + node_ip + "/24 dev " + node_if,
    "ip link set " + node_if + " up",
    "wpa_passphrase " + network_cfg[launch_options.mode]['SSID'] + " " + network_cfg[launch_options.mode]['PASSWORD'] + " | tee " + wpa_file_path,
    "sudo wpa_supplicant -B -i " + node_if + " -c " + wpa_file_path,
    "sleep 5",
    "iw dev"
]

# Execute commands
for command in sta_setup_commands:
    output = subprocess.run(command, shell=True, capture_output=True)
    print("\033[37m" + output.args + "\033[0m")
    if(output.stdout):
        print("\033[32mstdout: %s\033[0m" %output.stdout.decode('utf-8'), end="")
    if(output.stderr):
        print("\033[31mstderr: %s\033[0m" %output.stderr.decode('utf-8'), end="")
