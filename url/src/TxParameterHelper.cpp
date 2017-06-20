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
    mLastTimeoutTime(0),
    mMeanTimeout(static_cast<uint64_t>(-1)),
    mNSamples(0),
    mLastTimeoutWindow(minTimeoutWindowSize)
{
}

void TxParameterHelper::newTimeout(uint32_t timeout)
{
    mLastTimeoutTime = std::chrono::duration_cast<std::chrono::microseconds>(
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
    mLastTimeoutTime = mMaxTimeoutWindowSize>mLastTimeoutTime ? mLastTimeoutTime : mMaxTimeoutWindowSize;
    mNSamples++;
}


double TxParameterHelper::channelQuality()
{
    auto lastTimeOut = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count() - mLastTimeoutTime;
    if (mLastTimeoutWindow>lastTimeOut)
    {
        lastTimeOut = 0;
    }
    else
    {
        lastTimeOut -= mLastTimeoutWindow;
    }

    double x = mMeanTimeout/mMaxTimeoutWindowSize;
    double y = lastTimeOut/mMaxTimeoutWindowSize;

    return (std::sin(x*1.5)+0.5*std::sin(y*1.5))/1.4;
}

uint32_t TxParameterHelper::getOptimalSegmentSize()
{
    uint32_t mtu = (mMaxNMtu-mMinNMtu)*channelQuality()+mMinNMtu;
    return mtu*mMtuSize;
}

uint32_t TxParameterHelper::getOptimalTimeout()
{
    uint32_t to = (mMinTimeoutWindowSize-mMaxTimeoutWindowSize)*channelQuality()+mMaxTimeoutWindowSize;
    mLastTimeoutWindow = to*mMtuSize;
    return mLastTimeoutWindow;
}

uint32_t TxParameterHelper::getOptimalUackPacket()
{
    uint32_t nmtu = (mMinNMtu-mMaxNMtu)*channelQuality()+mMaxNMtu;
    return nmtu;
}

} // namespace urlsock
