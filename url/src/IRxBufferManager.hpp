#ifndef IRXBUFFERMANAGER_HPP_
#define IRXBUFFERMANAGER_HPP_

#include <list>
#include "Buffer.hpp"

namespace urlsock
{

using RxObject = std::pair<IpPort, Buffer>;
using RxList = std::list<RxObject>;

struct IRxBufferManager
{
    virtual ~IRxBufferManager() = default;
    virtual void enqueue(IpPort ipPort, Buffer&& message) = 0;
    virtual RxObject dequeue() = 0;
};

} // namespace urlsock

#endif
