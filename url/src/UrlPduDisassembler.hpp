#ifndef URL_URLPDUDISASSEMBLER_HPP_
#define URL_URLPDUDISASSEMBLER_HPP_

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
    bool hasInitialDataHeader();
    bool hasAckHeader();
    bool hasNackHeader();
    uint32_t getOffset();
    uint32_t getTotalMessageSize();
    uint16_t getMessageId();
    uint16_t getMac();
    bool isAcknowledgmentEnabled();
    uint8_t getCipherAlgo();
    uint8_t getIntProtAlgo();
    bool isRetransmit();
    ENackReason getNackReason();
    bool isValidPdu();
    Buffer getPayload();
    ConstBufferView& getPayloadView();

private:
    void handleProtocolInformationHeaders(uint32_t headerStart);

    ConstBufferView mPduData;
    bool mHasDataHeader;
    uint32_t mOffset;
    uint32_t mMsgId;
    uint32_t mMac;
    bool mHasInitialDataHeader;
    uint32_t mUrlMessageSize;
    bool mRetransmit;
    bool mIsAcknowledgeMode;
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
