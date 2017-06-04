#include <cstring>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <src/UrlPduDisassembler.hpp>
#include <src/UrlPduAssembler.hpp>

using namespace testing;
using namespace urlsock;

struct UrlPduDisassemblerTests : public ::testing::Test
{
    UrlPduDisassemblerTests()
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

TEST_F(UrlPduDisassemblerTests, shouldCreateAckPdu)
{
    constexpr uint32_t OFFSET = 0x210abcde;
    constexpr uint16_t MSG_ID = 0xf111;
    constexpr uint16_t MAC = 0x7654;
    pduAssembler.setAckHeader(OFFSET, MSG_ID, MAC);
    auto bout = pduAssembler.create();

    UrlPduDisassembler pduDisassembler(bout);
    ASSERT_TRUE(pduDisassembler.hasAckHeader());
    ASSERT_FALSE(pduDisassembler.hasDataHeader());
    ASSERT_FALSE(pduDisassembler.hasInitialDataHeader());
    ASSERT_FALSE(pduDisassembler.hasNackHeader());
    EXPECT_EQ(OFFSET, pduDisassembler.getOffset());
    EXPECT_EQ(MSG_ID, pduDisassembler.getMessageId());
    EXPECT_EQ(MAC, pduDisassembler.getMac());
}

TEST_F(UrlPduDisassemblerTests, shouldDataPdu)
{
    constexpr uint32_t OFFSET = 0x210abcde;
    constexpr uint16_t MSG_ID = 0xf111;
    constexpr uint16_t MAC = 0x7654;
    pduAssembler.setDataHeader(OFFSET, MSG_ID, MAC);

    Buffer expected(400);
    fillBufferWithRandomShit(expected);
    pduAssembler.setPayload(expected);

    auto bout = pduAssembler.create();

    UrlPduDisassembler pduDisassembler(bout);
    ASSERT_FALSE(pduDisassembler.hasAckHeader());
    ASSERT_TRUE(pduDisassembler.hasDataHeader());
    ASSERT_FALSE(pduDisassembler.hasInitialDataHeader());
    ASSERT_FALSE(pduDisassembler.hasNackHeader());
    EXPECT_EQ(OFFSET, pduDisassembler.getOffset());
    EXPECT_EQ(MSG_ID, pduDisassembler.getMessageId());
    EXPECT_EQ(MAC, pduDisassembler.getMac());
    ASSERT_EQ(expected.size(), pduDisassembler.getPayload().size());
    EXPECT_TRUE(!std::memcmp(expected.data(), pduDisassembler.getPayload().data(), expected.size()));
}

TEST_F(UrlPduDisassemblerTests, shouldInitialDataPdu)
{
    constexpr uint32_t MSG_SIZE = 0x210abcde;
    constexpr uint16_t MSG_ID = 0xf111;
    constexpr uint16_t MAC = 0x7654;
    constexpr uint8_t PROT_ALG = 0xf;
    constexpr uint8_t CIP_ALG = 0xf;
    pduAssembler.setInitialDataHeader(MSG_SIZE, MSG_ID, MAC, true, true, PROT_ALG, CIP_ALG);
    Buffer expected(400);
    fillBufferWithRandomShit(expected);
    pduAssembler.setPayload(expected);

    auto bout = pduAssembler.create();

    UrlPduDisassembler pduDisassembler(bout);
    ASSERT_FALSE(pduDisassembler.hasAckHeader());
    ASSERT_FALSE(pduDisassembler.hasDataHeader());
    ASSERT_TRUE(pduDisassembler.hasInitialDataHeader());
    ASSERT_FALSE(pduDisassembler.hasNackHeader());
    EXPECT_EQ(MSG_SIZE, pduDisassembler.getTotalMessageSize());
    EXPECT_EQ(MSG_ID, pduDisassembler.getMessageId());
    EXPECT_EQ(MAC, pduDisassembler.getMac());
    EXPECT_TRUE(pduDisassembler.isAcknowledgmentEnabled());
    EXPECT_TRUE(pduDisassembler.isRetransmit());
    ASSERT_EQ(expected.size(), pduDisassembler.getPayload().size());
    EXPECT_TRUE(!std::memcmp(expected.data(), pduDisassembler.getPayload().data(), expected.size()));
}

