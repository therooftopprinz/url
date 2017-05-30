#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <cstdint>
#include <exception>
#include <string>

namespace urlsock
{

class AllocationFailed : std::exception
{
public:
    AllocationFailed(std::string message):
        whatString(message)
    {}

    inline const char* what()
    {
        return whatString.c_str();
    }

private:
    std::string whatString;   
};

class BufferView
{
public:
    BufferView() = delete;
    BufferView(uint8_t* start, uint8_t* end):
        mData(start),
        mSize(uintptr_t(end)-uintptr_t(start))
    {}

    BufferView(uint8_t* start, size_t size):
        mData(start),
        mSize(size)
    {}

    inline uint8_t* data()
    {
        return mData;
    }

    inline const uint8_t* data() const
    {
        return mData;
    }

    inline size_t size() const
    {
        return mSize;
    }
private:
    uint8_t *mData;
    size_t mSize;
};

class Buffer
{
public:
    // Buffer(BufferView);
    Buffer(size_t bufferSize);
    Buffer(uint8_t* start, uint8_t* end);
    Buffer(void* start, size_t size);
    Buffer(const Buffer& other);
    Buffer(Buffer&& other);
    ~Buffer();

    Buffer& operator=(const Buffer& other);
    Buffer& operator=(Buffer&& other);
    void assign(uint8_t* start, uint8_t* end);

    /** Element access **/
    uint8_t* data();
    const uint8_t* data() const;

    /** Capacity **/
    bool empty();
    size_t size() const;
    void reserve(size_t newCap);
    size_t capacity();
    void shrink_to_fit();

    /** Modifiers **/
    void clear();
    void resize(size_t newSize);

private:
    void validateAlloc();
    size_t mDataSize;
    size_t mAllocSize;
    uint8_t *mAllocData;
};

} // namespace urlsock

#endif
