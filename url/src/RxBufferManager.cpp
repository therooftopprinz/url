#include "RxBufferManager.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

void RxBufferManager::enqueue(IpPort ipPort, Buffer&& message)
{
    std::lock_guard<std::mutex> lock(mRxBufferMutex);
    mRxBuffer.emplace_back(std::make_pair(ipPort, std::move(message)));
}

RxObject RxBufferManager::dequeue()
{
    std::unique_lock<std::mutex> lock(mRxBufferMutex);
    mRxBufferCv.wait_for(lock, std::chrono::milliseconds(1000),[this]()
        {
            return !mRxBuffer.empty();
        });

    if (!mRxBuffer.empty())
    {
        auto& rvi = mRxBuffer.back();
        auto rv = std::make_pair(rvi.first, std::move(rvi.second));
        mRxBuffer.pop_back();
        return rv;
    }
    return {};
}

} // namespace urlsock
