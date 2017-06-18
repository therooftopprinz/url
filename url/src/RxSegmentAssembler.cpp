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
        free(mUrlMessage);
    }
}

void RxSegmentAssembler::initUrlMessageSize(size_t size)
{
    if (mUrlMessage)
    {
        free(mUrlMessage);
        mReceivedBlocks.clear();
    }
    mUrlMessageReceivedSize = 0;
    mUrlMessage = (uint8_t*)malloc(size);
    mUrlMessageTotalSize = size;
}

bool RxSegmentAssembler::isInited()
{
    return mUrlMessage;
}

Buffer RxSegmentAssembler::claim()
{
    Buffer rv;
    rv.own(mUrlMessage, mUrlMessageTotalSize);
    mUrlMessage = nullptr;
    return rv;
}

const uint8_t* RxSegmentAssembler::peek() const
{
    return mUrlMessage;
}

size_t RxSegmentAssembler::size() const
{
    return mUrlMessageTotalSize;
}

size_t RxSegmentAssembler::receivedSize() const
{
    return mUrlMessageTotalSize;
}

RxSegmentAssembler::EReceivedSegmentStatus RxSegmentAssembler::receive(const ConstBufferView& data, uint32_t offset)
{
    return receive(data.data(), data.size(), offset);
}

RxSegmentAssembler::EReceivedSegmentStatus RxSegmentAssembler::receive(const uint8_t *start, size_t size, uint32_t offset)
{
    if ((offset+size) > mUrlMessageTotalSize)
    {
        return EReceivedSegmentStatus::DATA_OUTOFBOUNDS;
    }

    auto found = mReceivedBlocks.find(offset);
    if (found != mReceivedBlocks.end())
    {
        if (size != found->second)
        {
            return EReceivedSegmentStatus::INCORRECT_RTX_SIZE;
        }
        if (std::memcmp(mUrlMessage+found->first, start, size))
        {
            return EReceivedSegmentStatus::INCORRECT_RTX_DATA;
        }
    }
    else
    {
        auto bufferTail = offset+size;
        auto lb = mReceivedBlocks.lower_bound(offset);
        if (lb != mReceivedBlocks.end() && bufferTail > lb->first)
        {
            return EReceivedSegmentStatus::DATA_OVERLAPPED;
        }
        if (lb != mReceivedBlocks.begin() && ((--lb)->first+lb->second) > offset)
        {
            return EReceivedSegmentStatus::DATA_OVERLAPPED;
        }

        mReceivedBlocks.emplace(offset, size);
        std::memcpy(mUrlMessage + offset, start, size);
        mUrlMessageReceivedSize += size;
    }

    if (mUrlMessageReceivedSize == mUrlMessageTotalSize)
    {
        return EReceivedSegmentStatus::COMPLETE;
    }

    return EReceivedSegmentStatus::INCOMPLETE;
}

} // namespace urlsock
