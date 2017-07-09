#include "UdpEndpoint.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <cstring>
#include <unistd.h>

namespace urlsock
{

UdpEndpoint::UdpEndpoint(uint32_t port):
    mSendFlag(MSG_DONTWAIT),
    mRecvFlag(MSG_DONTWAIT),
    mLogger("UdpEndpoint")
{
    if ((mSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw std::runtime_error("cannot create socket");
    }

    mLogger << LOG_DEBUG <<" open " << port << " -> " << mSockFd;

    sockaddr_in myaddr;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(port);

    if (int t=bind(mSockFd, (sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
        std::string err = "bind failed: "+std::to_string(t)+" errorno: "+strerror(errno);
        throw std::runtime_error(err);
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(mSockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
}

UdpEndpoint::~UdpEndpoint()
{
    mLogger << LOG_DEBUG << " ~ " << mSockFd;
    if (mSockFd>=0)
    {
        mLogger << LOG_DEBUG << " close " << mSockFd;
        close(mSockFd);
    }
}

UdpEndpoint::UdpEndpoint(UdpEndpoint&& other):
    mSendFlag(other.mSendFlag),
    mRecvFlag(other.mRecvFlag),
    mLogger("UdpEndpoint")
{
    if (mSockFd>=0)
    {
        close(mSockFd);
    }
    mSockFd = other.mSockFd;
    other.mSockFd = -1;
}

UdpEndpoint& UdpEndpoint::operator=(UdpEndpoint&& other)
{
    if (mSockFd>=0)
    {
        close(mSockFd);
    }
    mSockFd = other.mSockFd;
    other.mSockFd = -1;
    return *this;
}

void UdpEndpoint::orRecvFlag(int flag)
{
    mRecvFlag |= flag; 
}
void UdpEndpoint::andRecvFlag(int flag)
{
    mRecvFlag &= flag; 
}

void UdpEndpoint::orSendFlag(int flag)
{
    mSendFlag |= flag; 
}
void UdpEndpoint::andSendFlag(int flag)
{
    mSendFlag &= flag; 
}


size_t UdpEndpoint::send(const BufferView& payload, IpPort target)
{
    if (!payload.size())
    {
        return 0;
    }
    sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_port = htons(portFromIpPort(target));
    to.sin_addr.s_addr = ipFromIpPort(target);
    std::memset(to.sin_zero, 0, sizeof  to.sin_zero);  
    return sendto(mSockFd, payload.data(), payload.size(), mSendFlag, (sockaddr*)&to, sizeof(to));
}

size_t UdpEndpoint::send(const Buffer& payload, IpPort target)
{
    if (!payload.size())
    {
        return 0;
    }
    sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_port = htons(portFromIpPort(target));
    to.sin_addr.s_addr = ipFromIpPort(target);
    std::memset(to.sin_zero, 0, sizeof  to.sin_zero);  
    return sendto(mSockFd, payload.data(), payload.size(), mSendFlag, (sockaddr*)&to, sizeof(to));
}

size_t UdpEndpoint::receive(BufferView&& payload, IpPort& from)
{
    sockaddr_in framddr;
    socklen_t framddrSz = sizeof(framddr);
    auto sz = recvfrom(mSockFd, payload.data(), payload.size(), mRecvFlag, (sockaddr*)&framddr, &framddrSz);
    from = IpPorter(framddr.sin_addr.s_addr, ntohs(framddr.sin_port));
    return sz;
}

size_t UdpEndpoint::receive(Buffer& payload, IpPort& from)
{
    pollfd sockPoller;
    sockPoller.fd = mSockFd;
    sockPoller.events = POLLIN;

    if (poll(&sockPoller, 1, 1000)<=0)
    {
        return 0;
    }

    if (!sockPoller.revents & POLLIN)
    {
        return 0;
    }

    int nbytes;
    ioctl(mSockFd,FIONREAD,&nbytes);
    if (!nbytes)
    {
        return 0;
    }

    if (uint32_t(nbytes)!=payload.size())
    {
        payload = Buffer(nbytes);
    }

    sockaddr_in framddr;
    socklen_t framddrSz = sizeof(framddr);
    recvfrom(mSockFd, payload.data(), nbytes, mRecvFlag, (sockaddr*)&framddr, &framddrSz);
    from = IpPorter(framddr.sin_addr.s_addr, framddr.sin_port);
    return nbytes;
}

} // namespace urlsock
