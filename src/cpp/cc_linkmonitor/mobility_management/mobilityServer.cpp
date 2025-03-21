#include <cc_linkmonitor/mobility_management/mobilityServer.hpp>

namespace cc_linkmonitor {

mobilityServer::mobilityServer(
    UUID_t s_id,
    socket_endpoint s_socket_addr,
    ConnectionManager::ip_map map,
    SharedMap<std::string, bool> &link_status
):
    node_id(s_id),
    server_endpoint(
        udp::endpoint(
            boost::asio::ip::address::from_string(s_socket_addr.ip_addr),
            s_socket_addr.port
        )
    ),
    server_socket(server_context),
    map_(map),
    link_status_(link_status)
{
    server_socket.open(server_endpoint.protocol());
    server_socket.bind(server_endpoint);
}

void mobilityServer::init(std::string endpoint_tx, int port)
{
    report_status(endpoint_tx, port);
}

void mobilityServer::report_status(std::string endpoint_tx, int port)
{
    linkStatus status_msg;
    linkStatus::status_list status = from_map(link_status_);

    status_msg.update(node_id, status);

    send(status_msg);
}

void mobilityServer::send(linkStatus &msg)
{
    std::size_t buf_len = 0;
    MessageNet_t linkMsg;
    msg.linkToNet(&linkMsg);

    CC_logDebug("[mobilityServer::send] msg len: " << linkMsg.length)

    for(auto it = map_.begin(); it != map_.end(); it++)
    {
        udp::endpoint target = udp::endpoint(
            boost::asio::ip::address::from_string(it->second),
            server_endpoint.port()
        );
        buf_len = server_socket.send_to(boost::asio::buffer(linkMsg.buffer, linkMsg.length), target);
    }
}

linkStatus::status_list mobilityServer::from_map(SharedMap<std::string, bool>& link_status)
{
    linkStatus::status_list status;
    CC_logInfo("Creating Status list: ")

    for(auto it = link_status.begin(); it != link_status.end(); it++)
    {
        CC_logInfo("Link: " << it->first << ":" << it->second)
        status.push_back(std::make_pair(it->first, it->second));
    }
    return status;
}

} // End namespace
