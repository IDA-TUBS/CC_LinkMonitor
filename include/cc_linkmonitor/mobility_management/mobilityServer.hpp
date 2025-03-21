#ifndef CC_MobilityServer_h
#define CC_MobilityServer_h

#include <cc_linkmonitor/mobility_management/connectionManager.hpp>
#include <cc_linkmonitor/abstraction/socket_endpoint.hpp>
#include <cc_linkmonitor/data_sharing/safe_map.hpp>
#include <cc_linkmonitor/messages.hpp>
#include <cc_linkmonitor/log.hpp>

#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <iostream>

namespace cc_linkmonitor {

class mobilityServer
{
    public:

    /**
     * @brief Construct a new mobility Server object
     * 
     * @param s_id The nodes ID
     * @param s_socket_addr client socket address (ip, port)
     * @param map connectin map from connection manager
     * @param link_status link status map
     */
    mobilityServer(
        UUID_t s_id,
        socket_endpoint s_socket_addr,
        ConnectionManager::ip_map map,
        SharedMap<std::string, bool> &link_status
    );

    /**
     * @brief Get the callback object
     * 
     * @return std::function<void(std::string, int)> 
     */
    std::function<void(std::string, int)> get_callback()
    {
        // placeholders only used for compatibility with downlink version which direct accesses the W2RP writer
        return std::bind(&mobilityServer::report_status, this, std::placeholders::_1, std::placeholders::_2);
    }

    /**
     * @brief initialize mobility server
     * 
     */
    void init(std::string endpoint_tx, int port);

    /**
     * @brief report status change to mobility client
     * 
     * @param endpoint_tx new ip
     * @param port new port
     */
    void report_status(std::string endpoint_tx, int port);

    private:

    /**
     * @brief convert link status map to ip map for status message
     * 
     * @param link_status link status container
     * @return linkStatus::ip_map 
     */
    linkStatus::status_list from_map(SharedMap<std::string, bool>& link_status);

    /**
     * @brief Send routine. Sends link status messages via UDP socket
     * 
     */
    void send(linkStatus &msg);

    /**
     * @brief The nodes ID
     * 
     */
    UUID_t node_id;

    /**
     * @brief ip map defining available connections
     * 
     */
    ConnectionManager::ip_map map_;

    /**
     * @brief link status map
     * 
     */
    SharedMap<std::string, bool>& link_status_;

    /**
     * @brief Boost asio IO context for the reader socket
     * 
     */
    boost::asio::io_context server_context;

    /**
     * @brief Socket endpoint for the client socket. Contains the assigned IP and port. 
     * 
     */
    udp::endpoint server_endpoint;

    /**
     * @brief Boost asio socket object 
     * 
     */
    udp::socket server_socket;
};

}; // end namespace cc_linkmonitor

#endif