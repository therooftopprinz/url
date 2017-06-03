#ifndef URLPDUDISASSEMBLER_HPP_
#define URLPDUDISASSEMBLER_HPP_

#include <map>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

class UrlPduDisassembler
{
public:
    UrlPduDisassembler(ConstBufferView buffer);
    bool hasDataHeader();
    bool hasDataInitialHeader();
    bool hasAckHeader();
    bool hasNackHeader();
    uint32_t getOffset();
    uint32_t getTotalMessageSize();
    uint16_t getMsgId();
    uint16_t getMac();
    bool getAcknowledgeMode();
    uint8_t getCipherAlgo();
    uint8_t getIntProtAlgo();
    bool getAcknowledgementType();
    ENackReason getNackReason();
    bool isValidPdu();
    Buffer getPayload();

private:
    void hanndleProtocolInformationHeaders(uint32_t headerStart, size_t& decodeCur);

    ConstBufferView mPduData;
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
    bool mValidPdu;
    ConstBufferView mPayload;
};

} // namespace urlsock

#endif
