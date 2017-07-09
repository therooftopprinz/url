#include "RxJob.hpp"
#include "ITxJobManager.hpp"
#include "IRxBufferManager.hpp"
#include "IEndPoint.hpp"
#include "UrlPduDisassembler.hpp"
#include "UrlPduAssembler.hpp"

namespace urlsock
{

RxJob::RxJob(ITxJobManager& itxManager, IRxBufferManager& rxBufferManager, IEndPoint& endpoint):
    mItxManager(itxManager),
    mRxBufferManager(rxBufferManager),
    mEndpoint(endpoint),
    mReceiving(true),
    mLogger("RxJob")
{
    mReceiveThread = std::thread(&RxJob::receiveThread, this);
}

RxJob::~RxJob()
{
    mReceiving = false;
    mReceiveThread.join();
}

void RxJob::processSegmentAssemblerReceived(UrlPduDisassembler& receivedPdu,
    RxSegmentAssembler::EReceivedSegmentStatus rcvState,
    std::map<IpPortMessageId, RxContext>::iterator rxContext, IpPort& senderIpPort)
{
    if (rcvState == RxSegmentAssembler::EReceivedSegmentStatus::COMPLETE)
    {
        UrlPduAssembler ackPdu;
        BufferView txBuffer(mBufferTx, UDP_MAX_SIZE);
        ackPdu.setAckHeader(receivedPdu.getOffset(), receivedPdu.getMessageId(), 0);
        auto ackPduRaw = ackPdu.createFrom(txBuffer);
        mEndpoint.send(ackPduRaw, senderIpPort);

        auto rxUrlMessageData = rxContext->second.mRxSegmentAssembler.claim();
        mRxBufferManager.enqueue(rxContext->first.first, std::move(rxUrlMessageData));
        mRxContexts.erase(rxContext);
    }
    else if (rxContext->second.mAcknowledgeMode && rcvState == RxSegmentAssembler::EReceivedSegmentStatus::INCOMPLETE)
    {
        UrlPduAssembler ackPdu;
        BufferView txBuffer(mBufferTx, UDP_MAX_SIZE);
        ackPdu.setAckHeader(receivedPdu.getOffset(), receivedPdu.getMessageId(), 0);
        auto ackPduRaw = ackPdu.createFrom(txBuffer);
        mEndpoint.send(ackPduRaw, senderIpPort);
    }
    else if(rxContext->second.mAcknowledgeMode)
    {
        UrlPduAssembler nackPdu;
        BufferView txBuffer(mBufferTx, UDP_MAX_SIZE);
        nackPdu.setAckHeader(receivedPdu.getOffset(), receivedPdu.getMessageId(), 0);
        switch (rcvState)
        {
            case RxSegmentAssembler::EReceivedSegmentStatus::INCORRECT_RTX_DATA:
                nackPdu.setNackInfoHeader(ENackReason::DUPLICATE_SEGMENT_MISMATCHED);
                break;
            case RxSegmentAssembler::EReceivedSegmentStatus::DATA_OUTOFBOUNDS:
                nackPdu.setNackInfoHeader(ENackReason::RECEIVED_SEGMENT_OUT_OF_BOUND);
                break;
            case RxSegmentAssembler::EReceivedSegmentStatus::INCORRECT_RTX_SIZE:
            case RxSegmentAssembler::EReceivedSegmentStatus::DATA_OVERLAPPED:
                nackPdu.setNackInfoHeader(ENackReason::RECEIVED_SEGMENT_OVERLAPPED);
                break;                       
            default:
                nackPdu.setNackInfoHeader(ENackReason(-1));
                break;
        }
        auto nackPduRaw = nackPdu.createFrom(txBuffer);
        mEndpoint.send(nackPduRaw, senderIpPort);
    }
}

void RxJob::receiveThread()
{
    IpPort senderIpPort;
    while (mReceiving.load())
    {
        /** NOTE: Optimal socket is blocking with timeout**/
        size_t receivedSize = mEndpoint.receive(BufferView(mBufferRx, UDP_MAX_SIZE), senderIpPort);
        if (!receivedSize||int64_t(receivedSize)==-1ll)
        {
            continue;
        }

        UrlPduDisassembler receivedPdu(ConstBufferView(mBufferRx, receivedSize));
        auto ipPortMsg = std::make_pair(senderIpPort, receivedPdu.getMessageId());
        if (receivedPdu.hasAckHeader()&&receivedPdu.hasNackHeader())
        {
            mLogger << LOG_ERROR << "NACK: from: " << iptoa(ipFromIpPort(ipPortMsg.first)) << ":" << portFromIpPort(ipPortMsg.first)
                << " msgId: " << ipPortMsg.second
                << " offset: " << receivedPdu.getOffset() << " reason: " << int(receivedPdu.getNackReason());
            mItxManager.reportNack(ipPortMsg, receivedPdu.getOffset(), receivedPdu.getNackReason());
        }
        else if (receivedPdu.hasAckHeader())
        {
            mLogger << LOG_DEBUG << "ACK: " << "from: " << iptoa(ipFromIpPort(ipPortMsg.first)) << ":" << portFromIpPort(ipPortMsg.first)
                << " msgId: " << ipPortMsg.second
                << " offset: " << receivedPdu.getOffset();
            mItxManager.reportAck(ipPortMsg, receivedPdu.getOffset());
        }
        else if (receivedPdu.hasDataHeader())
        {
            mLogger << LOG_DEBUG << "DATA: from: " << iptoa(ipFromIpPort(ipPortMsg.first)) << ":" << portFromIpPort(ipPortMsg.first)
                << " msgId: " << ipPortMsg.second
                << " offset: " << receivedPdu.getOffset();
            auto rxContext = mRxContexts.find(ipPortMsg);
            if (rxContext != mRxContexts.end())
            {
                mLogger << LOG_DEBUG << "PROCESS DATA";
                auto rcvState = rxContext->second.mRxSegmentAssembler.receive(receivedPdu.getPayloadView(),
                    receivedPdu.getOffset());
                processSegmentAssemblerReceived(receivedPdu, rcvState, rxContext, senderIpPort);
                if (rcvState != RxSegmentAssembler::EReceivedSegmentStatus::COMPLETE)
                {   
                    rxContext->second.mLastReceived = 
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                }
            }
            else
            {
                mLogger << LOG_ERROR << "DATA NOT IN CONTEXT";
                /** TODO: received data pdu without context **/
            }
        }
        else if (receivedPdu.hasInitialDataHeader())
        {
            /** TODO: check cases when ipPortMsg already exist**/
            mLogger << LOG_DEBUG << "DATA INT: from: " << iptoa(ipFromIpPort(ipPortMsg.first)) << ":" << portFromIpPort(ipPortMsg.first)
                << " msgId: " << ipPortMsg.second
                << " offset: " << receivedPdu.getOffset();
            auto rxContextInsertRes = mRxContexts.emplace(ipPortMsg, RxContext());
            rxContextInsertRes.first->second.mRxSegmentAssembler.initUrlMessageSize(receivedPdu.getTotalMessageSize());
            rxContextInsertRes.first->second.mAcknowledgeMode = receivedPdu.isAcknowledgmentEnabled();
            rxContextInsertRes.first->second.mLastReceived = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto rcvState = rxContextInsertRes.first->second.mRxSegmentAssembler
                .receive(receivedPdu.getPayloadView(),0);
            processSegmentAssemblerReceived(receivedPdu, rcvState, rxContextInsertRes.first, senderIpPort);
        }

        for (auto i=mRxContexts.begin(); i!=mRxContexts.end();)
        {
            const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            if ((now-i->second.mLastReceived)>(20000000u)) /** TODO: configurable rx expiry**/
            {
                mLogger << LOG_ERROR << "EXPIRED CONTEXT: from: "
                    << iptoa(i->first.first) << " msgId: "
                    << i->first.second;
                mRxContexts.erase(i++); 
            }
            else
            {
                i++;
            }
        }
    }
}

} // namespace urlsock
