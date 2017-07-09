#ifndef URL_RXSEGMENTASSEMBLER_HPP_
#define URL_RXSEGMENTASSEMBLER_HPP_

#include <map>
#include "Buffer.hpp"

namespace urlsock
{

class RxSegmentAssembler
{
public:
    enum class EReceivedSegmentStatus {COMPLETE, INCOMPLETE, INCORRECT_RTX_DATA, INCORRECT_RTX_SIZE, DATA_OUTOFBOUNDS, DATA_OVERLAPPED};
    RxSegmentAssembler();
    ~RxSegmentAssembler();
    void initUrlMessageSize(size_t size);
    bool isInited();
    Buffer claim();
    const uint8_t* peek() const;
    size_t size() const;
    size_t receivedSize() const;
    EReceivedSegmentStatus receive(const ConstBufferView& data, uint32_t offset);
    EReceivedSegmentStatus receive(const uint8_t *start, size_t size, uint32_t offset);
private:
    uint8_t *mUrlMessage;
    size_t mUrlMessageTotalSize;
    size_t mUrlMessageReceivedSize;
    std::map<uint32_t, uint32_t> mReceivedBlocks;
};

} // namespace urlsock

#endif
