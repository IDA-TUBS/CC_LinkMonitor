import os

def read_hostapd_conf(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    return lines

def modify_parameters(lines, parameters):
    modified_lines = []
    
    for line in lines:
        modified = False
        for parameter, new_value in parameters.items():
            if line.startswith(parameter):
                modified_lines.append(f'{parameter}={new_value}\n')
                modified = True
                break
        if not modified:
            modified_lines.append(line)
    
    # Check for parameters that were not found and need to be added
    for parameter, new_value in parameters.items():
        if not any(line.startswith(parameter) for line in lines):
            modified_lines.append(f'{parameter}={new_value}\n')
    
    return modified_lines

def write_hostapd_conf(file_path, lines):
    ensures_directories_exists(file_path)
    with open(file_path, 'w') as file:
        file.writelines(lines)

def update_hostapd_conf(template_path, new_conf_path, parameters):
    lines = read_hostapd_conf(template_path)
    modified_lines = modify_parameters(lines, parameters)
    write_hostapd_conf(new_conf_path, modified_lines)
    
def ensures_directories_exists(file_path):
    # Extract the directory path from the file path
    directory_path = os.path.dirname(file_path)
    
    # Check if the directory path exists
    if not os.path.exists(directory_path):
        # Create the directory path along with any necessary intermediate directories
        os.makedirs(directory_path)