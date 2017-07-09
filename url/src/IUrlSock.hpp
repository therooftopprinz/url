#ifndef URL_IURLSOCK_HPP_
#define URL_IURLSOCK_HPP_

#include "Buffer.hpp"
#include "RxSegmentAssembler.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

struct IUrlSock
{
    virtual ~IUrlSock() = default;
    virtual ESendResult send(const BufferView& payload, IpPort target) = 0;
    virtual ESendResult send(const Buffer& payload, IpPort target) = 0;
    virtual EReceiveResult receive(Buffer& payload, IpPort& targeter) = 0;
};

} // namespace urlsock

#endif
