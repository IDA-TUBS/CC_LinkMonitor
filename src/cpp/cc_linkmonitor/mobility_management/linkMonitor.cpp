#include <cc_linkmonitor/mobility_management/linkMonitor.hpp>

namespace cc_linkmonitor {

/* ---------------------------------------------- linkWriter -------------------------------------------------- */
linkWriter::linkWriter(
    UUID_t w_ID,
    socket_endpoint w_socket_addr,
    std::string multicast_group,
    std::chrono::nanoseconds hb_p,
    bool a_run
):
    id(w_ID),
    reader_endpoint(  
        udp::endpoint(
            boost::asio::ip::address::from_string(multicast_group), 
            w_socket_addr.port)
    ),
    writer_endpoint(
        udp::endpoint(
            boost::asio::ip::address::from_string(w_socket_addr.ip_addr), 
            w_socket_addr.port)
    ),
    writer_socket(writer_context),
    link_msg(),
    period(hb_p),
    auto_run(a_run),
    heartbeat_writer{}
{
    writer_socket.open(writer_endpoint.protocol());
    // writer_socket.set_option(udp::socket::reuse_address(true));
    // writer_socket.bind(udp::endpoint(boost::asio::ip::address_v4::any(), 
    //                     w_socket_addr.port));
    writer_socket.bind(writer_endpoint);

    // enable mc forwarding
    writer_socket.set_option(boost::asio::ip::multicast::hops(64));
 
    CC_logInfo("HB writer ID: " << id);
    if(auto_run)
    {
        run();
    }
}

linkWriter::linkWriter(
    UUID_t w_ID,
    socket_endpoint w_socket_addr,
    socket_endpoint r_socket_addr,
    std::chrono::nanoseconds hb_p,
    bool a_run
):
    id(w_ID),
    reader_endpoint(  
        udp::endpoint(
            boost::asio::ip::address::from_string(r_socket_addr.ip_addr), 
            r_socket_addr.port)
    ),
    writer_endpoint(
        udp::endpoint(
            boost::asio::ip::address::from_string(w_socket_addr.ip_addr), 
            w_socket_addr.port)
    ),
    writer_socket(writer_context),
    period(hb_p),
    auto_run(a_run),
    writer_active(false),
    heartbeat_writer{}
{
    writer_socket.open(writer_endpoint.protocol());
    writer_socket.bind(writer_endpoint);

    CC_logInfo("HB writer ID: " << id);
    if(auto_run)
    {
        run();
    }
}

void linkWriter::run()
{
    if(!writer_active)
    {    
        CC_logInfo("HB writer running...")
        heartbeat_writer = std::thread{&linkWriter::schedule_heartbeat, this};
        writer_active = true;
    }
    else
    {
        CC_logInfo("HB writer already running...")
    }
}

void linkWriter::schedule_heartbeat()
{
    // linkMessage link_msg;

    // Update timestamp now to compensate while loop condition check overhead
    auto start = std::chrono::steady_clock::now() - period;
    auto time_stamp = std::chrono::steady_clock::now();
    auto cycle_offset = std::chrono::nanoseconds(0);
    
    while(writer_active)
    {   
        link_msg.update(id);

        send(link_msg); 
        
        // Cycle offset compensation via offset budget
        if(time_stamp - start > period)
        {
            cycle_offset = cycle_offset + (time_stamp - start - period);
        }
        else if(time_stamp - start < period)
        {
            cycle_offset = cycle_offset + (time_stamp - start - period);
        }

        start = time_stamp;

        while(time_stamp < start + period - cycle_offset)
        {
            // Wait
            time_stamp = std::chrono::steady_clock::now();
        }
    }
}

std::size_t linkWriter::send(linkMessage &msg)
{
    std::size_t buf_len = 0;
    MessageNet_t linkMsg;
    msg.linkToNet(&linkMsg);
    buf_len = writer_socket.send_to(boost::asio::buffer(linkMsg.buffer, linkMsg.length), reader_endpoint);
    return buf_len;
}

std::size_t linkWriter::send(linkMessage &msg, udp::endpoint reader)
{
    std::size_t buf_len = 0;
    MessageNet_t linkMsg;
    msg.linkToNet(&linkMsg);
    buf_len = writer_socket.send_to(boost::asio::buffer(linkMsg.buffer, linkMsg.length), reader);
    return buf_len;
}

std::chrono::nanoseconds linkWriter::get_period()
{
    return period;
}

int linkWriter::get_heartbeat_count()
{
    return link_msg.hb_count;
}

void linkWriter::stop()
{
    writer_active = false;
    heartbeat_writer.join();
}

void linkWriter::join()
{
    heartbeat_writer.join();
}

/* ------------------------------------------------------------------------------------------------------------------ */


/* ---------------------------------------------- linkReader -------------------------------------------------- */

linkReader::linkReader(
    UUID_t r_id,
    socket_endpoint r_socket_addr,
    std::chrono::nanoseconds hb_p,
    std::chrono::nanoseconds hb_s,
    int hb_l
):
    reader_context(),
    node_id(r_id),
    multicast_endpoint(),
    reader_endpoint(
        udp::endpoint(boost::asio::ip::address::from_string(r_socket_addr.ip_addr), 
        r_socket_addr.port)
    ),
    reader_socket(reader_context),
    period(hb_p),    
    hb_slack(hb_s),
    hb_loss(hb_l),
    reader_active_flag(false),
    heartbeat_reader{},
    link_monitor{}
{
    CC_logInfo("Scheduling on ID: " << node_id)

    reader_socket.open(reader_endpoint.protocol());
    reader_socket.set_option(udp::socket::reuse_address(true));
    reader_socket.bind(reader_endpoint);
}

udp::endpoint linkReader::init_heartbeat(linkMessage &msg, bool logging)
{
    MessageNet_t reader_buffer;
    size_t ret_val = 0;
    std::chrono::steady_clock::time_point recv_time;

    // Store endpoint
    udp::endpoint sender_endpoint;

    CC_logInfo("Waiting for initial heartbeat...")

    ret_val = reader_socket.receive_from(boost::asio::buffer(reader_buffer.buffer, reader_buffer.max_size), sender_endpoint);
    recv_time = std::chrono::steady_clock::now();

    reader_buffer.length = ret_val;
    msg.netToLink(&reader_buffer);

    CC_logInfo("Sender: " << sender_endpoint.address() << ":" << sender_endpoint.port())

    connection_list.insert(std::make_pair(sender_endpoint.address().to_string(), recv_time));
    link_status_.insert(std::make_pair(sender_endpoint.address().to_string(), true));

    CC_logInfo("Initial heartbeat received. Reader starting...")
    CC_logInfo("Writer ID: " << msg.id << " Count: " << msg.hb_count)
    
    if(!reader_active_flag)
    {
        heartbeat_reader = std::thread{&linkReader::listen_for_heartbeat, this, logging};
        link_monitor = std::thread{&linkReader::link_check, this};
        reader_active_flag = true;
    }

    return sender_endpoint;
}

void linkReader::listen_for_heartbeat(bool logging)
{
    MessageNet_t reader_buffer; 
    linkMessage msg;
    size_t ret_val = 0;
    std::chrono::time_point<std::chrono::steady_clock> recv_time;

    // Store endpoint
    udp::endpoint sender_endpoint;
    
    CC_logInfo("Listening on: " << reader_endpoint.address() << ":" <<reader_endpoint.port())
    
    while(reader_active_flag)
    {
        ret_val = reader_socket.receive_from(boost::asio::buffer(reader_buffer.buffer, reader_buffer.max_size), sender_endpoint);
        recv_time = std::chrono::steady_clock::now();
        
        reader_buffer.length = ret_val;
        msg.netToLink(&reader_buffer);

        if(logging)
        {
            // CC_logDebug("recv hb: " << sender_endpoint.address().to_string() << " hb: " << msg.hb_count << " size: " << ret_val)
        }

        connection_list[sender_endpoint.address().to_string()] = recv_time;

        reader_buffer.clear();
        msg.clear();
        sender_endpoint = udp::endpoint();
    }
}

void linkReader::link_check()
{
    // local vector storing lost links
    std::vector<std::string> lost_links;
    while(true)
    {
        auto checkpoint = std::chrono::steady_clock::now();
        auto loss_threshold = (hb_loss * period) + hb_slack;
        std::chrono::time_point<std::chrono::steady_clock> last_hb;
        
        // Check HB reception of active connections
        for(auto it = connection_list.begin(); it != connection_list.end(); it++)
        {
            // CC_logDebug("Link: " << it->first << "Last: " <<  std::chrono::duration_cast<std::chrono::nanoseconds>(it->second.time_since_epoch()).count())
            last_hb = it->second;
            auto time_passed = checkpoint-last_hb;
            if(time_passed > loss_threshold)
            {
                // CC_logInfo("Connection loss: " << it->first)
                link_status_[it->first] = false;
                lost_links.push_back(it->first);
            }
            else
            {
                link_status_[it->first] = true;
            }
        }

        if(lost_links.size() > 0)
        {
            // CC_logDebug("Lost links: " << lost_links.size())
            link_change_.notify_one();
        }

        // Remove connections with missing HBs
        for(auto it = lost_links.begin(); it != lost_links.end(); it++)
        {
            // CC_logDebug("Removing lost link: " << *it)
            connection_list.erase(*it);
        }

        // Adjust cycle with respect to task execution time
        auto delta = std::chrono::steady_clock::now() - checkpoint;
        auto cycle = period/2 - delta;
        std::this_thread::sleep_for(cycle);

        lost_links.clear();
    }
}

void linkReader::stop()
{
    reader_active_flag = false;
    heartbeat_reader.join();
}

void linkReader::join()
{
    heartbeat_reader.join();
}

std::condition_variable& linkReader::link_change()
{
    return link_change_;
}

linkReader::servingSet& linkReader::link_status()
{
    return link_status_;
}

std::vector<std::string> linkReader::getActiveLinks()
{
    std::vector<std::string> active_links;
    for(auto it = connection_list.begin(); it != connection_list.end(); it++)
    {
        active_links.push_back(it->first);
    }
    
    return active_links;
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------- linkLogger ----------------------------------------------------- */

linkLogger::linkLogger(
    UUID_t r_id,
    socket_endpoint r_socket_addr
):
    logger_context(),
    node_id(r_id),
    multicast_endpoint(),
    logger_endpoint(
        udp::endpoint(boost::asio::ip::address::from_string(r_socket_addr.ip_addr), 
        r_socket_addr.port)
    ),
    logger_socket(logger_context),
    logger_active_flag(false),
    heartbeat_logger{}
{
    CC_logInfo("Scheduling on ID: " << node_id)

    logger_socket.open(logger_endpoint.protocol());
    logger_socket.set_option(udp::socket::reuse_address(true));
    logger_socket.bind(logger_endpoint);
}

void linkLogger::init_heartbeat()
{
    if(!logger_active_flag)
    {
        CC_logInfo("Heartbeat logger running...")
        heartbeat_logger = std::thread{&linkLogger::listen_for_heartbeat, this};
        logger_active_flag = true;
    }
    else
    {
        CC_logInfo("Heartbeat logger already active...")
    }
}

void linkLogger::listen_for_heartbeat()
{
    MessageNet_t reader_buffer; 
    linkMessage msg;
    size_t ret_val = 0;
    std::chrono::time_point<std::chrono::steady_clock> recv_time;

    // Store endpoint
    udp::endpoint sender_endpoint;
    
    CC_logInfo("Listening on: " << logger_endpoint.address() << ":" << logger_endpoint.port())
    
    while(logger_active_flag)
    {
        ret_val = logger_socket.receive_from(boost::asio::buffer(reader_buffer.buffer, reader_buffer.max_size), sender_endpoint);
        recv_time = std::chrono::steady_clock::now();
        
        reader_buffer.length = ret_val;
        msg.netToLink(&reader_buffer);

        CC_logInfo(sender_endpoint.address().to_string() << ", " << msg.id.get_value() << ", " << msg.hb_count )
        CC_logTrace(sender_endpoint.address().to_string() << ", " << msg.id.get_value() << ", " << msg.hb_count )

        reader_buffer.clear();
        msg.clear();
        sender_endpoint = udp::endpoint();
    }
}

void linkLogger::stop()
{
    logger_active_flag = false;
    heartbeat_logger.join();
}

void linkLogger::join()
{
    heartbeat_logger.join();
}


/* ------------------------------------------------------------------------------------------------------------------ */


} // End namespace