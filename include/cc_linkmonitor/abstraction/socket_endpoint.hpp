#ifndef SocketEndpoint_h
#define SocketEndpoint_h

#include <string>
#include <chrono>


namespace cc_linkmonitor{

/**
 * @brief Describes the parameters of an boost asio endpoint. Used for passing endpoints to functions.
 * 
 */
struct socket_endpoint
{
    std::string ip_addr;
    int port;

    socket_endpoint(){};
    socket_endpoint(    
        std::string ip, 
        int p
    ):
        ip_addr(ip),
        port(p)
    {};
};

/**
 * @brief Describes the parameteres of an resource management (rm) endpoint. Every rm endpoint is defined by its own endpoint(tx) and a destination endpoint (rx).
 * The destination endpoint describes the (rm) endpoint in an upper hierarchy level. 
 * Used to define the rm endpoints for the network clients (gateways)
 * 
 */
struct rm_endpoint
{
    std::string rx_ip;
    int rx_port;
    std::string tx_ip;
    int tx_port;

    rm_endpoint(){};

    rm_endpoint(
        std::string r_ip,
        int r_port,
        std::string t_ip,
        int t_port
    ):
        rx_ip(r_ip),
        rx_port(r_port),
        tx_ip(t_ip),
        tx_port(t_port)
    {};
};

/**
 * @brief Describes the network environment for a publishing node. 
 * The source socket endpoint is the nodes own endpoint
 * The target socket enpoint is the destination of the traffic published by the node
 * The rm endpoint describes the nodes own resource management endpoint (tx) and the endpoint of the resource manager to communicate with (rx)
 *
 */
struct network_environment
{
    struct socket_endpoint source;
    struct socket_endpoint target;
    std::string broadcast;

    network_environment(){};

    network_environment(
        std::string source_ip,
        int source_port,
        std::string target_ip,
        int target_port,
        std::string bc_ip
    ):
        source(source_ip, source_port),
        target(target_ip, target_port),
        broadcast(bc_ip)
    {};
};


}; // end namespace rscmng
#endif