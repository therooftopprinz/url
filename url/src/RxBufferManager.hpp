#ifndef URL_RXBUFFERMANAGER_HPP_
#define URL_RXBUFFERMANAGER_HPP_

#include <list>
#include <mutex>
#include <condition_variable>
#include "IRxBufferManager.hpp"

namespace urlsock
{

class RxBufferManager : public IRxBufferManager
{
public:
    void enqueue(IpPort ipPort, Buffer&& message);
    RxObject dequeue();
private:
    std::list<RxObject> mRxBuffer;
    std::mutex mRxBufferMutex;
    std::condition_variable mRxBufferCv;
};

} // namespace urlsock

#endif
