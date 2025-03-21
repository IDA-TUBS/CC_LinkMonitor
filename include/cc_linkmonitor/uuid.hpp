#ifndef CC_Guid_h
#define CC_Guid_h

#include <string>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cc_linkmonitor
{

const unsigned char DEFAULT_ID_CHAR[] = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"; 

const uint8_t VENDOR_ID_OFFSET = 0;
const uint8_t HOST_ID_OFFSET = 2;
const uint8_t PROCESS_ID_OFFSET = HOST_ID_OFFSET;

const uint8_t VENDOR_ID_LEN = 2;
const uint8_t HOST_ID_LEN = 2;
const uint8_t PROCESS_ID_LEN = 6;

/**
 * @brief GUID for applications / entities
 * 
 * Encoding:
 * Bytes 0-1: VendorID
 * Bytes 2-3: HostID
 * Bytes 4-11: ProcessID
 * Bytes 13-15: Entity ID (Reader/Writer ...)
 * 
 */
struct UUID_t
{
    static constexpr unsigned int size = 16;
    
    unsigned char value[size];

    UUID_t()
    {
        memset(value, 0xFF, size);
    };

    UUID_t(unsigned char val[size])
    {
        memcpy(value, val, size);
    }

    UUID_t(const unsigned char val[size])
    {
        memcpy(value, val, size);
    }


    /**
     * Guid prefix comparison operator
     * @param entity_id entity_id to compare
     * @return True if the entity_ids are equal
     */
    bool operator ==(
            const UUID_t& entity_id) const
    {
        return (memcmp(value, entity_id.value, size) == 0);
    }

    /**
     * Guid prefix comparison operator
     * @param entity_id Second guid prefix to compare
     * @return True if the entity_ids are not equal
     */
    bool operator !=(
            const UUID_t& entity_id) const
    {
        return (memcmp(value, entity_id.value, size) != 0);
    }

    /**
     * Guid prefix minor operator
     * @param entity_id Second entity_id to compare
     * @return True if entity_id is higher
     */
    bool operator <(
            const UUID_t& entity_id) const
    {
        return std::memcmp(value, entity_id.value, size) < 0;
    }

    /**
     * Checks whether this guid is for an entity on the same host as another guid.
     *
     * @param other_guid UUID_t to compare to.
     *
     * @return true when this guid is on the same host, false otherwise.
     */
    bool is_on_same_host_as(
            const UUID_t& other_guid) const
    {
        return memcmp((value+HOST_ID_OFFSET), (other_guid.value+HOST_ID_OFFSET), VENDOR_ID_LEN) == 0;
    }

    /**
     * Checks whether this guid is for an entity on the same host and process as another guid.
     *
     * @param other_guid UUID_t to compare to.
     *
     * @return true when this guid is on the same host and process, false otherwise.
     */
    bool is_on_same_process_as(
            const UUID_t& other_guid) const
    {
        return memcmp((value+PROCESS_ID_OFFSET), (other_guid.value+PROCESS_ID_OFFSET), PROCESS_ID_LEN) == 0;
    }

    uint16_t get_host()
    {
        return ((static_cast<uint16_t>(value[HOST_ID_OFFSET]) << 8) + static_cast<uint16_t>(value[HOST_ID_OFFSET+HOST_ID_LEN-1]));
    }

    uint32_t get_prefix()
    {
        return(
            (static_cast<uint32_t>(value[VENDOR_ID_OFFSET] << 24) + 
            static_cast<uint32_t>(value[VENDOR_ID_OFFSET+VENDOR_ID_LEN-1] << 16) + 
            static_cast<uint32_t>(value[HOST_ID_OFFSET] << 8) +
            static_cast<uint32_t>(value[HOST_ID_OFFSET+HOST_ID_LEN-1]))
        );
    }

    void overwrite(void* val, unsigned long val_size)
    { 
        // Reverse byte order of value so that the LSB of val is at value[15]!
        uint8_t* p = static_cast<uint8_t*>(val);
        std::reverse(p, p + val_size);
        memcpy(value+size-val_size, val, val_size);
    }

    int get_value(size_t offset, void* val, unsigned long length)
    {
        if(offset+length <= size)
        {
            memcpy(val, (value+offset), length);
            return 0;    
        }
        else
        {
            return -1;
        }
    }

    std::string get_value()
    {
        std::stringstream stream;
        stream << std::hex;
        for (uint8_t i = 0; i < 15; ++i)
        {
            stream << std::setw(1) << (int)value[i];
        }
        stream << std::setw(1) << (int)value[15];

        return stream.str();
    }

};

const UUID_t DEFAULT_ID(DEFAULT_ID_CHAR);

/**
 * @brief Output stream for Application GUID
 * 
 * @param output 
 * @param id 
 * @return std::ostream& 
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const UUID_t& id)
{
    output << std::hex;
    char old_fill = output.fill('0');
    for (uint8_t i = 0; i < 15; ++i)
    {
        output << std::setw(2) << (int)id.value[i] << ".";
    }
    output << std::setw(2) << (int)id.value[15];
    output.fill(old_fill);
    return output << std::dec;
}

/**
 * @brief Input stream for Application GUID
 * 
 * @param input 
 * @param id 
 * @return std::istream& 
 */
inline std::istream& operator >>(
        std::istream& input,
        UUID_t& id)
{
    std::istream::sentry s(input);

    if (s)
    {
        char point;
        unsigned short hex;
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);
            input >> std::hex >> hex;

            if (hex > 255)
            {
                input.setstate(std::ios_base::failbit);
            }

            id.value[0] = static_cast<unsigned char>(hex);

            for (int i = 1; i < 16; ++i)
            {
                input >> point >> hex;
                if ( point != '.' || hex > 255 )
                {
                    input.setstate(std::ios_base::failbit);
                }
                id.value[i] = static_cast<unsigned char>(hex);
            }

            input >> std::dec;
        }
        catch (std::ios_base::failure& )
        {
        }

        input.exceptions(excp_mask);
    }

    return input;
}

}; // end namespace cc_linkmonitor

#endif