#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <src/RxSegmentAssembler.hpp>
#include <src/RxJob.hpp>
#include <src/UrlPduAssembler.hpp>
#include <tests/mocks/IEndPointMock.hpp>
#include <tests/mocks/IRxBufferManagerMock.hpp>
#include <tests/mocks/ITxJobManagerMock.hpp>
#include <tests/mocks/Matchers.hpp>

using namespace testing;
using namespace urlsock;

struct RxJobTests : public Test
{
    RxJobTests():
        rxJob(txJobManagerMock, rxBufferManager, endPoint)
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
    IRxBufferManagerMock rxBufferManager;
    ITxJobManagerMock txJobManagerMock;
    RxJob rxJob;
};

TEST_F(RxJobTests, shouldSendOneSegmentUrlMessage)
{
    constexpr uint16_t MSG_ID = 42u;
    constexpr uint16_t MSG_SIZE = 621u;
    auto toSend = createUrlMessage(MSG_SIZE, 1);

    IpPort ipPort = IpPorter(127,0,0,1,6969);

    auto ack0 = createAck(0, MSG_ID, 0);    
    MessageMatcher ack0matcher(ack0, ipPort);

    EXPECT_CALL(rxBufferManager, enqueue_rvalHack(ipPort, IsBufferEq(toSend[0])));

    endPoint.expectSend(1, 0, false, 1, ack0matcher.get(), DefaultAction::get());

    endPoint.toReceive(createInitialDataPdu(toSend[0], MSG_SIZE, MSG_ID), ipPort);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    endPoint.waitForAllSending(100.0);
}
TEST_F(RxJobTests, shouldSendManySegmentUrlMessage)
{
    constexpr uint16_t MSG_ID = 42u;
    constexpr uint16_t MSG_SIZE = 605u;
    auto toSend = createUrlMessage(MSG_SIZE, 6);

    IpPort ipPort = IpPorter(127,0,0,1,6969);

    auto ack0 = createAck(0, MSG_ID, 0);
    auto ack100 = createAck(100, MSG_ID, 0);
    auto ack200 = createAck(200, MSG_ID, 0);
    auto ack300 = createAck(300, MSG_ID, 0);
    auto ack400 = createAck(400, MSG_ID, 0);
    auto ack500 = createAck(500, MSG_ID, 0);
    auto ack600 = createAck(600, MSG_ID, 0);

    MessageMatcher ack0matcher(ack0, ipPort);
    MessageMatcher ack100matcher(ack100, ipPort);
    MessageMatcher ack200matcher(ack200, ipPort);
    MessageMatcher ack300matcher(ack300, ipPort);
    MessageMatcher ack400matcher(ack400, ipPort);
    MessageMatcher ack500matcher(ack500, ipPort);
    MessageMatcher ack600matcher(ack600, ipPort);

    auto sent = combineBuffer(toSend);
    EXPECT_CALL(rxBufferManager, enqueue_rvalHack(ipPort, IsBufferEq(sent)));

    endPoint.expectSend(1, 0, false, 1, ack0matcher.get(), DefaultAction::get());
    endPoint.expectSend(2, 1, false, 1, ack100matcher.get(), DefaultAction::get());
    endPoint.expectSend(3, 2, false, 1, ack200matcher.get(), DefaultAction::get());
    endPoint.expectSend(4, 3, false, 1, ack300matcher.get(), DefaultAction::get());
    endPoint.expectSend(5, 4, false, 1, ack400matcher.get(), DefaultAction::get());
    endPoint.expectSend(6, 5, false, 1, ack500matcher.get(), DefaultAction::get());
    endPoint.expectSend(7, 6, false, 1, ack600matcher.get(), DefaultAction::get());

    endPoint.toReceive(createInitialDataPdu(toSend[0], MSG_SIZE, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[1], 100, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[2], 200, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[3], 300, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[4], 400, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[5], 500, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[6], 600, MSG_ID), ipPort);

    endPoint.waitForAllSending(100.0);
}

TEST_F(RxJobTests, shouldSendOutOfOrdeSresgmentUrlMessage)
{
    constexpr uint16_t MSG_ID = 42u;
    constexpr uint16_t MSG_SIZE = 601u;
    auto toSend = createUrlMessage(MSG_SIZE, 6);

    IpPort ipPort = IpPorter(127,0,0,1,6969);

    auto ack0 = createAck(0, MSG_ID, 0);
    auto ack100 = createAck(100, MSG_ID, 0);
    auto ack400 = createAck(400, MSG_ID, 0);
    auto ack300 = createAck(300, MSG_ID, 0);
    auto ack600 = createAck(600, MSG_ID, 0);
    auto ack500 = createAck(500, MSG_ID, 0);
    auto ack200 = createAck(200, MSG_ID, 0);

    MessageMatcher ack0matcher(ack0, ipPort);
    MessageMatcher ack100matcher(ack100, ipPort);
    MessageMatcher ack400matcher(ack400, ipPort);
    MessageMatcher ack300matcher(ack300, ipPort);
    MessageMatcher ack600matcher(ack600, ipPort);
    MessageMatcher ack500matcher(ack500, ipPort);
    MessageMatcher ack200matcher(ack200, ipPort);

    auto sent = combineBuffer(toSend);
    EXPECT_CALL(rxBufferManager, enqueue_rvalHack(ipPort, IsBufferEq(sent)));

    endPoint.expectSend(1, 0, false, 1, ack0matcher.get(), DefaultAction::get());
    endPoint.expectSend(2, 1, false, 1, ack100matcher.get(), DefaultAction::get());
    endPoint.expectSend(3, 2, false, 1, ack400matcher.get(), DefaultAction::get());
    endPoint.expectSend(4, 3, false, 1, ack300matcher.get(), DefaultAction::get());
    endPoint.expectSend(5, 4, false, 1, ack600matcher.get(), DefaultAction::get());
    endPoint.expectSend(6, 5, false, 1, ack500matcher.get(), DefaultAction::get());
    endPoint.expectSend(7, 6, false, 1, ack200matcher.get(), DefaultAction::get());

    endPoint.toReceive(createInitialDataPdu(toSend[0], MSG_SIZE, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[1], 100, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[4], 400, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[3], 300, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[6], 600, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[5], 500, MSG_ID), ipPort);
    endPoint.toReceive(createDataPdu(toSend[2], 200, MSG_ID), ipPort);

    endPoint.waitForAllSending(100.0);
}

// Test reception of Url message with one segment
// Test reception of Url message with many segments
// Test reception of Url message with out of order segments
