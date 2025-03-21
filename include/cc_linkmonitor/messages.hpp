#ifndef CC_Messages_h
#define CC_Messages_h

#include <cc_linkmonitor/data_sharing/message_net.hpp>
#include <cc_linkmonitor/uuid.hpp>
#include <cc_linkmonitor/log.hpp>

#include <string>
#include <chrono>

namespace cc_linkmonitor {

// ----------------------------- linkMessage -------------------------------------------------------------------------
class linkMessage
{
    public:
    /**
     * @brief UUID of the heartbeat sender
     * 
     */
    UUID_t id;

    /**
     * @brief Timestamp of heartbeat generation @source
     * 
     */
    std::chrono::steady_clock::time_point time_stamp;
    
    /**
     * @brief Heartbeat counter
     * 
     */
    uint32_t hb_count;

    /**
     * @brief Length of the message
     * 
     */
    int length =    sizeof(id) +
                    sizeof(time_stamp) +
                    sizeof(hb_count);
    
    /**
     * @brief Construct a new hb Message object
     * 
     */
    linkMessage();

    /**
     * @brief Construct a new hb Message object
     * 
     * @param link_id UUID of the heartbeat sender
     */
    linkMessage(UUID_t link_id);

    /**
     * @brief Update the heartbeat message info
     * 
     * @param link_id UUID of the heartbeat sender
     */
    void update(UUID_t link_id);

    /**
     * @brief Convert heartbeat message to byte stream for transmission
     * 
     * @param msg MessageNet_t struct to store the byte stream
     */
    void linkToNet(MessageNet_t* msg);

    /**
     * @brief Convert heartbeat message byte stream back to heartbeat object
     * 
     * @param msg MessageNet_t struct which holds the byte stream
     */
    void netToLink(MessageNet_t* msg);

    /**
     * @brief Reset the hb message object to default values
     * 
     */
    void clear();
};

// -------------------------------------------------------------------------------------------------------------------

/* --------------------------------------------------- linkStatus -------------------------------------------------- */
class linkStatus
{
    public:
    typedef std::vector<std::pair<std::string, bool>> status_list;

    /**
     * @brief UUID of the heartbeat sender
     * 
     */
    UUID_t id;

    /**
     * @brief Timestamp of message generation @source
     * 
     */
    std::chrono::system_clock::time_point time_stamp;
    
    /**
     * @brief IP address defining the new link to be used
     * 
     */
    status_list status_;

    /**
     * @brief msg count. Used for identifying duplicates due to FRER operation.
     * 
     */
    uint32_t count;

    /**
     * @brief Length of the message
     * 
     */
    int length =    sizeof(id) +
                    sizeof(time_stamp) +
                    sizeof(status_) +
                    sizeof(count);
    
    /**
     * @brief Construct a new link status object
     * 
     */
    linkStatus();

    /**
     * @brief Construct a new link status object
     * 
     * @param link_id UUID of the heartbeat sender
     */
    linkStatus(
        UUID_t node_id,
        status_list status
    );

    /**
     * @brief Update the link status message info
     * 
     */
    void update(
        UUID_t node_id,
        status_list status
    );

    /**
     * @brief Convert heartbeat message to byte stream for transmission
     * 
     * @param msg MessageNet_t struct to store the byte stream
     */
    void linkToNet(MessageNet_t* msg);

    /**
     * @brief Convert heartbeat message byte stream back to heartbeat object
     * 
     * @param msg MessageNet_t struct which holds the byte stream
     */
    void netToLink(MessageNet_t* msg);
    
    /**
     * @brief serialize status list
     * 
     * @param msg MessageNet_t struct which holds the byte stream
     * @return int success (0), error (-1)
     */
    int add_status_list(MessageNet_t* msg);

    /**
     * @brief deserialize status list
     * 
     * @param msg MessageNet_t struct which holds the byte stream
     * @return int success (0), error (-1)
     */
    int read_status_list(MessageNet_t* msg);

    /**
     * @brief Reset the link message object to default values
     * 
     */
    void clear();
};

} // end namespace cc_linkmonitor

#endif