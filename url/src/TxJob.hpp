#ifndef TXJOB_HPP_
#define TXJOB_HPP_

#include <map>
#include <mutex>
#include <condition_variable>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"
#include "ITxJob.hpp"
#include "IEndPoint.hpp"
#include "UrlPduAssembler.hpp"

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
        uint8_t intProtAlg, uint8_t cipherAlg);
    void eventAckReceived(uint32_t offset);
    void run();
private:
    void send(UrlPduAssembler&, uint32_t);
    void sendFirst();
    void sendBatch();
    void scheduledResend();
    const ConstBufferView& mMessage;
    IEndPoint& mEndpoint;
    uint16_t mMsgId;
    IpPort mIpPort;
    bool mSendLooping;
    bool mAcknowledgedMode;
    std::map<uint32_t,TxContext> mTxContextOffsetMap;
    std::mutex mTxContextLock;
    std::condition_variable mTxContextCv;
    uint32_t mNextOffset;
    uint32_t mRetryCount;
    double  mTimeoutBias;
    uint8_t mBufferTx[UDP_MAX_SIZE];
};

} // namespace urlsock

#endif
