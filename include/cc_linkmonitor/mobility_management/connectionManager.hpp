#ifndef CC_ConnectionManager_h
#define CC_ConnectionManager_h

#include <cc_linkmonitor/mobility_management/linkMonitor.hpp>

#include <condition_variable> 

namespace cc_linkmonitor {

/**
 * @brief 
 * 
 */
class ConnectionManager
{
    public:

    // pair<AP_IP, MOBILE_NODE_IP>
    typedef std::pair<std::string, std::string> dataplane_pair;
    typedef std::map<std::string, std::string> ip_map;

    /**
     * @brief Construct a new Connection Manager object
     * 
     * @param map ip address map
     * @param callback callback function to call on link change
     * @param link_change link change condition variable
     * @param link_status link status map
     */
    ConnectionManager(
        ip_map map,
        std::function<void(std::string, int)> callback,
        std::condition_variable &link_change,
        SharedMap<std::string, bool> &link_status
    );

    /**
     * @brief default destructor
     * 
     */
    ~ConnectionManager();

    /**
     * @brief Initialize Connection manager. Sets initial active pair.
     * 
     * @param link_id std::string defining the initial active pair (wifi IP of initial AP)
     */
    void init(std::string link_id);

    /**
     * @brief Get the active pair object
     * 
     * @return dataplane_pair std::pair<std::string, std::string>
     */
    dataplane_pair get_active_pair();

    /**
     * @brief join wrapper for internal threads
     * 
     */
    void join();

    private:

    /**
     * @brief Connection handling routine. 
     * Upon notification via link_change the routine checks the availability of the data plane connection and switches to another connection if necessary.
     * The connection switch is handled via a supplied callback to application middleware (here: W2RP)
     * 
     */
    void handle_connections();


    protected:

    std::condition_variable& link_change_;
    SharedMap<std::string, bool>& link_status_;

    ip_map map_;
    dataplane_pair active_pair_;

    std::thread connection_handler;

    std::function<void(std::string, int)> callback_;
};

/**
 * @brief ConnectionManager class with added switching delay. Used to emulate the handover delay experienced in classic handover scenarios
 * 
 */
class ConnectionManagerDelay: public ConnectionManager
{
    public:
    
    /**
     * @brief Construct a new Connection Manager Delay object with default delay (1 s)
     * 
     * @param map 
     * @param callback 
     * @param link_change 
     * @param link_status 
     */
    ConnectionManagerDelay(
        ip_map map,
        std::function<void(std::string, int)> callback,
        std::condition_variable &link_change,
        SharedMap<std::string, bool> &link_status
    );

    /**
     * @brief Construct a new Connection Manager Delay object with custom delay
     * 
     * @param map 
     * @param callback 
     * @param link_change 
     * @param link_status 
     * @param switching_delay 
     */
    ConnectionManagerDelay(
        ip_map map,
        std::function<void(std::string, int)> callback,
        std::condition_variable &link_change,
        SharedMap<std::string, bool> &link_status,
        std::chrono::milliseconds switching_delay
    );

    /**
     * @brief Default destructor
     * 
     */
    ~ConnectionManagerDelay();

    /**
     * @brief Initialize Connection manager. Sets initial active pair.
     * 
     * @param link_id std::string defining the initial active pair (wifi IP of initial AP)
     */
    void init(std::string link_id);

    private:

    /**
     * @brief Connection handling routine with added switching delay. 
     * Upon notification via link_change the routine checks the availability of the data plane connection and switches to another connection if necessary.
     * The connection switch is handled via a supplied callback to application middleware (here: W2RP) which is delayed by switching_delay_ milliseconds.
     * 
     */
    void handle_connections();

    std::chrono::milliseconds switching_delay_;
};

}; // end namespace linkmonitor

#endif