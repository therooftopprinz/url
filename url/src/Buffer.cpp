#include <cstdlib>
#include <cstring>
#include "Buffer.hpp"

namespace urlsock
{
Buffer::Buffer():
    mDataSize(0),
    mAllocSize(0),
    mAllocData(nullptr)
{
}

Buffer::Buffer(size_t bufferSize):
    mDataSize(bufferSize),
    mAllocSize(bufferSize),
    mAllocData((uint8_t*)std::malloc(bufferSize))
{
    validateAlloc();
}

Buffer::Buffer(const void* start, const void* end):
    mDataSize(uintptr_t(end) - uintptr_t(start)),
    mAllocSize(mDataSize),
    mAllocData((uint8_t*)std::malloc(mDataSize))
{
    validateAlloc();
    std::memcpy(mAllocData, start, mDataSize);
}

Buffer::Buffer(const void* start, size_t size):
    mDataSize(size),
    mAllocSize(size),
    mAllocData((uint8_t*)std::malloc(size))
{
    validateAlloc();
    std::memcpy(mAllocData, start, size);
}

Buffer::Buffer(const Buffer& other):
    mDataSize(other.mDataSize),
    mAllocSize(mDataSize),
    mAllocData((uint8_t*)std::malloc(mDataSize))
{
    validateAlloc();
    std::memcpy(mAllocData, other.mAllocData, mDataSize);
}

Buffer::Buffer(Buffer&& other):
    mDataSize(other.mDataSize),
    mAllocSize(other.mAllocSize),
    mAllocData(other.mAllocData)
{
    other.mDataSize = 0;
    other.mAllocSize = 0;
    other.mAllocData = nullptr;
}

Buffer::~Buffer()
{
    if (mAllocData)
    {
        free(mAllocData);
        mDataSize = 0;
        mAllocSize = 0;
    }
}

void Buffer::validateAlloc()
{   
    if (!mAllocData)
    {
        mDataSize = 0;
        mAllocSize = 0;
        mAllocData = nullptr;
        throw AllocationFailed("Malloc Nulled.");
    }
}

Buffer& Buffer::operator=(const Buffer& other)
{
    assign(other.mAllocData, other.mAllocData+other.mDataSize);
    return *this;
}

Buffer& Buffer::operator=(Buffer&& other)
{
    if (mAllocData)
    {
        free(mAllocData);
        mDataSize = 0;
        mAllocSize = 0;
    }
    mAllocData = other.mAllocData;
    mDataSize = other.mDataSize;
    mAllocSize = other.mAllocSize;
    return *this;
}

void Buffer::own(void* start, size_t size)
{
    if (mAllocData)
    {
        free(mAllocData);
        mDataSize = 0;
        mAllocSize = 0;
    }

    mAllocData = (uint8_t*)start;
    mDataSize = size;
    mAllocSize = size;
}

void Buffer::assign(uint8_t* start, uint8_t* end)
{
    uint8_t* newData = mAllocData;
    size_t wantedSize = uintptr_t(end) - uintptr_t(start);
    reserve(wantedSize);
    bool alloced = false;
    if (wantedSize > mAllocSize)
    {
        newData = (uint8_t*)std::malloc(wantedSize);
        mAllocSize = wantedSize;
        alloced = true;
    }
    if (!newData)
    {
        throw AllocationFailed("Malloc Nulled during resize.");
    }

    std::memcpy(newData, start, wantedSize);

    if (alloced)
    {
        free(mAllocData);
        mAllocData = newData;
    }

    mDataSize = wantedSize;
}

/** Element access **/
uint8_t* Buffer::data()
{
    return mAllocData;
}

const uint8_t* Buffer::data() const
{
    return mAllocData;
}

/** Capacity **/
bool Buffer::empty()
{
    return !mDataSize;
}

size_t Buffer::size() const
{
    return mDataSize;
}

void Buffer::reserve(size_t newCap)
{
    if (newCap > mAllocSize)
    {
        uint8_t*  newData = (uint8_t*)std::realloc(mAllocData, newCap);
        if (!newData)
        {
            return;
        }
        mAllocData = newData;
        mAllocSize = newCap;
    }
}

size_t Buffer::capacity()
{
    return mAllocSize;
}

void Buffer::shrink_to_fit()
{
    if (mAllocSize > mDataSize)
    {
        uint8_t* newData = (uint8_t*)std::realloc(mAllocData, mDataSize);
        if (!newData)
        {
            return;
        }
        mAllocSize = mDataSize;
        mAllocData = newData;
    }
}

/** Modifiers **/
void Buffer::clear()
{
    std::memset(mAllocData, 0, mDataSize);
}

void Buffer::resize(size_t newSize)
{
    reserve(newSize);
    if (newSize < mDataSize || newSize <= mAllocSize)
    {
        mDataSize = newSize;
        return;
    }
    uint8_t* newData = (uint8_t*)std::malloc(newSize);
    if (!newData)
    {
        throw AllocationFailed("Malloc Nulled during resize.");
    }

    std::memcpy(newData, mAllocData, mDataSize);
    free(mAllocData);

    mDataSize = newSize;
    mAllocSize = newSize;
    mAllocData = newData;
}

} // namespace urlsock