#ifndef CC_MobilityClient_h
#define CC_MobilityClient_h

#include <cc_linkmonitor/abstraction/socket_endpoint.hpp>
#include <cc_linkmonitor/data_sharing/safe_map.hpp>
#include <cc_linkmonitor/data_sharing/safe_queue.hpp>
#include <cc_linkmonitor/messages.hpp>
#include <cc_linkmonitor/log.hpp>

#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>

namespace cc_linkmonitor {

class mobilityClient 
{
    public:
    
    typedef SharedMap<std::string, bool> servingSet;

    /**
     * @brief Construct a new mobility Client object
     * 
     * @param c_id The nodes ID
     * @param c_socket_addr client socket address (ip, port)
     */
    mobilityClient(
        UUID_t c_id,
        socket_endpoint c_socket_addr
    );

    /**
     * @brief initialize mobility client
     * 
     * @return boost::asio::ip::udp::endpoint 
     */
    std::string init();

    /**
     * @brief periodic task to check link availability
     * 
     */
    void link_check();

    /**
     * @brief stop link monitor and recv thread
     * 
     */
    void stop();

    /**
     * @brief std::thread join() wrapper
     * 
     */
    void join();

    /**
     * @brief get link change cv
     * 
     * @return std::condition_variable& 
     */
    std::condition_variable& link_change();
    
    /**
     * @brief get link status list
     * 
     * @return std::vector<std::string>& 
     */
    servingSet& link_status();


    private:
    
    /**
     * @brief receive routine for link status messages
     * 
     */
    void listen_for_status();

    /**
     * @brief init link status map
     * 
     * @param status 
     * @return std::string 
     */
    std::string init_status(linkStatus::status_list status);

    /**
     * @brief Read link status vector and update internal map
     * 
     * @param status link status list provided by link status message
     * @return int 
     */
    int update_status(linkStatus::status_list status);

    /**
     * @brief The nodes ID
     * 
     */
    UUID_t node_id;

    /**
     * @brief flag indicating a running listening thread for link status messages
     * 
     */
    std::atomic<bool> active_flag;

    SafeQueue<linkStatus> receive_queue;

    /**
     * @brief SharedMap storing the status of control plane connections
     *
     */
    servingSet link_status_;

    /**
     * @brief CV for triggering the connection loss procedure in the connection manager
     * 
     */
    std::condition_variable link_change_;

        /**
     * @brief Boost asio IO context for the reader socket
     * 
     */
    boost::asio::io_context client_context;

    /**
     * @brief Socket endpoint for the client socket. Contains the assigned IP and port. 
     * 
     */
    boost::asio::ip::udp::endpoint client_endpoint;

    /**
     * @brief Boost asio socket object 
     * 
     */
    boost::asio::ip::udp::socket client_socket;
    
    /**
     * @brief The thread for running the receive routine
     * 
     */
    std::thread recv;

    /**
     * @brief The thread for running the link check routine
     * 
     */
    std::thread link_monitor;

};

}; // end namespace cc_linkmonitor

#endif