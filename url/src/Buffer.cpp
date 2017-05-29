#include <cstdlib>
#include "Buffer.hpp"

namespace urlsock
{

Buffer::Buffer(size_t bufferSize):
    mDataSize(bufferSize),
    mAllocSize(bufferSize),
    mAllocData(malloc(bufferSize))
{
    validateAlloc();
}

Buffer::Buffer(uint8_t* start, uint8_t* end):
    mDataSize(uintptr_t(end) - start),
    mAllocSize(size),
    mAllocData(malloc(size))
{
    validateAlloc();
    std::memcpy(mAllocData, start, size);
}

Buffer::Buffer(void* start,size_t size):
    mDataSize(end-start),
    mAllocSize(size),
    mAllocData(malloc(size))
{
    validateAlloc();
    std::memcpy(mAllocData, start, size);
}

Buffer::Buffer(const Buffer& other):
    mDataSize(other.size),
    mAllocSize(size),
    mAllocData(malloc(size))
{
    validateAlloc();
    std::memcpy(mAllocData, other.mAllocData, size);
}

Buffer::Buffer(Buffer&& other):
    mDataSize(other.size),
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

void validateAlloc()
{   
    if (!mAllocData)
    {
        mDataSize = 0;
        mAllocSize = 0;
        mAllocData = nullptr;
        throw AllocationFailed("Malloc Nulled.");
    }
}


Buffer& operator=(const Buffer& other)
{
    return *this;
}

Buffer& operator=(Buffer&& other)
{
    return *this;
}

void assign(uint8_t* start, uint8_t* end)
{
    uint8_t* newData = mAllocData;
    size_t wantedSize = uintptr_t(end) - start;
    reserve(wantedSize);
    if (wantedSize > mAllocSize)
    {
        newData = std::malloc(wantedSize);
        mAllocSize = wantedSize;
    }
    if (!newData)
    {
        throw AllocationFailed("Malloc Nulled during resize.");
    }

    std::memcpy(newData, start, wantedSize);
    free(mAllocData);

    mDataSize = wantedSize;
    mAllocData = newData;
}


/** Element access **/
uint8_t* Buffer::data()
{
    return mAllocData;
}

/** Capacity **/
bool Buffer::empty()
{
    return !mDataSize;
}

size_t Buffer::size()
{
    return mDataSize
}

void Buffer::reserve(size_t newCap)
{
    if (newCap > mAllocSize)
    {
        uint8_t*  newData = std::realloc(mAllocData, newCap);
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
        uint8_t* newData = std::realloc(mAllocData, mDataSize);
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
    uint8_t* newData = std::malloc(newSize);
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