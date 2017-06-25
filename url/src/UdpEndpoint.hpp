#ifndef UDPENDPOINT_HPP_
#define UDPENDPOINT_HPP_

#include "IEndPoint.hpp"

namespace urlsock
{

class UdpEndpoint : public IEndPoint
{
public:
    UdpEndpoint(uint32_t port);
    size_t send(const BufferView& payload, IpPort target);
    size_t send(const Buffer& payload, IpPort target);
    size_t receive(BufferView&& payload, IpPort& from);
    size_t receive(Buffer& payload, IpPort& from);
private:
    int mSockFd;
};

} // namespace urlsock

#endif
