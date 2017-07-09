#ifndef URL_TXPARAMETERHELPER_HPP_
#define URL_TXPARAMETERHELPER_HPP_

namespace urlsock
{

class TxParameterHelper
{
public:
    TxParameterHelper(uint32_t mtuSize,
        uint32_t maxUackPacket, uint32_t minUackPacket,
        uint32_t maxTimeoutWindowSize, uint32_t minTimeoutWindowSize,
        uint32_t maxNMtu, uint32_t minNMtu);
    void newTimeout(uint32_t timeout);
    uint32_t getOptimalSegmentSize();
    uint32_t getOptimalTimeout();
    uint32_t getOptimalUackPacket();

private:
    double channelQuality();
    const uint32_t mMtuSize;
    const uint32_t mMaxUackPacket;
    const uint32_t mMinUackPacket;
    const uint32_t mMaxTimeoutWindowSize;
    const uint32_t mMinTimeoutWindowSize;
    const uint32_t mMaxNMtu;
    const uint32_t mMinNMtu;

    uint64_t mLastTimeoutTime;
    uint64_t mMeanTimeout;
    uint64_t mNSamples;
    uint32_t mLastTimeoutWindow;
    double mChannelQuality;
};

} // namespace urlsock

#endif
