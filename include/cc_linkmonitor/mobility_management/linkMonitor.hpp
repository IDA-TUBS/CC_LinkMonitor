#ifndef CC_LinkMonitor_h
#define CC_LinkMonitor_h

#include <cc_linkmonitor/abstraction/socket_endpoint.hpp>
#include <cc_linkmonitor/data_sharing/safe_map.hpp>
#include <cc_linkmonitor/data_sharing/safe_queue.hpp>
#include <cc_linkmonitor/messages.hpp>
#include <cc_linkmonitor/log.hpp>

#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <iostream>

/**
 * @brief heartbeat-based link monitoring implementation for continuous connectivity / loss-free handover
 * 
 */

namespace cc_linkmonitor {

using boost::asio::ip::udp;

/**
 * @brief Struct defining the configuration parameters of the heartbeat writer
 * 
 */
struct linkWriterConfig_t
{
    socket_endpoint socket;
    socket_endpoint reader;
    std::string multicast_group;
    std::chrono::nanoseconds period;

    // Multicast constructor
    linkWriterConfig_t(
        std::string ip_addr,
        int port,
        std::string mc_addr,
        std::chrono::nanoseconds hb_p
    ):
        socket(ip_addr, port),
        multicast_group(mc_addr),
        period(hb_p)
    {};

    // Unicast constructor
    linkWriterConfig_t(
        std::string ip_addr,
        std::string reader_addr,
        int port,
        std::chrono::nanoseconds hb_p
    ):
        socket(ip_addr, port),
        reader(reader_addr, port),
        multicast_group(),
        period(hb_p)
    {};
};

/**
 * @brief Heartbeat writer to be used by the resource manager
 * 
 */
class linkWriter
{
    public:
    
    /**
     * @brief Used to start the heartbeat writer if auto_run was disabled
     * 
     */
    void run();

    /**
     * @brief Sends periodic heartbeat messages containing the current and next state of the system
     * 
     */
    void schedule_heartbeat();

    /**
     * @brief Broadcast slot message on the control channel (boost ASIO wrapper)
     * 
     * @param msg The scheduling message to be sent
     * @return std::size_t The length of the message sent in bytes (boost ASIO wrapper)
     */
    std::size_t send(linkMessage &msg);

    /**
     * @brief Send slot allocation / deallocation messages on the control channel
     * 
     * @param msg The allocation / deallocation message to send
     * @param reader the endpoint of the affected reader
     * @return std::size_t The length of the message sent in bytes (boost ASIO wrapper)
     */
    std::size_t send(linkMessage &msg, udp::endpoint reader);

    /**
     * @brief Get the period object
     * 
     * @return std::chrono::nanoseconds 
     */
    std::chrono::nanoseconds get_period();

    /**
     * @brief Get the heartbeat count object
     * 
     * @return int heartbeat counter value
     */
    int get_heartbeat_count();

    /**
     * @brief stop the heartbeat scheduling and transmission
     * 
     */
    void stop();

    /**
     * @brief std::thread join() wrapper
     * 
     */
    void join();

    /**
     * @brief Construct a new link Writer object
     * 
     * @param w_ID writer ID
     * @param w_socket_addr Writer endpoint (socket)
     * @param multicast_group mutlicast address
     * @param hb_p heartbeat period
     * @param a_run autorun flag
     */
    linkWriter(    
        UUID_t w_ID,
        socket_endpoint w_socket_addr,
        std::string multicast_group,
        std::chrono::nanoseconds hb_p,
        bool a_run = true
    );

    /**
     * @brief Construct a new link Writer object
     * 
     * @param w_ID writer ID
     * @param w_socket_addr Writer endpoint (socket)
     * @param r_socket_addr reader endpoint (socket)
     * @param hb_p heartbeat period
     * @param a_run autorun flag
     */
    linkWriter(    
        UUID_t w_ID,
        socket_endpoint w_socket_addr,
        socket_endpoint r_socket_addr,
        std::chrono::nanoseconds hb_p,
        bool a_run = true
    );
    
    private:
    
    /**
     * @brief ID of the node running the hb writer
     * 
     */
    UUID_t id; 
    
    /**
     * @brief io context for writer (needed for boost ASIO socket)
     * 
     */
    boost::asio::io_context writer_context;

    /**
     * @brief reader endpoint, defines the destination socket IP and port
     * 
     */
    udp::endpoint reader_endpoint;

    /**
     * @brief writer endpoint, defines the source socket IP and port
     * 
     */
    udp::endpoint writer_endpoint;

    /**
     * @brief writer socket, uesd for scheduling messages (broadcast)
     * 
     */
    udp::socket writer_socket;

    /**
     * @brief linkMessage object used for heartbeat transmission. 
     * 
     */
    linkMessage link_msg;

    /**
     * @brief Period of the heartbeat messages
     * 
     */
    std::chrono::nanoseconds period;

    /**
     * @brief The thread running the schedule writer 
     * 
     */
    std::thread heartbeat_writer;
    
    /**
     * @brief flag indicating whether the writer should start sending heartbeats on object creation or manually triggered by the run method
     * 
     */
    bool auto_run;

    /**
     * @brief flag indicating whether the heartbeat writer and heartbeat scheduling is active
     * 
     */
    bool writer_active;
};



/**
 * @brief Struct defining the configuration parameters of the heartbeat reader
 * 
 */
struct linkReaderConfig_t
{
    socket_endpoint socket;
    std::string multicast_group;
    std::chrono::nanoseconds period;
    std::chrono::nanoseconds slack;
    int loss;

    // Multicast constructor
    linkReaderConfig_t(
        std::string ip_addr,
        int port,
        std::string mc_addr,
        std::chrono::nanoseconds hb_p,
        std::chrono::nanoseconds hb_slack,
        int hb_loss
    ):
        socket(ip_addr, port),
        multicast_group(mc_addr),
        period(hb_p),
        slack(hb_slack),
        loss(hb_loss)
    {};

    // Unicast constructor
    linkReaderConfig_t(
        std::string ip_addr,
        int port,
        std::chrono::nanoseconds hb_p,
        std::chrono::nanoseconds hb_slack,
        int hb_loss
    ):
        socket(ip_addr, port),
        multicast_group(),
        period(hb_p),
        slack(hb_slack),
        loss(hb_loss)
    {};
};

/**
 * @brief Heartbeat reader to be used by the participaiting nodes
 * 
 */
class linkReader
{
  public:
    
    typedef SharedMap<std::string, std::chrono::time_point<std::chrono::steady_clock>> heartbeatLog;
    typedef SharedMap<std::string, bool> servingSet;

    /**
     * @brief Waits for heartbeat messages
     * 
     * @param logging enable/disable logging of heartbeat reception
     */
    void listen_for_heartbeat(bool logging = true);

    /**
     * @brief periodic task to check link availability
     * 
     * @return * void 
     */
    void link_check();

    /**
     * @brief Wait for first slot tick to initiate synchronisation
     * 
     * @param msg the hbMessage containting the initial slot tick
     * @param logging enable/disable logging of heartbeat reception
     * @return udp::endpoint 
     */
    udp::endpoint init_heartbeat(linkMessage &msg, bool logging = true);

    /**
     * @brief stop the heartbeat reader thread 
     * 
     */
    void stop();

    /**
     * @brief std::thread wrapper
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

    /**
     * @brief Construct a new (empty) Schedule reader object
     * 
     */
    linkReader();

    /**
     * @brief Construct a new unicast link reader object
     * 
     * @param r_id The nodes ID
     * @param r_socket_addr reader socket address (ip, port)
     * @param hb_p heartbeat period
     * @param hb_s heartbeat cycle slack (to account for jitter)
     * @param hb_l permitted consecutive loss of heartbeats until fallback
     */
    linkReader(
        UUID_t r_id, 
        socket_endpoint r_socket_addr,
        std::chrono::nanoseconds hb_p,
        std::chrono::nanoseconds hb_s,
        int hb_l
    );

    /**********************************************************************************************/
    /******************** Multicast Constructor -- (!) OUT OF ORDER (!) ***************************/
    // /**
    //  * @brief Construct a new multicast link reader object
    //  * 
    //  * @param r_id The nodes ID
    //  * @param r_socket_addr reader socket address (ip, port)
    //  * @param multicast_group multicast address for link monitoring
    //  * @param connections shared map of available connections
    //  * @param f_trigger fallback trigger
    //  * @param hb_p heartbeat period
    //  * @param hb_s heartbeat cycle slack (to account for jitter)
    //  * @param hb_l permitted consecutive loss of heartbeats until fallback
    //  */
    // linkReader(
    //     UUID_t r_id, 
    //     socket_endpoint r_socket_addr,
    //     std::string multicast_group,
    //     std::condition_variable &loss_trigger,
    //     std::vector<std::string> &lost_links,
    //     std::chrono::nanoseconds hb_p,
    //     std::chrono::nanoseconds hb_s,
    //     int hb_l
    // );
    /**********************************************************************************************/
    /**********************************************************************************************/

    /**
     * @brief return active connection list for gui visualization purposes
     * 
     * @return servingSet containing all active links
     */
    std::vector<std::string> getActiveLinks();

  private:

    /**
     * @brief The nodes ID
     * 
     */
    UUID_t node_id;

    /**
     * @brief slack for the slot interval to accomodate for jitter.
     * 
     */
    std::chrono::nanoseconds hb_slack;

    /**
     * @brief Attribute for allowed hb loss
     * 
     */
    int hb_loss; 

    /**
     * @brief Flag indicating a running listen thread for heartbeat messages
     * 
     */
    bool reader_active_flag;

    /**
     * @brief period for heartbeat reception
     * 
     */
    std::chrono::nanoseconds period;

    /**
     * @brief SharedMap logging latest heartbeat receptions per link
     * 
     */
    heartbeatLog connection_list;

    /**
     * @brief SharedMap storing the status of control plane connections
     *
     */
    servingSet link_status_;

    /**
     * @brief Boost asio IO context for the reader socket
     * 
     */
    boost::asio::io_context reader_context;

    /**
     * @brief multicast group for heartbeat transmission
     * 
     */
    udp::endpoint multicast_endpoint;

    /**
     * @brief Socket endpoint for the reader socket. Contains the assigned IP and port. 
     * 
     */
    udp::endpoint reader_endpoint;

    /**
     * @brief Boost asio socket object 
     * 
     */
    udp::socket reader_socket;
    
    /**
     * @brief The thread for running the schedule reader routine
     * 
     */
    std::thread heartbeat_reader;

    /**
     * @brief The thread for running the link check routine
     * 
     */
    std::thread link_monitor;

    /**
     * @brief CV for triggering the connection loss procedure in the connection manager
     * 
     */
    std::condition_variable link_change_;
};


/**
 * @brief Struct defining the configuration parameters of the heartbeat logger
 * 
 */
struct linkLoggerConfig_t
{
    socket_endpoint socket;
    std::string multicast_group;

    // Multicast constructor
    linkLoggerConfig_t(
        std::string ip_addr,
        int port,
        std::string mc_addr
    ):
        socket(ip_addr, port),
        multicast_group(mc_addr)
    {};

    // Unicast constructor
    linkLoggerConfig_t(
        std::string ip_addr,
        int port
    ):
        socket(ip_addr, port),
        multicast_group()
    {};
};


/**
 * @brief Heartbeat reader to be used by the participaiting nodes
 * 
 */
class linkLogger
{
    public:
    
    /**
     * @brief Waits for heartbeat messages
     * 
     */
    void listen_for_heartbeat();

    /**
     * @brief Wait for first slot tick to initiate synchronisation
     * 
     */
    void init_heartbeat();

    /**
     * @brief stop the heartbeat reader thread 
     * 
     */
    void stop();

    /**
     * @brief std::thread wrapper
     * 
     */
    void join();

    /**
     * @brief Construct a new (empty) Schedule reader object
     * 
     */
    linkLogger();

    /**
     * @brief Construct a new unicast link logger object
     * 
     * @param r_id The nodes ID
     * @param r_socket_addr reader socket address (ip, port)
     */
    linkLogger(
        UUID_t r_id, 
        socket_endpoint r_socket_addr
    );

    /**
     * @brief Construct a new multicast link logger object
     * 
     * @param r_id The nodes ID
     * @param r_socket_addr reader socket address (ip, port)
     * @param multicast_group multicast group address
     */
    linkLogger(
        UUID_t r_id, 
        socket_endpoint r_socket_addr,
        std::string multicast_group
    );

    private:

    /**
     * @brief The nodes ID
     * 
     */
    UUID_t node_id;

    /**
     * @brief Flag indicating a running listen thread for heartbeat messages
     * 
     */
    bool logger_active_flag;

    /**
     * @brief Boost asio IO context for the reader socket
     * 
     */
    boost::asio::io_context logger_context;

    /**
     * @brief multicast group for heartbeat transmission
     * 
     */
    udp::endpoint multicast_endpoint;

    /**
     * @brief Socket endpoint for the reader socket. Contains the assigned IP and port. 
     * 
     */
    udp::endpoint logger_endpoint;

    /**
     * @brief Boost asio socket object 
     * 
     */
    udp::socket logger_socket;
    
    /**
     * @brief The thread for running the schedule reader routine
     * 
     */
    std::thread heartbeat_logger;
};



}; // end namespace linkmonitor
#endif