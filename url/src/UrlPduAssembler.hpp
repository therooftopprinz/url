#ifndef URLPDUCODEC_HPP_
#define URLPDUCODEC_HPP_

#include <map>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

class UrlPduAssembler
{
public:
    UrlPduAssembler();
    /** Data Transmit **/
    void setDataHeader(uint32_t offset, uint16_t msgId, uint16_t mac);
    void setInitialDataHeader(uint32_t urlMessageSize, uint16_t msgId, uint16_t mac,
        bool retransmit, bool acknowledgedMode, uint8_t cipherAlgo, uint8_t intProtAlgo);
    /** Acknowledgement **/
    void setAckHeader(uint32_t offset, uint16_t msgId, uint16_t mac);
    void setNackInfoHeader(ENackReason reason);
    void setPayload(ConstBufferView payload);
    Buffer create();

private:
    inline size_t calculateHeaderSize();
    inline void createDataHeader(Buffer& buffer);
    inline void createInitialDataHeader(Buffer& buffer);
    inline void createNackInfoHeader(Buffer& buffer);
    inline void createAckHeader(Buffer& buffer);
    inline void createHeaders(Buffer& buffer);
    inline void putPayload(Buffer& buffer);
    inline void validate();

    bool mHasDataHeader;
    uint32_t mOffset;
    uint32_t mMsgId;
    uint32_t mMac;
    bool mHasInitialDataHeader;
    uint32_t mUrlMessageSize;
    bool mRetransmit;
    bool mAcknowledgementMode;
    uint8_t mCipherAlgorithm;
    uint8_t mIntProtAlgorithm;
    bool mHasAckHeader;
    bool mHasNackInfoHeader;
    ENackReason mNackReason;
    ConstBufferView mPayload;
};

} // namespace urlsock

#endif
