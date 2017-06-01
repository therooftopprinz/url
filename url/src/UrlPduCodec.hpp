#ifndef URLPDUCODEC_HPP_
#define URLPDUCODEC_HPP_

#include <map>
#include "Buffer.hpp"

namespace urlsock
{

enum class ENackReason{DUPLICATE_SEGMENT_MISMATCHED, RECEIVED_SEGMENT_OVERLAPPED, RECEIVED_SEGMENT_OUT_OF_BOUND};

class BadCall : public std::exception
{
public:
    BadCall(std::string message):mMessage(message){}
    const char* what(){return mMessage.c_str();}
private:
    std::string mMessage;
};

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
    void setPayload(const BufferView& payload);
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
    const BufferView mPayload;
};

class UrlPduDisassembler
{
public:
    UrlPduDisassembler(const BufferView& buffer);
    bool hasDataHeader();
    bool hasDataInitialHeader();
    bool hasAckHeader();
    bool hasNackHeader();
    uint32_t getOffsetOrTotalMessageSize();
    uint16_t getMsgId();
    uint16_t getMac();
    bool getAcknowledgeMode();
    uint8_t getCipherAlgo();
    uint8_t getIntProtAlgo();
    bool getAcknowledgementType();
    ENackReason getNackReason();
    Buffer getPayload();

private:
    const BufferView& mPduData;
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
};

} // namespace urlsock

#endif
