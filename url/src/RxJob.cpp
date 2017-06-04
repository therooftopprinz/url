#include "RxJob.hpp"
#include "ITxJobManager.hpp"
#include "IRxBufferManager.hpp"
#include "IEndPoint.hpp"
#include "RxSegmentAssembler.hpp"
#include "UrlPduDisassembler.hpp"
#include "UrlPduAssembler.hpp"

namespace urlsock
{
constexpr size_t udpMaxSize = 65536u;
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

void RxJob::receiveThread()
{
    uint8_t buffer[udpMaxSize];
    IpPort senderIpPort;
    while (mReceiving.load())
    {
        size_t receivedSize = mEndpoint.receive(BufferView(buffer, udpMaxSize), senderIpPort);
        UrlPduDisassembler receivedPdu(ConstBufferView(buffer, receivedSize));
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
                rxContext->second.mRxContext.receive(receivedPdu.getPayloadView(), receivedPdu.getOffset());
                /** TODO: send ack/nack for the pdu**/
            }
            else
            {
                /** TODO: received data pdu without context **/
            }
        }
        else if (receivedPdu.hasInitialDataHeader())
        {
            auto& context = mRxContexts[ipPortMsg];
            context.mRxSegmentAssembler.initUrlMessageSize(receivedPdu.getTotalMessageSize());
            context.mRxSegmentAssembler.receive(receivedPdu.getPayloadView(), 0);
            /** TODO: send ack/nack for the pdu**/
        }
    }
}

} // namespace urlsock
