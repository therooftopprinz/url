#include "UrlPduDisassembler.hpp"

namespace urlsock
{

UrlPduDisassembler::UrlPduDisassembler(ConstBufferView buffer):
    mPduData(buffer),
    mValidPdu(true)
{
    size_t decodeCur = 0;
    try
    {
        while (true)
        {
            auto curData = buffer.get<const uint32_t>(decodeCur);
            decodeCur += 4;
            switch (curData>>30)
            {
                case 0b00:
                    mHasAckHeader = true;
                    mOffset = curData&0x3FFFFFFF;
                    curData = buffer.get<const uint32_t>(decodeCur);
                    decodeCur += 4;
                    mMsgId = curData>>16;
                    mMac = curData&0xFFFF;
                    continue;
                case 0b01:
                    mHasInitialDataHeader = true;
                    mUrlMessageSize = curData&0x3FFFFFFF;
                    curData = buffer.get<const uint32_t>(decodeCur);
                    decodeCur += 4;
                    mMsgId = curData>>16;
                    mMac = curData&0xFFFF;
                    break;
                case 0b10:
                    mHasDataHeader = true;
                    mOffset = curData&0x3FFFFFFF;
                    curData = buffer.get<const uint32_t>(decodeCur);
                    decodeCur += 4;
                    mMsgId = curData>>16;
                    mMac = curData&0xFFFF;
                    break;
                case 0b11:
                    hanndleProtocolInformationHeaders(curData, decodeCur);
                    continue;
                default:
                    mValidPdu = false;
                    break;
            }
            break;
        }

        if (mHasDataHeader|mHasInitialDataHeader)
        {
            auto payloadSize = buffer.size()-decodeCur;
            if (payloadSize)
            {
                mPayload = ConstBufferView(buffer.data()+decodeCur, payloadSize);
            }
            else
            {
                mValidPdu = false;
            }
        }
    }
    catch (std::out_of_range& e)
    {
        mValidPdu = false;
    }
}

void UrlPduDisassembler::hanndleProtocolInformationHeaders(uint32_t headerStart, size_t& decodeCur)
{
    switch((headerStart>>27)&0b111)
    {
        case 0:
            mNackReason = (ENackReason)(headerStart&0x7FFFFFF);
            break;
        default:
            mValidPdu = false;
    }
}

bool UrlPduDisassembler::hasDataHeader()
{
    return mHasDataHeader;
}

bool UrlPduDisassembler::hasDataInitialHeader()
{
    return mHasInitialDataHeader;
}

bool UrlPduDisassembler::hasAckHeader()
{
    return mHasAckHeader;
}

bool UrlPduDisassembler::hasNackHeader()
{
    return mHasNackInfoHeader;
}

uint32_t UrlPduDisassembler::getOffset()
{
    return mOffset;
}

uint32_t UrlPduDisassembler::getTotalMessageSize()
{
    return mUrlMessageSize;
}

uint16_t UrlPduDisassembler::getMsgId()
{
    return mMsgId;
}

uint16_t UrlPduDisassembler::getMac()
{
    return mMac;
}

bool UrlPduDisassembler::getAcknowledgeMode()
{
    return mAcknowledgementMode;
}

uint8_t UrlPduDisassembler::getCipherAlgo()
{
    return mCipherAlgorithm;
}

uint8_t UrlPduDisassembler::getIntProtAlgo()
{
    return mIntProtAlgorithm;
}

ENackReason UrlPduDisassembler::getNackReason()
{
    return mNackReason;
}

bool UrlPduDisassembler::isValidPdu()
{
    return mValidPdu;
}

Buffer UrlPduDisassembler::getPayload()
{
    return Buffer(mPayload.data(), mPayload.size());
}

} // namespace urlsock