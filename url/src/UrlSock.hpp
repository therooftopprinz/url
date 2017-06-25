#ifndef URLSOCK_HPP_
#define URLSOCK_HPP_

#include "IUrlSock.hpp"

namespace urlsock
{

class UrlSock : public IUrlSock
{
public:
    ESendResult send(const BufferView& payload, IpPort target);
    ESendResult send(const Buffer& payload, IpPort target);
    ESendResult send(Buffer&& payload, IpPort target);
    EReceiveResult receive(Buffer& payload, IpPort& targeter);
private:
};

} // namespace urlsock

#endif
