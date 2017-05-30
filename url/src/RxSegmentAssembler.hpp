#ifndef RXSEGMENTASSEMBLER_HPP_
#define RXSEGMENTASSEMBLER_HPP_

#include <map>
#include "Buffer.hpp"

namespace urlsock
{

class RxSegmentAssembler
{
public:
    enum class EReceiveStatus {COMPLETE, INCOMPLETE, INCORRECT_RTX_DATA, INCORRECT_RTX_SIZE, DATA_OUTOFBOUNDS};
    RxSegmentAssembler();
    ~RxSegmentAssembler();
    void init(size_t size);
    bool isInited();
    uint8_t* claim();
    EReceiveStatus receive(const Buffer& data, uint32_t offset);
    EReceiveStatus receive(const BufferView& data, uint32_t offset);
    EReceiveStatus receive(const uint8_t *start, size_t size, uint32_t offset);
private:
    uint8_t *mUrlMessage;
    size_t mUrlMessageTotalSize;
    size_t mUrlMessageReceivedSize;
    std::map<uint32_t, uint32_t> mReceivedBlocks;
};

} // namespace urlsock

#endif
