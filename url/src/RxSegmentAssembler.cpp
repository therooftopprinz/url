#include <cstring>
#include "Buffer.hpp"
#include "RxSegmentAssembler.hpp"

namespace urlsock
{

RxSegmentAssembler::RxSegmentAssembler():
    mUrlMessage(nullptr),
    mUrlMessageTotalSize(0),
    mUrlMessageReceivedSize(0)
{
}

RxSegmentAssembler::~RxSegmentAssembler()
{
    if (mUrlMessage)
    {
        delete[] mUrlMessage;
    }
}

void RxSegmentAssembler::init(size_t size)
{
    if (mUrlMessage)
    {
        delete[] mUrlMessage;
        mReceivedBlocks.clear();
    }
    mUrlMessageReceivedSize = 0;
    mUrlMessage = new uint8_t[size];
    mUrlMessageTotalSize = size;
}

bool RxSegmentAssembler::isInited()
{
    return mUrlMessage;
}

uint8_t* RxSegmentAssembler::claim()
{
    uint8_t *rv = mUrlMessage;
    mUrlMessage = nullptr;
    return rv;
}

RxSegmentAssembler::EReceiveStatus RxSegmentAssembler::receive(const Buffer& data, uint32_t offset)
{
    return receive(data.data(), data.size(), offset);
}

RxSegmentAssembler::EReceiveStatus RxSegmentAssembler::receive(const BufferView& data, uint32_t offset)
{
    return receive(data.data(), data.size(), offset);
}

RxSegmentAssembler::EReceiveStatus RxSegmentAssembler::receive(const uint8_t *start, size_t size, uint32_t offset)
{
    if ((offset+size) > mUrlMessageTotalSize)
    {
        return EReceiveStatus::DATA_OUTOFBOUNDS;
    }

    auto found = mReceivedBlocks.find(offset);
    if (found != mReceivedBlocks.end())
    {
        if (size != found->second)
        {
            return EReceiveStatus::INCORRECT_RTX_SIZE;
        }
        if (!std::memcmp(mUrlMessage+found->first, start, size))
        {
            return EReceiveStatus::INCORRECT_RTX_DATA;
        }
    }
    else
    {
        mReceivedBlocks.emplace(offset, size);
        std::memcpy(mUrlMessage + offset, start, size);
    }

    mUrlMessageTotalSize += size;

    if (mUrlMessageReceivedSize == mUrlMessageTotalSize)
    {
        return EReceiveStatus::COMPLETE;
    }

    return EReceiveStatus::INCOMPLETE;
}

} // namespace urlsock
