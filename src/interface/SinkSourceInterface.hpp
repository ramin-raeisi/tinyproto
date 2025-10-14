#ifndef SINK_SOURCE_INTERFACE_HPP
#define SINK_SOURCE_INTERFACE_HPP

#include <cstdint>

class ISink {
public:
    virtual ~ISink() = default;

    /// @brief Interface for generic send method
    /// @param  
    /// @param numBytesToSend 
    /// @return Number of bytes successfully sent
    virtual int send(uint8_t*, int numBytesToSend) = 0;
};

class ISource {
public:
    virtual ~ISource() = default;

    /// @brief Interface for generic receive method
    /// @param  
    /// @param maxBytesToReceive 
    /// @return Number of Bytes received
    virtual int receive(uint8_t*, int maxBytesToReceive) = 0;
};

#endif // SINK_SOURCE_INTERFACE_HPP