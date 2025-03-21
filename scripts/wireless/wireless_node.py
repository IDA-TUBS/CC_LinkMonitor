# This file defines the availbe nodes of the V2X Demonstrator for local configuration

WIFI_IP_PREFIX = '192.168.20.'
WIFI_CELL_ID = "AA:AA:AA:AA:AA:AA"
WIFI_CTRL_IP_PREFIX = '192.168.30.'
WIFI_CTRL_CELL_ID = "00:11:22:33:44:55"
ETHERNET_IP_PREFIX = '192.168.2.'
DEFAULT_ROUTER = '192.168.2.200'

# List of all systems available for v2x experiments
system_list = []

# Class representing the parameters need for adhoc configuration of a v2x enable system
class wirelessNode:
    _num_systems = 1
    def __init__(self, label: str ,user: str, wifi_interface: str, wifi_ctrl_interface: str, wifi_ip_suffix: str, ethernet_interface: str, ethernet_ip_suffix: str):
        self.index = wirelessNode._num_systems
        wirelessNode._num_systems += 1
        self.label = label
        self.user = user
        self.wifi_interface = wifi_interface
        self.wifi_ctrl_interface = wifi_ctrl_interface
        self.wifi_ip = WIFI_IP_PREFIX + wifi_ip_suffix
        self.wifi_cell_id = WIFI_CELL_ID
        self.wifi_ctrl_ip = WIFI_CTRL_IP_PREFIX + wifi_ip_suffix
        self.wifi_ctrl_cell_id = WIFI_CTRL_CELL_ID
        self.ethernet_interface = ethernet_interface
        self.ethernet_ip = ETHERNET_IP_PREFIX + ethernet_ip_suffix
        system_list.append(self)
        
# WLTS / DDS Systems
wireless1 = wirelessNode('WirelessSetup1', 'wireless1', 'wlp1s0', 'wlp4s0', '1', 'eno1', '111')
wireless2 = wirelessNode('WirelessSetup2', 'wireless2', 'wlp1s0', 'wlp3s0', '2', 'eno1', '112')

# Demonstrator V2X extension
telemetry = wirelessNode('telemetry_unit', 'telemetry', 'wlp1s0', 'wlp3s0', '3', 'enp0s31f6', '100')
client2 = wirelessNode('client_pc2', 'client', 'wlp1s0', 'wlp3s0', '4', 'enp0s31f6', '102')
client3 = wirelessNode('client_pc3', 'client', 'wlp1s0', 'wlp3s0', '5', 'enp0s31f6', '103')

# RC Truck
truck = wirelessNode('rc_truck', 'truck', 'wlan0', 'wlan1', '6', 'eth0', '55')

# GPS Pi Truck
pi = wirelessNode('gps_pi', 'pi', 'wlan0', 'wlan1', '7', 'eth0', '56')
