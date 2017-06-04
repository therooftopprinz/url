#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <cstdint>
#include <stdexcept>
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

class Buffer
{
public:
    Buffer();
    Buffer(size_t bufferSize);
    Buffer(const void* start, const void* end);
    Buffer(const void* start, size_t size);
    Buffer(const Buffer& other);
    Buffer(Buffer&& other);
    ~Buffer();

    Buffer& operator=(const Buffer& other);
    Buffer& operator=(Buffer&& other);
    void assign(uint8_t* start, uint8_t* end);
    void own(void* start, size_t size);

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

template <typename T>
class BbVvT
{
public:
    BbVvT():
        mData(nullptr),
        mSize(0)
    {}

    BbVvT(const Buffer& buffer):
        mData(buffer.data()),
        mSize(buffer.size())
    {}

    BbVvT(Buffer& buffer):
        mData(buffer.data()),
        mSize(buffer.size())
    {}

    BbVvT(const BbVvT<T>& other):
        mData(other.mData),
        mSize(other.mSize)
    {}

    BbVvT(T start, T end):
        mData(start),
        mSize(uintptr_t(end)-uintptr_t(start))
    {}

    BbVvT(T start, size_t size):
        mData(start),
        mSize(size)
    {}

    BbVvT& operator=(const BbVvT<T>& other)
    {
        mData = other.mData;
        mSize = other.mSize;
        return *this;
    }

    inline T data()
    {
        return mData;
    }

    inline T data() const
    {
        return mData;
    }


    template<typename Tr>
    Tr& get(size_t offset)
    {
        if((offset+sizeof(Tr))>mSize)
        {
            throw std::out_of_range("buffer out of range");
        }
        return *reinterpret_cast<Tr*>(mData+offset);
    }

    template<typename Tr>
    const Tr& get(size_t offset) const
    {
        if((offset+sizeof(Tr))>mSize)
        {
            throw std::out_of_range("buffer out of range");
        }
        return *reinterpret_cast<Tr*>(mData+offset);
    }

    inline size_t size() const
    {
        return mSize;
    }
private:
    T mData;
    size_t mSize;
};

using BufferView = BbVvT<uint8_t*>;
using ConstBufferView = BbVvT<const uint8_t*>;

} // namespace urlsock

#endif
