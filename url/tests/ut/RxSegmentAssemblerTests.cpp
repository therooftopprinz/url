#include <cstring>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <src/RxSegmentAssembler.hpp>

using namespace testing;
using namespace urlsock;

struct RxSegmentAssemblerTests : public ::testing::Test
{
    RxSegmentAssemblerTests()
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

    RxSegmentAssembler rxSegmentAssembler;
};

TEST_F(RxSegmentAssemblerTests, shouldCompleteWhenComplete)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(1000);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::COMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
}

TEST_F(RxSegmentAssemblerTests, shouldIncompleteThenCompleteWhenComplete)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(200);
    Buffer rxSegment2(400);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 200));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::COMPLETE, rxSegmentAssembler.receive(rxSegment2, 600));

    std::vector<uint8_t> expected;
    expected.insert(expected.end(), rxSegment.data(), rxSegment.data()+rxSegment.size());
    expected.insert(expected.end(), rxSegment.data(), rxSegment.data()+rxSegment.size());
    expected.insert(expected.end(), rxSegment.data(), rxSegment.data()+rxSegment.size());
    expected.insert(expected.end(), rxSegment2.data(), rxSegment2.data()+rxSegment2.size());

    ASSERT_EQ(rxSegmentAssembler.size(), expected.size());
    EXPECT_TRUE(!std::memcmp(expected.data(), rxSegmentAssembler.peek(), expected.size()));
}

TEST_F(RxSegmentAssemblerTests, shouldNotErrorWhenReceivedAgainWithSameSizeAndData)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(200);
    Buffer rxSegment2(400);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment2, 600));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::COMPLETE, rxSegmentAssembler.receive(rxSegment, 200));

    std::vector<uint8_t> expected;
    expected.insert(expected.end(), rxSegment.data(), rxSegment.data()+rxSegment.size());
    expected.insert(expected.end(), rxSegment.data(), rxSegment.data()+rxSegment.size());
    expected.insert(expected.end(), rxSegment.data(), rxSegment.data()+rxSegment.size());
    expected.insert(expected.end(), rxSegment2.data(), rxSegment2.data()+rxSegment2.size());

    ASSERT_EQ(rxSegmentAssembler.size(), expected.size());
    EXPECT_TRUE(!std::memcmp(expected.data(), rxSegmentAssembler.peek(), expected.size()));
}

TEST_F(RxSegmentAssemblerTests, shouldErrorWhenReceivedAgainWithDifferentSize)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(200);
    Buffer rxSegment2(400);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment2, 600));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCORRECT_RTX_SIZE, rxSegmentAssembler.receive(rxSegment2, 400));
}

TEST_F(RxSegmentAssemblerTests, shouldErrorWhenReceivedAgainWithDifferentData)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(200);
    Buffer rxSegment2(400);
    Buffer rxSegment3(200);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment2, 600));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCORRECT_RTX_DATA, rxSegmentAssembler.receive(rxSegment3, 400));
}

TEST_F(RxSegmentAssemblerTests, shouldErrorWhenReceivedAgainWithOverlappedDataFromPrevious)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(100);
    Buffer rxSegment2(20);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 200));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment2, 100));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::DATA_OVERLAPPED, rxSegmentAssembler.receive(rxSegment2, 119));
}

TEST_F(RxSegmentAssemblerTests, shouldErrorWhenReceivedAgainWithOverlappedDataFromNext)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(100);
    Buffer rxSegment2(20);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 200));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment2, 180));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::DATA_OVERLAPPED, rxSegmentAssembler.receive(rxSegment2, 161));
}

TEST_F(RxSegmentAssemblerTests, shouldErrorWhenReceivedAgainWithOverlappedDataFromWholeNext)
{
    rxSegmentAssembler.init(1000);
    Buffer rxSegment(100);
    Buffer rxSegment2(200);
    fillBufferWithRandomShit(rxSegment);
    fillBufferWithRandomShit(rxSegment2);
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 0));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 200));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::INCOMPLETE, rxSegmentAssembler.receive(rxSegment, 400));
    EXPECT_EQ(RxSegmentAssembler::EReceiveStatus::DATA_OVERLAPPED, rxSegmentAssembler.receive(rxSegment2, 150));
}
