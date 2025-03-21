#include <cc_linkmonitor/mobility_management/mobilityClient.hpp>

namespace cc_linkmonitor {

mobilityClient::mobilityClient(
    UUID_t c_id,
    socket_endpoint c_socket_addr
):
    node_id(c_id),
    client_endpoint(
        boost::asio::ip::udp::endpoint(
            boost::asio::ip::address::from_string(c_socket_addr.ip_addr),
            c_socket_addr.port
        )
    ),
    client_socket(client_context),
    active_flag(false),
    recv{},
    link_monitor{}
{    
    client_socket.open(client_endpoint.protocol());
    client_socket.bind(client_endpoint);
}

std::string mobilityClient::init()
{
    MessageNet_t client_buffer; 
    linkStatus msg;
    size_t ret_val = 0;

    // Store endpoint
    boost::asio::ip::udp::endpoint server_endpoint;
    
    CC_logInfo("Listening on: " << client_endpoint.address() << ":" << client_endpoint.port())

    ret_val = client_socket.receive_from(boost::asio::buffer(client_buffer.buffer, client_buffer.max_size), server_endpoint);
    
    client_buffer.length = ret_val;
    msg.netToLink(&client_buffer);

    std::string link = init_status(msg.status_);

    if(active_flag.load() == false)
    {
        CC_logDebug("Starting threads...")
        active_flag.store(true);
        recv = std::thread{&mobilityClient::listen_for_status, this};
        link_monitor = std::thread{&mobilityClient::link_check, this};
    }
    return link;
}

void mobilityClient::listen_for_status()
{
    MessageNet_t client_buffer; 
    linkStatus msg;
    size_t ret_val = 0;

    // Store endpoint
    boost::asio::ip::udp::endpoint server_endpoint;
    
    while(active_flag.load() == true)
    {
        ret_val = client_socket.receive_from(boost::asio::buffer(client_buffer.buffer, client_buffer.max_size), server_endpoint);
        
        client_buffer.length = ret_val;
        msg.netToLink(&client_buffer);

        receive_queue.enqueue(msg);

        client_buffer.clear();
        msg.clear();
        server_endpoint = boost::asio::ip::udp::endpoint();
    }
}

std::string mobilityClient::init_status(linkStatus::status_list status)
{
    std::string first_link;
    bool link_set = false;
    CC_logInfo("Status list: ")

    for(auto it = status.begin(); it != status.end(); it++)
    {
        CC_logInfo("Link: " << it->first << ":" << it->second)

        if(!link_set)
        {
            first_link = it->first;
            link_set = true;
        }
        link_status_[it->first] = it->second;
    }
    return first_link;
}

int mobilityClient::update_status(linkStatus::status_list status)
{
    // local vector storing lost links
    std::vector<std::string> lost_links;

    CC_logInfo("Status list: ")

    for(auto it = status.begin(); it != status.end(); it++)
    {
        CC_logInfo("Link: " << it->first << ":" << it->second)
        auto entry = link_status_.find(it->first);
        if(entry != link_status_.end())
        {
            // Check if a link was lost
            if((entry->second == true) && (it->second == false))
            {
                lost_links.push_back(it->first);
            }
        }
        link_status_[it->first] = it->second;
    }

    CC_logDebug("Lost links: " << lost_links.size())
    return lost_links.size();
}

void mobilityClient::link_check()
{
    linkStatus msg;
    
    while(active_flag.load() == true)
    {
        msg = receive_queue.dequeue();

        if(update_status(msg.status_) > 0)
        {
            link_change_.notify_one();
        }
    }
}

void mobilityClient::stop()
{
    active_flag.store(false);
    mobilityClient::join();
}

void mobilityClient::join()
{
    recv.join();
    link_monitor.join();
}

std::condition_variable& mobilityClient::link_change()
{
    return link_change_;
}

mobilityClient::servingSet& mobilityClient::link_status()
{
    return link_status_;
}


} // End namespace
