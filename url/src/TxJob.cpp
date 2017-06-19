#include <iostream>
#include "TxJob.hpp"
namespace urlsock
{

TxJob::TxJob(const ConstBufferView& buffer, IEndPoint& endpoint, IpPortMessageId ipPortMessage, bool acknowledgedMode,
    uint8_t intProtAlg, uint8_t cipherAlg, uint32_t mtuSize):
    mMessage(buffer),
    mEndpoint(endpoint),
    mMsgId(ipPortMessage.second),
    mIpPort(ipPortMessage.first),
    mAcknowledgedMode(acknowledgedMode),
    mFirstAcked(false),
    mAckReceived(false),
    mNearestExpiry(static_cast<uint64_t>(-1)),
    mNextOffset(0),
    mRetryCount(0),
    MTU_SIZE(mtuSize),
    mTimeoutBias(0.0),
    mMaxConsequentSending(10)
{
}

void TxJob::eventAckReceived(uint32_t offset)
{
    {
        std::lock_guard<std::mutex> guard(mTxContextLock);
        auto found = mTxContextOffsetMap.find(offset);
        if (found == mTxContextOffsetMap.end())
        {
            /** TODO: ack not in list**/
            std::cout << "not in list " << offset << std::endl;
            return;
        }
        if (!offset)
        {
            mFirstAcked = true;
        }
        mTxContextOffsetMap.erase(found);
        mAckReceived = true;
    }
    mTxContextCv.notify_one();
    std::cout << "ack for " << offset << std::endl;
}

bool TxJob::hasSchedulable()
{
    std::lock_guard<std::mutex> guard(mTxContextLock);
    if (!mTxContextOffsetMap.empty() || mNextOffset < mMessage.size())
    {
        return true;
    }
    return false;
}

void TxJob::run()
{
    sendFirst();
    while (hasSchedulable())
    {
        scheduledSend();
        scheduledResend();
        // TODO: Check
    }
}

void TxJob::send(UrlPduAssembler& pdu, uint32_t sendSize)
{
    std::cout << "Sending from " << (void*)(mMessage.data()+mNextOffset) <<
        " with size " << sendSize << std::endl;

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
    if ((mNextOffset+sendSize) > mMessage.size())
    {
        sendSize = mMessage.size() - mNextOffset;
    }

    UrlPduAssembler pdu;
    pdu.setInitialDataHeader(mMessage.size(), mMsgId, 0,
        false, mAcknowledgedMode, 0, 0);
    pdu.setPayload(ConstBufferView(mMessage.data()+mNextOffset, sendSize));
    send(pdu, sendSize);
}

void TxJob::scheduledSend()
{
    std::lock_guard<std::mutex> guard(mTxContextLock);
    if (!mFirstAcked)
    {
        return;
    }
    if (mTxContextOffsetMap.size() >= mMaxConsequentSending)
    {
        return;
    }

    auto nToSend = mMaxConsequentSending-mTxContextOffsetMap.size();
    for (auto i=0u; i<nToSend && mNextOffset<mMessage.size(); i++)
    {
        auto sendSize = 300; /** TODO: base this on channel quality**/
        if ((mNextOffset+sendSize) > mMessage.size())
        {
            sendSize = mMessage.size() - mNextOffset;
        }

        UrlPduAssembler pdu;
        pdu.setDataHeader(mNextOffset, mMsgId, 0);
        pdu.setPayload(ConstBufferView(mMessage.data()+mNextOffset, sendSize));
        send(pdu, sendSize);
    }
}

void TxJob::scheduledResend()
{
    std::unique_lock<std::mutex> lock(mTxContextLock);
    auto twait = 1000u;
    mTxContextCv.wait_for(lock, std::chrono::microseconds(twait),
        [this]()
        {
            return mAckReceived;
        });


    const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    for (auto& i : mTxContextOffsetMap)
    {
        const auto tdiff = now - i.second.mTimeSent;
        if (tdiff > 1000u) /** TODO: timeout based on channel quality**/
        {
            const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            i.second.mTimeSent = now;
            UrlPduAssembler pdu;
            if (!i.first)
            {
                pdu.setInitialDataHeader(i.first, mMsgId, 0,
                    false, mAcknowledgedMode, 0, 0);
            }
            else
            {   
                pdu.setDataHeader(i.first, mMsgId, 0);
            }
            pdu.setPayload(ConstBufferView(mMessage.data()+i.first, i.second.mSegmentSize));
            BufferView txBuffer(mBufferTx, UDP_MAX_SIZE);
            auto pduRaw = pdu.createFrom(txBuffer);
            mEndpoint.send(pduRaw, mIpPort);
            std::cout << "timeout resending " << i.first;
        }
        else if (tdiff < mNearestExpiry)
        {
            mNearestExpiry = now;
        }

        mAckReceived = false;
    }
}

} // namespace urlsock
