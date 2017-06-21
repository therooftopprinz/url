#include "Buffer.hpp"
#include "RxSegmentAssembler.hpp"

namespace urlsock
{

ESendResult UrlSock::send(const BufferView& payload, IpPort target)
{
}

ESendResult UrlSock::send(const Buffer& payload, IpPort target)
{
}

ESendResult UrlSock::send(Buffer&& payload, IpPort target)
{
}

EReceiveResult UrlSock::receive(Buffer& payload, IpPort& targeter)
{
}

} // namespace urlsock

#endif
