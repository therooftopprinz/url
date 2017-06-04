#ifndef IRXBUFFERMANAGERMOCK_HPP_
#define IRXBUFFERMANAGERMOCK_HPP_

#include "IRxBufferManager.hpp"

namespace urlsock
{

struct IRxBufferManagerMock : public struct IRxBufferManager
{
    MOCK_METHOD2(enqueue, void(IpPort, Buffer&&));
    MOCK_METHOD0(dequeue, RxObject());
}

} // namespace urlsock

#endif
