#ifndef RXJOB_HPP_
#define RXJOB_HPP_

#include <atomic>
#include <map>
#include <thread>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

class ITxJobManager;
class IRxBufferManager;
class IEndPoint;
class RxSegmentAssembler;

struct RxContext
{
    RxSegmentAssembler mRxSegmentAssembler;
    bool mAcknowledgeMode;
    uint8_t mIntProtAlgo;
    uint8_t mCipherAlgo;
    uint8_t key[16];
}

class RxJob
{
public:
    RxJob(ITxJobManager& itxManager, IRxBufferManager& rxBufferManager, IEndPoint& enpoint);
    ~RxJob();
private:
    void receiveThread();

    ITxJobManager& mItxManager;
    IRxBufferManager& mRxBufferManager;
    IEndPoint& mEndpoint;
    std::atomic_bool mReceiving;
    std::thread mReceiveThread;
    std::map<IpPortMessageId, RxContext> mRxContexts;
};

} // namespace urlsock

#endif
