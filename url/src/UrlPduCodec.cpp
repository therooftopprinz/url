#include "UrlPduCodec.hpp"
#include <cstring>

namespace urlsock
{

UrlPduAssembler::UrlPduAssembler():
    mHasDataHeader(false),
    mHasInitialDataHeader(false),
    mHasAckHeader(false),
    mHasNackInfoHeader(false)
{}

void UrlPduAssembler::setDataHeader(uint32_t offset, uint16_t msgId, uint16_t mac)
{
    mHasDataHeader = true;
    mOffset = offset;
    mMsgId = msgId;
    mMac = mac;
}

void UrlPduAssembler::setInitialDataHeader(uint32_t urlMessageSize, uint16_t msgId, uint16_t mac,
    bool retransmit, bool acknowledgedMode, uint8_t cipherAlgo, uint8_t intProtAlgo)
{
    mHasInitialDataHeader = true;
    mUrlMessageSize = urlMessageSize;
    mMsgId = msgId;
    mMac = mac;
    mRetransmit = retransmit;
    mAcknowledgementMode = acknowledgedMode;
    mCipherAlgorithm = cipherAlgo;
    mIntProtAlgorithm = intProtAlgo;
}

void UrlPduAssembler::setAckHeader(uint32_t offset, uint16_t msgId, uint16_t mac)
{
    mHasAckHeader = true;
    mOffset = offset;
    mMsgId = msgId;
    mMac = mac;
}

void UrlPduAssembler::setNackInfoHeader(ENackReason reason)
{
    mHasNackInfoHeader = true;
    mNackReason = reason;
}

void UrlPduAssembler::setPayload(const BufferView& payload)
{
    mPayload = payload;
}

Buffer UrlPduAssembler::create()
{
    validate();
    size_t pduSize = calculateHeaderSize()+mPayload.size();
    Buffer buffer(pduSize);
    createHeaders(buffer);
    putPayload(buffer);
    return buffer;
}

size_t UrlPduAssembler::calculateHeaderSize()
{
    size_t pduSize = 0;
    pduSize += mHasAckHeader ? 8:0;
    pduSize += mHasInitialDataHeader ? 12:0;
    pduSize += mHasDataHeader ? 8:0;
    pduSize += mHasNackInfoHeader ? 4:0;
    return pduSize;
}

void UrlPduAssembler::createDataHeader(Buffer& buffer)
{
    buffer.get<uint32_t>(0) = 
        (0b10ul<<30) | (mOffset&0x3FFFFFFF);
    buffer.get<uint32_t>(4) =
        (mMsgId << 16) | (mMac);
}

void UrlPduAssembler::createInitialDataHeader(Buffer& buffer)
{
    buffer.get<uint32_t>(0) = 
        (0b01ul<<30) | (mUrlMessageSize&0x3FFFFFFF);
    buffer.get<uint32_t>(4) =
        (mMsgId << 16) | (mMac);
    buffer.get<uint32_t>(8) =
        (mIntProtAlgorithm&0xF) |
        (mCipherAlgorithm&0xF)<<4 |
        (mAcknowledgementMode? 1ul<<8 : 0) |
        (mRetransmit? 1ul<<9 : 0);

}

void UrlPduAssembler::createNackInfoHeader(Buffer& buffer)
{
    buffer.get<uint32_t>(0) = 
        (0b11ul<<30) | (uint32_t(mNackReason)&0x3FFFFFFF);
    buffer.get<uint32_t>(4) = (mOffset&0x3FFFFFFF);
    buffer.get<uint32_t>(8) =
        (mMsgId << 16) | (mMac);
}

void UrlPduAssembler::createAckHeader(Buffer& buffer)
{
    buffer.get<uint32_t>(0) = mOffset&0x3FFFFFFF;
    buffer.get<uint32_t>(4) =
        (mMsgId << 16) | (mMac);
}

void UrlPduAssembler::createHeaders(Buffer& buffer)
{
    if (mHasDataHeader)
    {
        createDataHeader(buffer);
    }
    else if (mHasInitialDataHeader)
    {
        createInitialDataHeader(buffer);
    }
    else if (mHasNackInfoHeader)
    {
        createNackInfoHeader(buffer);
    }
    else
    {
        createAckHeader(buffer);
    }
}

void UrlPduAssembler::putPayload(Buffer& buffer)
{
    size_t dataOffset = calculateHeaderSize();
    std::memcpy(buffer.data()+dataOffset, mPayload.data(), mPayload.size());
}

void UrlPduAssembler::validate()
{
    if (!(mHasDataHeader||mHasInitialDataHeader||(mHasAckHeader|mHasNackInfoHeader)) ||
        (mHasDataHeader&&mHasInitialDataHeader) ||
        (mHasInitialDataHeader&&(mHasAckHeader|mHasNackInfoHeader)) ||
        (mHasDataHeader&&(mHasAckHeader|mHasNackInfoHeader)))
    {
        throw BadCall("Invalid header combination.");
    }
    if (mHasAckHeader&&mPayload.size())
    {
        throw BadCall("Payload on non data pdu.");
    }
    if (!mHasAckHeader&&mHasNackInfoHeader)
    {
        throw BadCall("Nack information header without ack header.");
    }
}

} // namespace urlsock
