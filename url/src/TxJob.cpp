#include "TxJob.hpp"
namespace urlsock
{

TxJob::TxJob(const ConstBufferView& buffer, IEndPoint& endpoint, IpPortMessageId ipPortMessage, bool acknowledgedMode,
    uint8_t intProtAlg, uint8_t cipherAlg):
    mMessage(buffer),
    mEndpoint(endpoint),
    mMsgId(ipPortMessage.second),
    mIpPort(ipPortMessage.first),
    mSendLooping(true),
    mAcknowledgedMode(acknowledgedMode),
    mNextOffset(0),
    mRetryCount(0),
    mTimeoutBias(0.0)
{
}


    // const ConstBufferView& mMessage;
    // IEndPoint& mEndpoint;
    // uint16_t mMsgId;
    // IpPort mIpPort;
    // bool mSendLooping;
    // bool mAcknowledgedMode;
    // std::map<uint32_t,TxContext> mTxContextOffsetMap;
    // std::mutex mTxContextLock;
    // std::condition_variable mTxContextCv;
    // uint32_t mNextOffset;
    // uint32_t mRetryCount;
    // double  mTimeoutBias;
    // uint8_t mBufferTx[UDP_MAX_SIZE];

void TxJob::eventAckReceived(uint32_t offset)
{
    std::lock_guard<std::mutex> guard(mTxContextLock);
    auto found = mTxContextOffsetMap.find(offset);
    if (found == mTxContextOffsetMap.end())
    {
        /** TODO: ack not in list**/
        return;
    }
    mTxContextOffsetMap.erase(found);
}

void TxJob::run()
{
    while (mSendLooping)
    {
        {
            std::lock_guard<std::mutex> guard(mTxContextLock);
            if (mTxContextOffsetMap.empty())
            {
                if (!mNextOffset)
                {
                    sendFirst();
                }
                else if (mNextOffset >= mMessage.size())
                {
                    mSendLooping = false;
                    continue;
                }
                else
                {
                    sendBatch();
                }
            }
        }
        scheduledResend();
    }
}

void TxJob::send(UrlPduAssembler& pdu, uint32_t sendSize)
{
    if ((mNextOffset+sendSize) > mMessage.size())
    {
        sendSize = mMessage.size() - mNextOffset;
    }

    BufferView txBuffer(mBufferTx, UDP_MAX_SIZE);
    auto pduRaw = pdu.createFrom(txBuffer);
    mEndpoint.send(pduRaw, mIpPort);
    if (mAcknowledgedMode)
    {
        TxContext context;
        context.mTimeSent = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        context.mSegmentSize = sendSize;
        mTxContextOffsetMap.emplace(std::make_pair(mNextOffset, context));
    }
    mNextOffset += sendSize;
}

void TxJob::sendFirst()
{
    auto sendSize = 300; /** TODO: base this on channel quality**/
    UrlPduAssembler pdu;
    pdu.setInitialDataHeader(mMessage.size(), mMsgId, 0,
        false, mAcknowledgedMode, 0, 0);
    pdu.setPayload(ConstBufferView(mMessage.data()+mNextOffset, sendSize));
    send(pdu, sendSize);
}

void TxJob::sendBatch()
{
    for (auto i=0u; i<10 && mNextOffset<mMessage.size(); i++) /** TODO: base batch size to channel quality**/
    {
        auto sendSize = 300; /** TODO: base this on channel quality**/
        UrlPduAssembler pdu;
        pdu.setDataHeader(mNextOffset, mMsgId, 0);
        pdu.setPayload(ConstBufferView(mMessage.data()+mNextOffset, sendSize));
        send(pdu, sendSize);
    }
}

void TxJob::scheduledResend()
{

}

} // namespace urlsock
