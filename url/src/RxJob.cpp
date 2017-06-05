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
    mReceiving(true)
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
        auto rxUrlMessageData = rxContext->second.mRxSegmentAssembler.claim();
        mRxBufferManager.enqueue(rxContext->first.first, std::move(rxUrlMessageData));
        mRxContexts.erase(rxContext);
    }
    else if (rcvState == RxSegmentAssembler::EReceivedSegmentStatus::INCOMPLETE)
    {
        UrlPduAssembler ackPdu;
        BufferView txBuffer(mBufferTx, udpMaxSize);
        ackPdu.setAckHeader(receivedPdu.getOffset(), receivedPdu.getMessageId(), receivedPdu.getMac());
        auto ackPduRaw = ackPdu.createFrom(txBuffer);
        mEndpoint.send(ackPduRaw, senderIpPort);
    }
    else
    {
        UrlPduAssembler nackPdu;
        BufferView txBuffer(mBufferTx, udpMaxSize);
        nackPdu.setAckHeader(receivedPdu.getOffset(), receivedPdu.getMessageId(), receivedPdu.getMac());
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
        size_t receivedSize = mEndpoint.receive(BufferView(mBufferRx, udpMaxSize), senderIpPort);
        if (!receivedSize)
        {
            continue;
        }

        UrlPduDisassembler receivedPdu(ConstBufferView(mBufferRx, receivedSize));
        auto ipPortMsg = std::make_pair(senderIpPort, receivedPdu.getMessageId());
        if (receivedPdu.hasAckHeader()&&receivedPdu.hasNackHeader())
        {
            mItxManager.reportNack(ipPortMsg, receivedPdu.getOffset(), receivedPdu.getNackReason());
        }
        else if (receivedPdu.hasAckHeader())
        {
            mItxManager.reportAck(ipPortMsg, receivedPdu.getOffset());
        }
        else if (receivedPdu.hasDataHeader())
        {
            auto rxContext = mRxContexts.find(ipPortMsg);
            if (rxContext != mRxContexts.end())
            {
                auto rcvState = rxContext->second.mRxSegmentAssembler.receive(receivedPdu.getPayloadView(),
                    receivedPdu.getOffset());
                processSegmentAssemblerReceived(receivedPdu, rcvState, rxContext, senderIpPort);
                rxContext->second.mLastReceived = 
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            }
            else
            {
                /** TODO: received data pdu without context **/
            }
        }
        else if (receivedPdu.hasInitialDataHeader())
        {
            /** TODO: check cases when ipPortMsg already exist**/
            auto rxContextInsertRes = mRxContexts.emplace(ipPortMsg, RxContext());
            rxContextInsertRes.first->second.mRxSegmentAssembler.initUrlMessageSize(receivedPdu.getTotalMessageSize());
            auto rcvState = rxContextInsertRes.first->second.mRxSegmentAssembler
                .receive(receivedPdu.getPayloadView(),0);
            processSegmentAssemblerReceived(receivedPdu, rcvState, rxContextInsertRes.first, senderIpPort);
        }

        for (auto i=mRxContexts.begin(); i!=mRxContexts.end(); i++)
        {
            const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            if ((now-i->second.mLastReceived)>(20000000u)) /** TODO: configurable rx expiry**/
            {
                // mRxContexts.erase(i);   
            }
        }
    }
}

} // namespace urlsock
