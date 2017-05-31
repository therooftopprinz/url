#ifndef URLPDUCODEC_HPP_
#define URLPDUCODEC_HPP_

#include <map>
#include "Buffer.hpp"

namespace urlsock
{

enum class ENackReason{DUPLICATE_SEGMENT_MISMATCHED, RECEIVED_SEGMENT_OVERLAPPED, RECEIVED_SEGMENT_OUT_OF_BOUND};

class UrlPduFactory
{
public:
    /** Data Transmit **/
    static Buffer createDataPdu(uint32_t offset, uint16_t msgId, uint16_t mac, const BufferView& payload);
    static Buffer createInitialDataPdu(uint32_t urlMessageSize, uint16_t msgId, uint16_t mac,
        bool retransmit, bool acknowledgedMode, uint8_t cipherAlgo, uint8_t intProtAlgo, const BufferView& payload);
    /** Acknowledgement **/
    static Buffer createAckPdu(uint32_t offset);
    static Buffer createNackPdu(uint32_t offset, ENackReason reason);
};

class UrlPduDisassembler
{
public:
    /** Data Transmit **/
    static Buffer createDataPdu(const BufferView& payload, uint32_t offset);
    static Buffer createInitialDataPdu(const BufferView& payload, uint32_t urlMessageSize, bool acknowledgedMode, bool reTrasnmit);
    /** Acknowledgement **/
    static Buffer createAckPdu(uint32_t offset);
    static Buffer createNackPdu(uint32_t offset, ENackReason reason);
};

} // namespace urlsock

#endif
