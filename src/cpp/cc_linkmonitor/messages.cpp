#include <cc_linkmonitor/messages.hpp>

using namespace cc_linkmonitor;

// ----------------------------- linkMessage -------------------------------------------------------------------------
linkMessage::linkMessage()
:
    id(),
    time_stamp(),
    hb_count(0)
{}

linkMessage::linkMessage(UUID_t link_id)
:
    id(link_id),
    time_stamp(),
    hb_count(0)
{}

void linkMessage::update(UUID_t link_id)
{
    id = link_id;
    time_stamp = std::chrono::steady_clock::now();
    hb_count++;
}

void linkMessage::linkToNet(MessageNet_t* msg)
{
    msg->add(&id, sizeof(id));
    msg->add(&time_stamp, sizeof(time_stamp));
    msg->add(&hb_count, sizeof(hb_count));
}

void linkMessage::netToLink(MessageNet_t* msg)
{
    msg->read(&id, sizeof(id));
    msg->read(&time_stamp, sizeof(time_stamp));
    msg->read(&hb_count, sizeof(hb_count));
}

void linkMessage::clear()
{
    id = DEFAULT_ID;
    time_stamp = std::chrono::steady_clock::time_point();
    hb_count = 0;
}

// -------------------------------------------------------------------------------------------------------------------

// ----------------------------- linkStatus -------------------------------------------------------------------------
linkStatus::linkStatus()
:
    id(),
    time_stamp(),
    status_()
{}

linkStatus::linkStatus(UUID_t node_id, status_list status)
:
    id(node_id),
    time_stamp(),
    status_(status),
    count(0)
{}

void linkStatus::update(UUID_t link_id, status_list status)
{
    id = link_id;
    time_stamp = std::chrono::system_clock::now();
    status_ = status;
    count++;
}

void linkStatus::linkToNet(MessageNet_t* msg)
{
    msg->add(&id, sizeof(id));
    msg->add(&time_stamp, sizeof(time_stamp));
    msg->add(&count, sizeof(count));
    add_status_list(msg);
}

void linkStatus::netToLink(MessageNet_t* msg)
{
    msg->read(&id, sizeof(id));
    msg->read(&time_stamp, sizeof(time_stamp));
    msg->read(&count, sizeof(count));
    if(read_status_list(msg) < 0)
    {
        CC_logDebug("[linkStatus::netToLink] read_status_list returned with error")
    }
}


int linkStatus::add_status_list(MessageNet_t* msg)
{
    // Add the size of the vector
    uint32_t size = status_.size();
    if (msg->add(&size, sizeof(size)) != 0)
    {
        CC_logError("[linkStatus::add_status_list] adding vector size failed: " << msg->pos)
        return -1; // Not enough space
    }

    // Add each pair
    for (auto it = status_.begin(); it != status_.end(); it++)
    {
        // Serialize the string length and data
        uint32_t str_len = it->first.size();        
        if (msg->add(&str_len, sizeof(str_len)) != 0 ||
            msg->add(it->first.c_str(), str_len) != 0)
        {
            CC_logError("[linkStatus::add_status_list] adding string failed: " << msg->pos)
            return -1; // Not enough space
        }

        // Serialize the boolean
        if (msg->add(&it->second, sizeof(it->second)) != 0)
        {
            CC_logError("[linkStatus::add_status_list] adding boolean failed: " << msg->pos)
            return -1;  // Not enough space
        }
    }

    return 0; // Success
}

int linkStatus::read_status_list(MessageNet_t* msg)
{
    // Clear the target vector
    status_.clear();

    // Read the size of the vector
    uint32_t size;
    if (msg->read(&size, sizeof(size)) != 0)
    {
        CC_logError("[linkStatus::read_status_list] reading vector size failed: " << msg->pos)
        return -1; // Error reading size
    }

    // Read each pair
    for (uint32_t i = 0; i < size; ++i)
    {
        // Read the string length
        uint32_t str_len;
        if (msg->read(&str_len, sizeof(str_len)) != 0)
        {
            CC_logError("[linkStatus::read_status_list] reading string length failed: " << msg->pos)
            return -1; // Error reading string length
        }

        // Read the string data
        std::string str(str_len, '\0');
        if (msg->read(&str[0], str_len) != 0)
        {
            CC_logError("[linkStatus::read_status_list] reading string data failed: " << msg->pos)
            return -1; // Error reading string data
        }

        // Read the boolean
        bool value;
        if (msg->read(&value, sizeof(value)) != 0)
        {
            CC_logError("[linkStatus::read_status_list] reading boolean failed: " << msg->pos)
            return -1; // Error reading boolean
        }

        // Add the pair to the vector
        status_.emplace_back(std::move(str), value);
    }
    return 0; // Success
}

void linkStatus::clear()
{
    id = DEFAULT_ID;
    time_stamp = std::chrono::system_clock::time_point();
    status_ = status_list();
    count = 0;
}

// -------------------------------------------------------------------------------------------------------------------
