#ifndef CC_MessageNet_h
#define CC_MessageNet_h

#include <cc_linkmonitor/messages.hpp>
#include <cc_linkmonitor/log.hpp>



namespace cc_linkmonitor {

#ifndef CC_MAX_MSG_LENGTH
#define CC_MAX_MSG_LENGTH
enum { max_msg_length = 1472 };
#endif

/**
 * @brief Storage struct for serialized message 
 * 
 */
struct MessageNet_t
{
    /**
	 * @brief Construct a new MessageNet_t object
	 * 
	 */
	MessageNet_t()
		: MessageNet_t(max_msg_length)
  	{
  	}

	/**
	 * @brief Destroy the MessageNet_t object
	 * 
	 */
	~MessageNet_t()
	{
        if(buffer != nullptr && allocated)
        {
            delete buffer;
        }
	}

    MessageNet_t(const MessageNet_t& msg)
    {
        buffer = new char[msg.max_size];
        memcpy(buffer, msg.buffer, msg.length);
        pos = msg.pos;
        length = msg.length;
        max_size = msg.max_size;

        allocated = true;
    };

    MessageNet_t& operator=(const MessageNet_t& msg)
    {
        this->buffer = new char[msg.max_size];
        memcpy(buffer, msg.buffer, msg.length);
        this->pos = msg.pos;
        this->length = msg.length;
        this->max_size = msg.max_size;

        allocated = true;
    
        return *this;
    };

	/**
	 * @brief Construct a new MessageNet_t object with given size
	 * 
	 * @param size 
	 */
	explicit MessageNet_t(uint32_t size)
	{
		pos = 0;
		length = 0;

		if(size != 0)
		{
			buffer = new char[size];
            allocated = true;
		}
		else
		{
			buffer = nullptr;
		}

		max_size = size;
	};

    /**
     * @brief Wrap given byte stream in MessageNet_t object
     * 
     * @param stream 
     * @param size 
     */
    MessageNet_t(char* stream, uint32_t size, uint32_t max_size)
    {
        buffer = stream;
        max_size = max_size;
        length = size;
        pos = 0;

        allocated = false;
    }

    /**
     * @brief Add generic payload to message
     * 
     * @param val generic data object
     * @param size size of the data object
     * @return int (0): success (-1): not enough space
     */
    int add(void* val, unsigned long size)
    {
        if(pos+size < max_size)
        {
            memcpy(buffer+pos, val, size);
            pos += size;
            length += size;
            return 0;
        }
        else
        {
            CC_logError("[MessageNet_t::read] write size surpasses buffer length")
            return -1;
        }
    }

        /**
     * @brief Add generic payload to message
     * 
     * @param val generic data object
     * @param size size of the data object
     * @return int (0): success (-1): not enough space
     */
    int add(const void* val, unsigned long size)
    {
        if(pos+size < max_size)
        {
            memcpy(buffer+pos, val, size);
            pos += size;
            length += size;
            return 0;
        }
        else
        {
            CC_logError("[MessageNet_t::read] write size surpasses buffer length")
            return -1;
        }
    }

    /**
     * @brief Update buffer parameters after external assignment (to be used for BOOST ASIO socket reads)
     * 
     * @param size 
     */
    void update(uint32_t size)
    {
        pos = size;
        length = size;
    }
    
    /**
     * @brief clear the content of the message object and reset the read/write head
     * 
     */
    void clear()
    {
        memset(buffer, 0, length);
        pos = 0;
        length = 0;
    }

    /**
     * @brief Reset read/write head
     * 
     */
    void reset()
    {
        pos = 0;
    }

    /**
     * @brief Partial Deserialization of the message
     * 
     * @param val Target variabel
     * @param size Sizeof of the variable to be read
     * @return int (0): success (-1): overflow 
     */
    int read(void* val, unsigned long size)
    {
        if(pos+size <= length)
        {
            memcpy(val, (buffer+pos), size);
            pos += size;
            return 0;    
        }
        else
        {
            CC_logError("[MessageNet_t::read] read size surpasses buffer length")
            return -1;
        }
    }

    // Attributes
	/**
	 * @brief pointer to serialized char array
	 * 
	 */
	char* buffer;

	/**
	 * @brief Read or write position
	 * 
	 */
	uint32_t pos;

	/**
	 * @brief max buffer size
	 * 
	 */
	uint32_t max_size;

	/**
	 * @brief current length of the message
	 * 
	 */
	uint32_t length;

    /**
     * @brief Flag to indicate whether memory has been allocated dynamically
     * 
     */
    bool allocated;
};

} // end namespace cc_linkmonitor

#endif 