#include <cc_linkmonitor/log.hpp>

void cc_linkmonitor::init_logging()
{
    boost::log::add_common_attributes();
    boost::log::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());
    boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());

    #ifdef CONSOLE_ON
    boost::log::add_console_log
    (
        std::cout,
        // boost::log::keywords::format = "[RMLog][%TimeStamp%][%Severity%]: %Message%",
        boost::log::keywords::format = (
            boost::log::expressions::stream
                << "[RM][" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
                << "]["
                << boost::log::expressions::attr<boost::log::trivial::severity_level>("Severity")
                << "]: "
                << boost::log::expressions::smessage
        ),
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::filter = boost::log::trivial::severity >= boost::log::trivial::debug
    );
    #endif
}

void cc_linkmonitor::init_file_log(std::string log_prefix, std::string log_suffix)
{
    char* home_dir = getenv("HOME");

    std::string log_file = std::string(home_dir) + "/" + log_prefix + log_suffix + ".log";
    init_logging();
    boost::log::add_common_attributes();    

    #ifdef FILE_ON
    boost::log::add_file_log
    (
        boost::log::keywords::file_name = log_file,
        boost::log::keywords::target_file_name = log_file,
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::format = (
            boost::log::expressions::stream
                << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                << ", "
                << boost::log::expressions::smessage
        ),
        boost::log::keywords::filter = boost::log::trivial::severity == boost::log::trivial::trace
    );
    #endif
}

void cc_linkmonitor::init_app_log(std::string log_prefix, std::string log_suffix)
{
    char* home_dir = getenv("HOME");

    std::string log_file = std::string(home_dir) + "/" + log_prefix + log_suffix + ".log";
    init_logging();
    boost::log::add_common_attributes();    

    boost::log::add_file_log
    (
        boost::log::keywords::file_name = log_file,
        boost::log::keywords::target_file_name = log_file,
        boost::log::keywords::auto_flush = true,
        // boost::log::keywords::format = "%TimeStamp%, %Message%",
        boost::log::keywords::format = (
            boost::log::expressions::stream
                << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
                << ", "
                << boost::log::expressions::smessage
                << ", "
        ),
        boost::log::keywords::filter = boost::log::trivial::severity == boost::log::trivial::debug
    );
}

void cc_linkmonitor::init_app_log(std::string log)
{
    char* home_dir = getenv("HOME");
    
    std::string log_file = std::string(home_dir) + "/" + log_file;
    init_logging();
    boost::log::add_common_attributes();    

    boost::log::add_file_log
    (
        boost::log::keywords::file_name = log_file,
        boost::log::keywords::target_file_name = log_file,
        boost::log::keywords::auto_flush = true,
        // boost::log::keywords::format = "%TimeStamp%, %Message%",
        boost::log::keywords::format = (
            boost::log::expressions::stream
                << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
                << ", "
                << boost::log::expressions::smessage
                << ", "
        ),
        boost::log::keywords::filter = boost::log::trivial::severity == boost::log::trivial::debug
    );
}