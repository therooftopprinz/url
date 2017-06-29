#include "Buffer.hpp"
#include "UrlSock.hpp"
#include "TxJob.hpp"

namespace urlsock
{

UrlSock::UrlSock(uint16_t port):
    mEndpoint(port),
    mRxJob(mTxJobManager, mRxBufferManager, mEndpoint)
{
}

ESendResult UrlSock::send(const uint8_t* buffer, size_t size, IpPort target)
{
    ConstBufferView pl(buffer, size);
    IpPortMessageId ippmsg = std::make_pair(target, mMsgId++);
    TxJob txj(pl, mEndpoint, ippmsg, true, 0, 0, 1500);
    mTxJobManager.createITxJob(ippmsg, txj);
    auto rv = txj.run();
    mTxJobManager.deleteITxJob(ippmsg);

    return rv;
}

ESendResult UrlSock::send(const BufferView& payload, IpPort target)
{
    return send(payload.data(), payload.size(),target);
}

ESendResult UrlSock::send(const Buffer& payload, IpPort target)
{
    return send(payload.data(), payload.size(),target);
}

EReceiveResult UrlSock::receive(Buffer& payload, IpPort& targeter)
{
    auto rv = mRxBufferManager.dequeue();
    if (!rv.second.size())
    {
        return EReceiveResult::TimeOut;
    }
    payload = std::move(rv.second);
    targeter = rv.first;
    return EReceiveResult::Ok;
}

} // namespace urlsock
