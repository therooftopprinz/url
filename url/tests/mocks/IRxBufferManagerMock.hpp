#ifndef IRXBUFFERMANAGERMOCK_HPP_
#define IRXBUFFERMANAGERMOCK_HPP_

#include <src/IRxBufferManager.hpp>

namespace urlsock
{

struct IRxBufferManagerMock : public IRxBufferManager
{
    MOCK_METHOD2(enqueue_rvalHack, void(IpPort, Buffer&));
    MOCK_METHOD0(dequeue, RxObject());
    void enqueue(IpPort ipPort, Buffer&& message)
    {
        enqueue_rvalHack(ipPort, message);
    }
};

} // namespace urlsock

#endif
