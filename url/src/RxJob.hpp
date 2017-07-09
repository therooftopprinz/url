#ifndef URL_RXJOB_HPP_
#define URL_RXJOB_HPP_

#include <atomic>
#include <map>
#include <thread>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"
#include "RxSegmentAssembler.hpp"
#include "Logger.hpp"

namespace urlsock
{

class ITxJobManager;
class IRxBufferManager;
class IEndPoint;
class UrlPduDisassembler;

struct RxContext
{
    RxSegmentAssembler mRxSegmentAssembler;
    uint64_t mLastReceived;
    bool mAcknowledgeMode;
    uint8_t mIntProtAlgo;
    uint8_t mCipherAlgo;
    uint8_t key[16];
};

class RxJob
{
public:
    RxJob(ITxJobManager& itxManager, IRxBufferManager& rxBufferManager, IEndPoint& enpoint);
    ~RxJob();
private:
    void receiveThread();
    inline void processSegmentAssemblerReceived(UrlPduDisassembler& receivedPdu,
        RxSegmentAssembler::EReceivedSegmentStatus rcvState,
        std::map<IpPortMessageId, RxContext>::iterator rxContext, IpPort& senderIpPort);
    ITxJobManager& mItxManager;
    IRxBufferManager& mRxBufferManager;
    IEndPoint& mEndpoint;
    std::atomic_bool mReceiving;
    std::thread mReceiveThread;
    std::map<IpPortMessageId, RxContext> mRxContexts;
    uint8_t mBufferRx[UDP_MAX_SIZE];
    uint8_t mBufferTx[UDP_MAX_SIZE];
    Logger mLogger;
};

} // namespace urlsock

#endif
