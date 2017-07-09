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
    mTxParameterHelper(mtuSize,
        MAX_UACK_PACKET, MIN_UACK_PACKET,
        MAX_TIMEOUT_WINDOW_SIZE, MIN_TIMEOUT_WINDOW_SIZE,
        MAX_NMTU, MIN_NMTU),
    mLogger("TxJob")
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
            mLogger << LOG_ERROR << " ACK NOT IN LIST " << offset;
            return;
        }
        if (!offset)
        {
            mFirstAcked = true;
        }
        mTxContextOffsetMap.erase(found);
        mAckReceived = true;
        mRetryCount = 0;
    }
    mTxContextCv.notify_one();
}

void TxJob::eventNackReceived(uint32_t offset, ENackReason nackReason)
{
}

bool TxJob::hasSchedulable()
{
    std::lock_guard<std::mutex> guard(mTxContextLock);
    return !mTxContextOffsetMap.empty() || mNextOffset < mMessage.size();
}

ESendResult TxJob::run()
{
    sendFirst();
    while (hasSchedulable())
    {
        scheduledSend();
        scheduledResend();
        // TODO: Check
        if (mRetryCount > 10) // TODO: base on configurable max
        {
            return ESendResult::TooManyRetries;
        }
    }
    return ESendResult::Ok;
}

void TxJob::send(UrlPduAssembler& pdu, uint32_t sendSize)
{
    mLogger << LOG_DEBUG << "Sending from " << (void*)(mMessage.data()+mNextOffset) <<
        " with size " << sendSize;

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
    auto sendSize = mTxParameterHelper.getOptimalSegmentSize();
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
    auto maxUackSending = mTxParameterHelper.getOptimalUackPacket();
    if (mTxContextOffsetMap.size() >= maxUackSending)
    {
        return;
    }

    auto nToSend = maxUackSending-mTxContextOffsetMap.size();
    for (auto i=0u; i<nToSend && mNextOffset<mMessage.size(); i++)
    {
        auto sendSize = mTxParameterHelper.getOptimalSegmentSize();
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
        if (tdiff > mTxParameterHelper.getOptimalTimeout())
        {
            const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            i.second.mTimeSent = now;
            UrlPduAssembler pdu;

            auto sendSize = mTxParameterHelper.getOptimalSegmentSize();
            if (sendSize >= i.second.mSegmentSize)
            {
                sendSize = i.second.mSegmentSize;
            }
            else
            {
                TxContext context;
                context.mTimeSent = i.second.mTimeSent;
                context.mSegmentSize = i.second.mSegmentSize-sendSize;
                mTxContextOffsetMap.emplace(std::make_pair(i.first+sendSize, context));
            }

            if (!i.first)
            {
                pdu.setInitialDataHeader(i.first, mMsgId, 0,
                    false, mAcknowledgedMode, 0, 0);
            }
            else
            {   
                pdu.setDataHeader(i.first, mMsgId, 0);
            }
            pdu.setPayload(ConstBufferView(mMessage.data()+i.first, sendSize));
            BufferView txBuffer(mBufferTx, UDP_MAX_SIZE);
            auto pduRaw = pdu.createFrom(txBuffer);
            mEndpoint.send(pduRaw, mIpPort);
            mLogger << LOG_DEBUG << "timeout resending " << i.first;
            mRetryCount++;
        }
        else if (tdiff < mNearestExpiry)
        {
            mNearestExpiry = now;
        }

        mAckReceived = false;
    }
}

} // namespace urlsock
