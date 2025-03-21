#include <cc_linkmonitor/mobility_management/linkMonitor.hpp>
#include <cc_linkmonitor/uuid.hpp>

/**
 * @brief Basic downlink client node for cc_linkmonitor
 * 
 * @param argc 
 * @param argv [1] = node ID suffix [2] heartbeat period [us]
 * @return int 
 */
int main(int argc, char* argv[])
{

    // Default configuration
    int node_suffix = 0;
    std::chrono::microseconds hb_period(3000);
    std::string ip_base = "192.168.20.";

    // Check supplied arguments
    if(argc > 1)
    {
        node_suffix = atoi(argv[1]);
    }
    if(argc > 2)
    {
        hb_period = std::chrono::microseconds(atoi(argv[2]));
    }

    // Define ID
    // 00.00.00.00.00.00.00.00.00.00.00.00.00.00.FF.xx
    unsigned char guid[16] = {0x00};
    guid[14] = static_cast<unsigned char>(0xff);
    guid[15] = static_cast<unsigned char>(node_suffix);
    
    cc_linkmonitor::UUID_t node_id(guid);    

    /* -------------- Create link writers for every wifi interface -------------- */
    /* ------------------------------ Link Writer 1 ----------------------------- */
    // Assemble ip
    std::string node_ip = ip_base + std::to_string(node_suffix);
    // Define HB writer config
    cc_linkmonitor::linkWriterConfig_t link_conf(
        /*HB Socket interface*/ node_ip,
        /*Reader Addr */        "192.168.20.4",
        /*HB Socket Port */     50000,
        /*HB Period*/           hb_period
    );

    /* ------------------------------ Link Writer 2 ----------------------------- */
    std::string node_ip_2 = "192.168.30." + std::to_string(node_suffix);
    cc_linkmonitor::linkWriterConfig_t link_conf_2 = link_conf;
    link_conf_2.socket.ip_addr = node_ip_2;
    link_conf_2.reader.ip_addr = "192.168.30.5";

    cc_linkmonitor::linkWriter link_writer(
        node_id, 
        link_conf.socket,
        link_conf.reader,
        link_conf.period, 
        false
    );

    cc_linkmonitor::linkWriter link_writer2(
        node_id, 
        link_conf_2.socket,
        link_conf_2.reader,
        link_conf_2.period, 
        false
    );
    
    link_writer.run();
    link_writer2.run();

    /* ------------------------- Begin main application ------------------------- */
    while(true)
    {
        // dummy while loop for demo
    }
    /* -------------------------- End main application -------------------------- */

    link_writer.stop();
    link_writer2.stop();

    return 0;
}
