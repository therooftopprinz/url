#ifndef URLSOCK_HPP_
#define URLSOCK_HPP_

#include "IUrlSock.hpp"

namespace urlsock
{

classs UrlSock : public IUrlSock
{
    ESendResult send(const BufferView& payload, IpPort target);
    ESendResult send(const Buffer& payload, IpPort target);
    ESendResult send(Buffer&& payload, IpPort target);
    EReceiveResult receive(Buffer& payload, IpPort& targeter);
};

} // namespace urlsock

#endif
