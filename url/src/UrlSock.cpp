#include "Buffer.hpp"
#include "UrlSock.hpp"

namespace urlsock
{

ESendResult UrlSock::send(const BufferView& payload, IpPort target)
{
    return {};
}

ESendResult UrlSock::send(const Buffer& payload, IpPort target)
{
    return {};
}

ESendResult UrlSock::send(Buffer&& payload, IpPort target)
{
    return {};
}

EReceiveResult UrlSock::receive(Buffer& payload, IpPort& targeter)
{
    return {};
}

} // namespace urlsock
