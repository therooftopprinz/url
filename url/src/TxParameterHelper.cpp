#include <chrono>
#include <cmath>
#include "TxParameterHelper.hpp"
namespace urlsock
{

TxParameterHelper::TxParameterHelper(uint32_t mtuSize,
        uint32_t maxUackPacket, uint32_t minUackPacket,
        uint32_t maxTimeoutWindowSize, uint32_t minTimeoutWindowSize,
        uint32_t maxNMtu, uint32_t minNMtu):
    mMtuSize(mtuSize),
    mMaxUackPacket(maxUackPacket),
    mMinUackPacket(minUackPacket),
    mMaxTimeoutWindowSize(maxTimeoutWindowSize),
    mMinTimeoutWindowSize(minTimeoutWindowSize),
    mMaxNMtu(maxNMtu),
    mMinNMtu(minNMtu),
    mLastTimeout(0),
    mMeanTimeout(static_cast<uint64_t>(-1)),
    mNSamples(0)
{
}

void TxParameterHelper::newTimeout(uint32_t timeout)
{
    mLastTimeout = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    if (mMeanTimeout == static_cast<uint64_t>(-1))
    {
        mMeanTimeout = timeout;
    }
    else
    {
        mMeanTimeout = mMeanTimeout + (timeout - mMeanTimeout)/mNSamples;
    }

    mMeanTimeout = mMaxTimeoutWindowSize>mMeanTimeout ? mMeanTimeout : mMaxTimeoutWindowSize;
    mLastTimeout = mMaxTimeoutWindowSize>mLastTimeout ? mMeanTimeout : mMaxTimeoutWindowSize;
    mNSamples++;
}


double TxParameterHelper::channelQuality()
{
    double x = mMeanTimeout/mMaxTimeoutWindowSize;
    double y = mLastTimeout/mMaxTimeoutWindowSize;

    return (sin(x*1.5)+0.5*sin(y*1.5))/1.4;
}

uint32_t TxParameterHelper::getOptimalSegmentSize()
{
    uint32_t mtu = (mMaxNMtu-mMinNMtu)*channelQuality()+mMinNMtu;
    return mtu*mMtuSize;
}

uint32_t TxParameterHelper::getOptimalTimeout()
{
    uint32_t mtu = (mMinTimeoutWindowSize-mMaxTimeoutWindowSize)*channelQuality()+mMaxTimeoutWindowSize;
    return mtu*mMtuSize;
}

} // namespace urlsock
