#include <cstring>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <src/TxJob.hpp>
#include <src/UrlPduAssembler.hpp>
#include <src/UrlPduDisassembler.hpp>
#include <src/RxSegmentAssembler.hpp>
#include <tests/mocks/IEndPointMock.hpp>
#include <tests/mocks/IRxBufferManagerMock.hpp>
#include <tests/mocks/ITxJobManagerMock.hpp>
#include <tests/mocks/Matchers.hpp>

using namespace testing;
using namespace urlsock;

struct TxJobTests : public Test
{
    TxJobTests()    
    {
        std::srand(std::time(0));
    }

    void fillBufferWithRandomShit(Buffer& buffer)
    {
        std::transform(buffer.data(), buffer.data()+buffer.size(), buffer.data(), [](const uint8_t)
        {
            return std::rand()&0xFF;
        });
    }

    std::vector<Buffer> createUrlMessage(uint32_t size, uint32_t numberOfSegments)
    {
        std::vector<Buffer> rv;
        auto segmentSize = size/numberOfSegments;

        for (auto i=0u; i<numberOfSegments; i++)
        {
            Buffer segmentPayload(segmentSize);
            fillBufferWithRandomShit(segmentPayload);
            rv.push_back(segmentPayload);
        }
        if (auto segmentSize = size%numberOfSegments)
        {
            Buffer segmentPayload(segmentSize);
            fillBufferWithRandomShit(segmentPayload);
            rv.push_back(segmentPayload);
        }
        return rv;
    }

    Buffer combineBuffer(std::vector<Buffer> blist)
    {
        size_t size = 0;
        for (const auto& i : blist)
        {
            size += i.size();
        }
        Buffer rv(size);
        uint8_t *offset = rv.data();
        for (const auto& i : blist)
        {
            std::memcpy(offset, i.data(), i.size());
            offset+=i.size();
        }
        return rv;
    }

    Buffer createAck(uint32_t offset, uint16_t messageId, uint16_t mac)
    {
        UrlPduAssembler ackPdu;
        ackPdu.setAckHeader(offset, messageId, mac);
        return ackPdu.create();
    }

    Buffer createInitialDataPdu(Buffer& segmentData, uint32_t size, uint16_t msgId)
    {
        UrlPduAssembler segment0;
        segment0.setInitialDataHeader(size, msgId, 0, false, true, 0, 0);
        segment0.setPayload(segmentData);
        return segment0.create();
    }

    Buffer createDataPdu(Buffer& segmentData, uint32_t offset, uint16_t msgId)
    {
        UrlPduAssembler segmentN;
        segmentN.setDataHeader(offset, msgId, 0);
        segmentN.setPayload(segmentData);
        return segmentN.create();
    }

    IEndPointMock endPoint;
};

class RxChecker
{
public:
    RxChecker(ITxJob& txJob, IpPort ipPort, MessageId messageId):
        mTxJob(txJob),
        mIpPort(ipPort),
        mMessageId(messageId),
        mCompleted(false)
    {}

    ActionFunctor get()
    {
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        return std::bind(&RxChecker::onReceive, this, _1, _2, _3);
    }
private:
    void ackNotifierThread(uint32_t offset)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(500)); // Simulate latency
        std::cout << "sending ack for: " << offset << std::endl;
        mTxJob.eventAckReceived(offset);
    }

    void onReceive(const void *buffer, size_t size, IpPort ipPort)
    {
        UrlPduDisassembler receivedPdu(ConstBufferView((const uint8_t*)buffer, size));

        if ((receivedPdu.hasDataHeader() ||receivedPdu.hasInitialDataHeader()) && ipPort == mIpPort &&
            receivedPdu.getMessageId() == mMessageId)
        {
            auto rcvState = mRxSegmentAssembler.receive(receivedPdu.getPayloadView(),
                receivedPdu.getOffset());
            std::thread(&RxChecker::ackNotifierThread, this, receivedPdu.getOffset()).detach();
            if (rcvState==RxSegmentAssembler::EReceivedSegmentStatus::COMPLETE)
            {
                mCompleted = true;
            } 
        }
    }

    ITxJob& mTxJob;
    IpPort mIpPort;
    MessageId mMessageId;
    bool mCompleted;
    RxSegmentAssembler mRxSegmentAssembler;
};

TEST_F(TxJobTests, shouldSendOneSegmentUrlMessage)
{
    constexpr uint16_t MSG_ID = 42u;
    constexpr uint16_t MSG_SIZE = 621u;
    auto toSend = createUrlMessage(MSG_SIZE, 1);

    IpPort ipPort = IpPorter(127,0,0,1,6969);

    Buffer messageData(MSG_SIZE);
    std::cout << "data: " << (void*)messageData.data() << std::endl; 
    fillBufferWithRandomShit(messageData);

    ConstBufferView messageDataCb(messageData);
    TxJob txJob(messageDataCb, endPoint, std::make_pair(ipPort, MSG_ID), true,
        0, 0, 1500);
    RxChecker rxChecker(txJob, ipPort, MSG_ID);

    AnyMessageMatcher allSendOfIpPort(ipPort);
    endPoint.expectSend(1, 0, false, -1, allSendOfIpPort.get(), rxChecker.get());
    txJob.run();

    endPoint.waitForAllSending(100.0);
}

/** TODO: Nack cases **/