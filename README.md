# Continuous Connectivity Linkmonitoring 

The cc_linkmonitoring library provides a proof-of-concept implementation of resource management routines designed for reliable, low-latency streaming of large data objects in wireless networks [1].
This open-source library is intended for researchers, engineers, and developers working on industrial wireless communication and real-time networking.
Unlike state-of-the-art handover solutions, this approach leverages proactive link establishment and fast loss detection to meet reliability and latency constraints without requiring redundant data transmissions.

For more detailed information on how the resource management works, as well as performance evaluation (in simulation and using physical demonstrator setups), please refer to the [LOTUS](https://ida-tubs.github.io/lotus/) project website or the papers listed below.


## Requirements

### Software 
The cc_linkmonitoring library is completely written in C++ and intended to be used on an arbitrary linux host. So far, the library has successfully been deployed on both x86 (Intel) and ARM64 (Nvidia Jetson) - both running Ubuntu 22.04. Deployment on other versions and distribution should not be a problem. All packages required for running cc_linkmonitoring (specific boost version) are bundled with this library.

### Hardware
For working with the cc_linkmonitoring, a minimal setup consisting of four linux hosts is needed:

- 1x *mobile Client* node with at least two wireless interfaces, hosting a linkmonitoring writer in addition to a subscribing/publishing application.
- 1x *Edge Server* node, hosting a linkmonitoring reader in addition to a publishing/subscribing application.
- 2x *Access Point* nodes with one wireless interface each, providing connectivity between edge server and mobile node.

Edge server and access points should be connected via a basic ethernt switch.
The library was tested with 802.11 based interfaces, however none of the functionalities require 802.11-specifi mechanisms.

*Note: a simpler static setup, where connection losses are emulated is possible as well*



## Installation

Clone this repository.

```bash
git clone https://github.com/IDA-TUBS/CC_LinkMonitor
```

From within the cloned repository install boost using the following script (mandatory!)

```bash
sudo ./install_boost.sh
```

Then build and install cc_linkmonitor library:

```bash
sudo ./build.sh -j<n_threads>
```

**Important:** cc_linkmonitor will be installed as a system-wide library. This allows use of cc_linkmonitor in any arbitrary C++ program. Standard practices need to be followed when including cc_linkmonitor (linking in makefile, includes in source code).

### Build configuration

The build scripts offers the possibility to configure different logging options.

* **LOG=ON** enables logging statements
* **CONSOLE=ON** Adds a console log
* **FILE=ON** enables file logging

#### Debug build type

Using -DCMAKE_BUILD_TYPE=Debug, logging will be enabled and a console log is supplied by default

#### Logging

For logging boost/log is employed. 
The console log will display logging messages for all severity levels besides "trace"
The file log only logs "trace" messages



## Features

- uplink and downlink implementations
- fast end-to-end loss detection through custom heartbeat protocol
- end-to-end path switching through destination address change 
- Application integration through API callback


## Overview and Usage

To use cc_linkmonitoring, one needs to host a *linkReader* on the edge node and at least two *linkWriters* on the mobile node (one per interface).
Furthermore a *connectionManager* instance is needed for monitoring available paths, and configuring the active path.
For uplink transmissions, where a publisher is hosted on the mobile node, a *mobility server* on the edge node and a *mobility client* on the mobile node are employed to forward path switch events.
An example can be found in `examples/`


### Edge Node
#### Add a linkReader:
```cpp
/* --------------- create linkReader for connection monitoring -------------- */
cc_linkmonitor::linkReaderConfig_t link_conf(
    /*HB Socket interface*/ "192.168.2.100",
    /*HB Socket Port */     50000,
    /*HB Period*/           hb_period,
    /*HB Slack*/            std::chrono::microseconds(2000),
    /*Max HB loss*/         2  
);

cc_linkmonitor::linkReader link_reader(
    node_id,
    link_conf.socket,
    link_conf.period,
    link_conf.slack,
    link_conf.loss
);
```
For the HB protocol, the period, the allowed jitter (hb slack) and the number of lost heartbeats to trigger a path switch, can be configured.

#### Add a connection manager:
```cpp
/* ------------------ create ip map for connection manager ------------------ */
/**
 * @brief Add AP, mobile Node ip pairs to the ipmap
 * @param pair.first AP ethernet interface (connection from RM to AP)
 * @param pair.second mobileNode wifi interface (connection from AP to mobileNode)
 */
cc_linkmonitor::ConnectionManager::ip_map map;
map.insert(std::make_pair("192.168.2.102", "192.168.20.6"));
map.insert(std::make_pair("192.168.2.103", "192.168.30.6"));

std::function<void(std::string, int)> writer_callback;
/* -------------------------------------------------------------------------- */
/* ----------------- assign function to writer_callback here ---------------- */
/* -------------------------------------------------------------------------- */

cc_linkmonitor::ConnectionManager manager(
    map,
    writer_callback,
    link_reader.link_change(),
    link_reader.link_status()
);
```
The `ip_map` stores ip pairs of the AP's ethernet interface and the connected mobile node interface.
The `link_change` variable is a condition variable used to trigger a path switch event.
The `link_status` map stores the status of the paths (available, lost).
And the writer_callback is used to trigger a destination address change in the publishing application.

#### Add a mobility server
Only needed for uplink transmission mode (mobile node -> edge server)
```cpp
/* ------------------------- create mobility server ------------------------- */
cc_linkmonitor::socket_endpoint mobility_server_endpoint(
    std::string("0.0.0.0"),
    40000
);
cc_linkmonitor::mobilityServer server(
    node_id,
    mobility_server_endpoint,
    map,
    link_reader.link_status()
);
```
The mobility server sends the `link_status` lists via UDP to the mobility client.
The `ip_map` is used to determine the targets for the `link_status` messages


### mobile Node
#### Add linkWriter instances
```cpp
/* -------------- Create link writers for every wifi interface -------------- */
/* ------------------------------ Link Writer 1 ----------------------------- */
// Assemble ip
std::string node_ip = ip_base + std::to_string(node_suffix);
// Define HB writer config
cc_linkmonitor::linkWriterConfig_t link_conf(
    /*HB Socket interface*/ node_ip,
    /*Reader Addr */        "192.168.20.4",
    /*HB Socket Port */     50000,
    /*HB Period*/           hb_period
);

/* ------------------------------ Link Writer 2 ----------------------------- */
std::string node_ip_2 = "192.168.30." + std::to_string(node_suffix);
cc_linkmonitor::linkWriterConfig_t link_conf_2 = link_conf;
link_conf_2.socket.ip_addr = node_ip_2;
link_conf_2.reader.ip_addr = "192.168.30.5";

cc_linkmonitor::linkWriter link_writer(
    node_id, 
    link_conf.socket,
    link_conf.reader,
    link_conf.period, 
    false
);

cc_linkmonitor::linkWriter link_writer2(
    node_id, 
    link_conf_2.socket,
    link_conf_2.reader,
    link_conf_2.period, 
    false
);

link_writer.run();
link_writer2.run();
```
In the examples, the node suffix is porvided as launch argument.


#### Add a mobility client
*Only needed for uplink transmission mode (mobile node -> edge server)*
```cpp
/* --------------------------- Mobility Client --------------------------- */
cc_linkmonitor::socket_endpoint mobility_client_endpoint(
    std::string("0.0.0.0"),
    40000
);
cc_linkmonitor::mobilityClient client(
    node_id,
    mobility_client_endpoint
);
```
The mobility client updates the `link_status` list and the `link_change` variable according to the messages provided by the mobility server.

##### Add a connection manager
*Only needed for uplink transmission mode (mobile node -> edge server)*
```cpp
/* --------------------------- Connection Manager --------------------------- */
// create ip map for connection manager
cc_linkmonitor::ConnectionManager::ip_map map;
map.insert(std::make_pair("192.168.2.102", "192.168.20.4"));
map.insert(std::make_pair("192.168.2.103", "192.168.30.5"));

std::function<void(std::string, int)> writer_callback;
/* -------------------------------------------------------------------------- */
/* ----------------- assign function to writer_callback here ---------------- */
/* -------------------------------------------------------------------------- */

cc_linkmonitor::ConnectionManager manager(
    map,
    writer_callback,
    client.link_change(),
    client.link_status()
);
```
The client side connection manager is identical to the server side connection manager.


### Network configuration

The link monitoring mechanism requires specific forwarding rules at the edge node and AP nodes.
Example configurations can be found in `scripts/backbone`.

Additionally, example configurations for the wireless interfaces can be found in `scripts/wireless`.
These include both configurations for AP mode and IBSS mode.

## References

[1] Daniel Tappe, Alex Bendrick and Rolf Ernst, "**Continuous multi-access communication for high-resolution low-latency V2X sensor streaming**",  In *2024 IEEE Intelligent Vehicles Symposium (IV)*, 2024, June. <https://doi.org/10.24355/dbbs.084-202405020845-0>

[2] Daniel Tappe, Alex Bendrick and Rolf Ernst, "**Continuous Streaming in Roaming Scenarios: AModel Truck-Based Demonstration**", *arXiv preprint arXiv:n.n*, 2025. <https://doi.org/n.n/arXiv.n.n>

[3] Daniel Tappe, Alex Bendrick and Rolf Ernst, "**Enabling Continuous Low Latency Streaming in Industrial Roaming Scenarios**", *arXiv preprint arXiv:n.n*, 2025. <https://doi.org/n.n/arXiv.n.n>

## Contact

Daniel Tappe (tappe@ida.ing.tu-bs.de)

Alex Bendrick (bendrick@ida.ing.tu-bs.de)

When using or referencing the continuous streaming approach in scientific works we kindly ask you reference the papers mentioned above.

## Acknowledgement

<table>
  <tr>
    <td>
        <a href="https://www.dfg.de/en">
            <img src="docs/dfg_logo_weiss.png" alt="dfg_logo" width="100"></td>
        </a>
    </td>
    <td>
        This work was supported by the Deutsche Forschungsgemeinschaft (DFG, German Research Foundation) under Grant ER168/35-1.
    </td>
  </tr>
</table>
