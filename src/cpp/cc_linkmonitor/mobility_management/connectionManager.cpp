#include <cc_linkmonitor/mobility_management/connectionManager.hpp>

using namespace cc_linkmonitor;

/* ------------------------------------------------- ConnectionManager ---------------------------------------------------- */
ConnectionManager::ConnectionManager(
    ip_map map,
    std::function<void(std::string, int)> callback,
    std::condition_variable &link_change,
    SharedMap<std::string, bool> &link_status
):
    map_(map),
    callback_(callback),
    link_change_(link_change),
    link_status_(link_status),
    connection_handler{}
{
    
}

ConnectionManager::~ConnectionManager()
{}

void ConnectionManager::init(std::string link_id)
{
    bool match = false;
    for(auto it = map_.begin(); it != map_.end(); it++)
    {
        if(it->first == link_id)
        {
            active_pair_ = *it;
            match = true;
        }
    }

    if(match)
    {
        callback_(active_pair_.second, 55000);
        connection_handler = std::thread{&ConnectionManager::handle_connections, this};
    }
    else
    {
        CC_logError("[ConnectionManager]: " << link_id << " not available in ip map. Active pair could not be set")
    }
}

ConnectionManager::dataplane_pair ConnectionManager::get_active_pair()
{
    return active_pair_;
}

void ConnectionManager::join()
{
    connection_handler.join();
}

void ConnectionManager::handle_connections()
{
    std::mutex con_mutex;
    
    while(true)
    {
        std::unique_lock<std::mutex> lock(con_mutex);
        link_change_.wait(lock);

        CC_logInfo("Handling connection loss")
        
        if(!link_status_[active_pair_.first])
        {
            CC_logInfo("Data plane connection lost: " << active_pair_.first << ", reconfiguring...")

            // Find next available link
            bool reconfigure = false;
            for(auto it = link_status_.begin(); it != link_status_.end(); it++)
            {
                if(it->second)
                {
                    // Reconfigure
                    auto entry = map_.find(it->first);
                    if(entry != map_.end())
                    {
                        active_pair_ = std::make_pair(entry->first, entry->second);
                        reconfigure = true;
                        break;
                    }
                    else
                    {
                        CC_logError("Missing ip map entry for: " << it->first)
                    }
                }
            }
            if(reconfigure)
            {
                callback_(active_pair_.second, 55000);
                CC_logInfo("New Data plane connection: " << active_pair_.second << ":" << 55000)
            }
            else
            {
                CC_logInfo("No other link available. Data plane connection unchanged: " << active_pair_.second << ":" << 55000)
            }
        }
    }
}
/* -------------------------------------------------------------------------------------------------------------------------------- */

/* ----------------------------------------------------- ConnectionManager -------------------------------------------------------- */
ConnectionManagerDelay::ConnectionManagerDelay(
    ip_map map,
    std::function<void(std::string, int)> callback,
    std::condition_variable &link_change,
    SharedMap<std::string, bool> &link_status
):
    ConnectionManager(map, callback, link_change, link_status),
    switching_delay_(1000)
{

}

ConnectionManagerDelay::ConnectionManagerDelay(
    ip_map map,
    std::function<void(std::string, int)> callback,
    std::condition_variable &link_change,
    SharedMap<std::string, bool> &link_status,
    std::chrono::milliseconds switch_delay
):
    ConnectionManager(map, callback, link_change, link_status),
    switching_delay_(switch_delay)
{

}

ConnectionManagerDelay::~ConnectionManagerDelay()
{

}

void ConnectionManagerDelay::init(std::string link_id)
{
    bool match = false;
    for(auto it = map_.begin(); it != map_.end(); it++)
    {
        if(it->first == link_id)
        {
            active_pair_ = *it;
            match = true;
        }
    }

    if(match)
    {
        callback_(active_pair_.second, 55000);
        connection_handler = std::thread{&ConnectionManagerDelay::handle_connections, this};
    }
    else
    {
        CC_logError("[ConnectionManager]: " << link_id << " not available in ip map. Active pair could not be set")
    }
}

void ConnectionManagerDelay::handle_connections()
{
    std::mutex con_mutex;
    
    while(true)
    {
        std::unique_lock<std::mutex> lock(con_mutex);
        link_change_.wait(lock);

        CC_logInfo("Handling connection loss")
        
        if(!link_status_[active_pair_.first])
        {
            CC_logInfo("Data plane connection lost, reconfiguring...")

            // Find next available link
            bool reconfigure = false;
            for(auto it = link_status_.begin(); it != link_status_.end(); it++)
            {
                if(it->second)
                {
                    // Reconfigure
                    auto entry = map_.find(it->first);
                    if(entry != map_.end())
                    {
                        active_pair_ = std::make_pair(entry->first, entry->second);
                        reconfigure = true;
                        break;
                    }
                    else
                    {
                        CC_logError("Missing ip map entry for: " << it->first)
                    }
                }
            }
            if(reconfigure)
            {
                std::this_thread::sleep_for(switching_delay_);
                callback_(active_pair_.second, 55000);
                CC_logInfo("New Data plane connection: " << active_pair_.second << ":" << 55000)
            }
            else
            {
                CC_logInfo("No other link available. Data plane connection unchanged: " << active_pair_.second << ":" << 55000)
            }
        }
    }
}