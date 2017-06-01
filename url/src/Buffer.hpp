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
    BufferView():
        mData(nullptr),
        mSize(0)
    {}

    BufferView(const BufferView& other):
        mData(other.mData),
        mSize(other.mSize)
    {}

    BufferView(uint8_t* start, uint8_t* end):
        mData(start),
        mSize(uintptr_t(end)-uintptr_t(start))
    {}

    BufferView(uint8_t* start, size_t size):
        mData(start),
        mSize(size)
    {}

    static const BufferView createFrom(const uint8_t* start, size_t size)
    {
        const BufferView rv;
        rv.mData = (uint8_t*)start;
        rv.mSize = size;
        return rv;
    }

    static const BufferView createFrom(const uint8_t* start, const uint8_t* end)
    {
        const BufferView rv;
        rv.mData = (uint8_t*)start;
        rv.mSize = uintptr_t(end)-uintptr_t(start);
        return rv;
    }


    const BufferView& operator=(const BufferView& other) const
    {
        mData = other.mData;
        mSize = other.mSize;
        return *this;
    }

    inline uint8_t* data()
    {
        return mData;
    }

    inline const uint8_t* data() const
    {
        return mData;
    }

    template<typename T>
    T& get(size_t offset)
    {
        return *(T*)(mData+offset);
    }

    template<typename T>
    const T& get(size_t offset) const
    {
        return *(T*)(mData+offset);
    }

    inline size_t size() const
    {
        return mSize;
    }
private:
    mutable uint8_t *mData;
    mutable size_t mSize;
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

    template<typename T>
    T& get(size_t offset)
    {
        return *(T*)(mAllocData+offset);
    }

    template<typename T>
    const T& get(size_t offset) const
    {
        return *(T*)(mAllocData+offset);
    }

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
