# PTPD initialization for V2X demonstrator evaluation

import subprocess
import argparse
import time
import sys
import os
from pathlib import Path

import wireless.wireless_node as ran

LOG_PATH = os.path.join(Path.home(), 'ptp_logs/')

PTPD_MASTER_CMD = 'sudo ptpd -M -i '
PTPD_MASTER_CMD_SUFFIX = ' -d 1 --e2e -r=-7'

PTPD_SLAVE_CMD = 'sudo ptpd -s -i '
PTPD_SLAVE_CMD_SUFFIX = ' -d 1 --e2e'

PTPD_LOG = '-S ' + LOG_PATH + 'ptpd_stat.log'

SETUP_TIME = 10*60 #sec

# Define launch arguments
parser = argparse.ArgumentParser(description='Configure ptpd setup', formatter_class=argparse.RawTextHelpFormatter)
parser.add_argument('-m', '--master', dest='master', default=False, type=bool,
                    help="ptpd master flag")
parser.add_argument('-node', dest='node', default=1, choices=range(1, len(ran.system_list)+1), type=int,
                    help="""specifies on which node the setup script is executed\nAvailable systems:\n\t""" +
                            '\n\t'.join(str(system.index) + ": " + str(system.label) for system in ran.system_list))
launch_options = parser.parse_args()

# Decrement launch argument since it is used as list index
launch_options.node -= 1

# Role independent commands
ptpd_setup_commands = [
    'sudo pkill ptpd',
    'sudo rm -rf ptp_logs/*'
]

# Check which role was assigned to this node and append the respective command
if(launch_options.master):
    ptpd_setup_commands.append(PTPD_MASTER_CMD + ran.system_list[launch_options.node].ethernet_interface + PTPD_MASTER_CMD_SUFFIX + ' ' + PTPD_LOG)
else:
    ptpd_setup_commands.append(PTPD_SLAVE_CMD + ran.system_list[launch_options.node].ethernet_interface + PTPD_SLAVE_CMD_SUFFIX + ' ' + PTPD_LOG)

# Check if the log directory LOG_PATH exists
if not os.path.exists(LOG_PATH):
    # Create the directory
    os.makedirs(LOG_PATH)
    print(f"Directory '{LOG_PATH}' created.")
else:
    print(f"Directory '{LOG_PATH}' already exists.")

# Execute all ptpd commands
for command in ptpd_setup_commands:
    output = subprocess.run(command, shell=True, capture_output=True)
    print("\033[37m" + output.args + "\033[0m")
    if(output.stdout):
        print("\033[32mstdout: %s\033[0m" %output.stdout.decode('utf-8'), end="")
    if(output.stderr):
        print("\033[31mstderr: %s\033[0m" %output.stderr.decode('utf-8'), end="")

time.sleep(SETUP_TIME)

print('PTPD Setup complete')