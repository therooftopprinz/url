#ifndef URL_URLSOCK_HPP_
#define URL_URLSOCK_HPP_

#include <atomic>
#include "IUrlSock.hpp"
#include "UdpEndpoint.hpp"
#include "TxJobManager.hpp"
#include "RxBufferManager.hpp"
#include "RxJob.hpp"

namespace urlsock
{

class UrlSock : public IUrlSock
{
public:
    UrlSock(uint16_t port);
    ESendResult send(const uint8_t* buffer, size_t size, IpPort target);
    ESendResult send(const BufferView& payload, IpPort target);
    ESendResult send(const Buffer& payload, IpPort target);
    EReceiveResult receive(Buffer& payload, IpPort& targeter);
private:
    UdpEndpoint mEndpoint;
    TxJobManager mTxJobManager;
    RxBufferManager mRxBufferManager;
    RxJob mRxJob;
    std::atomic<uint16_t> mMsgId;
};

} // namespace urlsock

#endif
