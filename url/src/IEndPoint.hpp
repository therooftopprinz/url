#ifndef IENDPOINT_HPP_
#define IENDPOINT_HPP_

#include "Buffer.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

struct IEndPoint
{
    virtual ~IEndPoint() = default;
    virtual size_t send(const BufferView& payload, IpPort target) = 0;
    virtual size_t send(const Buffer& payload, IpPort target) = 0;
    virtual size_t send(Buffer&& payload, IpPort target) = 0;
    virtual size_t receive(BufferView&& payload, IpPort& target) = 0;
    virtual size_t receive(Buffer& payload, IpPort& target) = 0;
};

} // namespace urlsock

#endif
