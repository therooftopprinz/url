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
    MallocFailed(std::string message):
        whatString(message)
    {}

    inline const char* what()
    {
        return whatString.c_str();
    }

private:
    std::string whatString;   
}

class Buffer
{
public:
    Buffer(BufferView) = delete;
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

    /** Capacity **/
    bool empty();
    size_t size();
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
    uint8_t *mallocData;
};

} // namespace urlsock

#endif