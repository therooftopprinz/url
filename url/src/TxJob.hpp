#ifndef TXJOB_HPP_
#define TXJOB_HPP_

#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"
#include "ITxJob.hpp"
#include "IEndPoint.hpp"
#include "UrlPduAssembler.hpp"
#include "RxSegmentAssembler.hpp"
#include "TxParameterHelper.hpp"

namespace urlsock
{

struct TxContext
{
    uint64_t mTimeSent;
    uint32_t mSegmentSize;
};

class TxJob : public ITxJob
{
public:
    TxJob(const ConstBufferView& buffer, IEndPoint& endpoint, IpPortMessageId ipPortMessage, bool acknowledgedMode,
        uint8_t intProtAlg, uint8_t cipherAlg, uint32_t mtuSize);
    void eventAckReceived(uint32_t offset);
    void eventNackReceived(uint32_t offset, ENackReason nackReason);
    ESendResult run();
private:
    bool hasSchedulable();
    void send(UrlPduAssembler&, uint32_t);
    void sendFirst();
    void scheduledSend();
    void scheduledResend();
    const ConstBufferView& mMessage;
    IEndPoint& mEndpoint;
    uint16_t mMsgId;
    IpPort mIpPort;
    bool mAcknowledgedMode;
    bool mFirstAcked;
    std::map<uint32_t,TxContext> mTxContextOffsetMap;
    std::mutex mTxContextLock;
    std::condition_variable mTxContextCv;
    bool mAckReceived;
    uint64_t mNearestExpiry;
    uint32_t mNextOffset;
    std::atomic<uint32_t> mRetryCount;

    const uint32_t MTU_SIZE; 
    const uint32_t MIN_UACK_PACKET = 1;
    const uint32_t MAX_UACK_PACKET = 15;
    const uint32_t MAX_TIMEOUT_WINDOW_SIZE = 3000; // 5s
    const uint32_t MIN_TIMEOUT_WINDOW_SIZE = 100; // 2ms
    const uint32_t MIN_NMTU = 1;
    const uint32_t MAX_NMTU = 44;

    TxParameterHelper mTxParameterHelper;
    uint8_t mBufferTx[UDP_MAX_SIZE];
};

} // namespace urlsock

#endif
