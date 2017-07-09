#ifndef URL_UDPENDPOINT_HPP_
#define URL_UDPENDPOINT_HPP_

#include "IEndPoint.hpp"
#include "Logger.hpp"

namespace urlsock
{

class UdpEndpoint : public IEndPoint
{
public:
    UdpEndpoint(uint32_t port);
    ~UdpEndpoint();
    UdpEndpoint(const UdpEndpoint&) = delete;
    void operator=(const UdpEndpoint&) = delete;
    UdpEndpoint(UdpEndpoint&&);
    UdpEndpoint& operator=(UdpEndpoint&&);
    void orSendFlag(int flag);
    void orRecvFlag(int flag);
    void andSendFlag(int flag);
    void andRecvFlag(int flag);
    size_t send(const BufferView& payload, IpPort target);
    size_t send(const Buffer& payload, IpPort target);
    size_t receive(BufferView&& payload, IpPort& from);
    size_t receive(Buffer& payload, IpPort& from);
private:
    int mSockFd;
    int mSendFlag;
    int mRecvFlag;
    Logger mLogger;
};

} // namespace urlsock

#endif
