#include <cstring>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <src/UrlPduAssembler.hpp>

using namespace testing;
using namespace urlsock;

struct UrlPduAssemblerTests : public ::testing::Test
{
    UrlPduAssemblerTests()
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

    UrlPduAssembler pduAssembler;
};

TEST_F(UrlPduAssemblerTests, shouldCreateAck)
{
    constexpr uint16_t MSG_ID = 1234;
    constexpr uint16_t MAC = 5129;
    constexpr uint32_t OFFSET = 12345;
    pduAssembler.setAckHeader(12345, MSG_ID, MAC);
    auto bout = pduAssembler.create();
    EXPECT_EQ(OFFSET, bout.get<uint32_t>(0));
    EXPECT_EQ(MSG_ID, bout.get<uint32_t>(4)>>16);
    EXPECT_EQ(MAC, bout.get<uint32_t>(4)&0xFFFF);
    EXPECT_EQ(8u, bout.size());
}

TEST_F(UrlPduAssemblerTests, shouldCreateInitialData)
{
    constexpr uint32_t URL_MESSAGE_SIZE = 1000;
    constexpr uint16_t MSG_ID = 1234;
    constexpr uint16_t MAC = 14429;

    const uint8_t payload[] = {1,2,3,4,5,6,7,8,9,10};
    ConstBufferView payloadView = ConstBufferView(payload, payload+10);
    pduAssembler.setInitialDataHeader(URL_MESSAGE_SIZE, MSG_ID, MAC,
        false, true, 8, 9);
    pduAssembler.setPayload(payloadView);
    auto bout = pduAssembler.create();

    EXPECT_EQ((0b01ul<<30) | URL_MESSAGE_SIZE, bout.get<uint32_t>(0));
    EXPECT_EQ(MSG_ID, bout.get<uint32_t>(4)>>16);
    EXPECT_EQ(MAC, bout.get<uint32_t>(4)&0xFFFF);
    EXPECT_EQ(9u, bout.get<uint32_t>(8)&0xF);
    EXPECT_EQ(8u, (bout.get<uint32_t>(8)>>4)&0xF);
    EXPECT_TRUE(bout.get<uint32_t>(8)&(1u<<8));
    EXPECT_FALSE(bout.get<uint32_t>(8)&(1u<<9));
    EXPECT_EQ(22u, bout.size());
    EXPECT_TRUE(!std::memcmp(payload, bout.data()+12, 10));
}

TEST_F(UrlPduAssemblerTests, shouldCreateData)
{
    constexpr uint16_t MSG_ID = 1234;
    constexpr uint16_t MAC = 119;
    constexpr uint32_t OFFSET = 4321;
    const uint8_t payload[] = {1,2,3,4,5,6,7,8,9,10,11};
    ConstBufferView payloadView = ConstBufferView(payload, payload+11);
    pduAssembler.setDataHeader(OFFSET, MSG_ID, MAC);
    pduAssembler.setPayload(payloadView);
    auto bout = pduAssembler.create();

    EXPECT_EQ((0b10ul<<30) | OFFSET, bout.get<uint32_t>(0));
    EXPECT_EQ(MSG_ID, bout.get<uint32_t>(4)>>16);
    EXPECT_EQ(MAC, bout.get<uint32_t>(4)&0xFFFF);
    EXPECT_EQ(19u, bout.size());
    EXPECT_TRUE(!std::memcmp(payload, bout.data()+8, 11));
}

TEST_F(UrlPduAssemblerTests, shouldCreateNack)
{
    constexpr uint16_t MSG_ID = 1234;
    constexpr uint16_t MAC = 1429;
    constexpr uint32_t OFFSET = 45678u;
    pduAssembler.setNackInfoHeader(ENackReason::DUPLICATE_SEGMENT_MISMATCHED);
    pduAssembler.setAckHeader(OFFSET, MSG_ID, MAC);
    auto bout = pduAssembler.create();
    EXPECT_EQ((0b11ul<<30) | uint32_t(ENackReason::DUPLICATE_SEGMENT_MISMATCHED), bout.get<uint32_t>(0));
    EXPECT_EQ(OFFSET, bout.get<uint32_t>(4));
    EXPECT_EQ(MSG_ID, bout.get<uint32_t>(8)>>16);
    EXPECT_EQ(MAC, bout.get<uint32_t>(8)&0xFFFF);
    EXPECT_EQ(12u, bout.size());
}
