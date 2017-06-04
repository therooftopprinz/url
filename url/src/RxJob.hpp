#ifndef RXJOB_HPP_
#define RXJOB_HPP_

#include <map>
#include <thread>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"

namespace urlsock
{

class ITxJobManager;
class IRxBufferManager;
class IEndPoint;
class RxSegmentAssembler;
 
class RxJob
{
public:
    RxJob(ITxJobManager& itxManager, IRxBufferManager& rxBufferManager, IEndPoint& enpoint);
    ~RxJob();
private:
    void receiveThread();

    ITxJobManager& mItxManager;
    IRxBufferManager& mRxBufferManager;
    IEndPoint& mEnpoint;
    std::thread mReceiveThread;
    std::map<IpPortMessageId, RxSegmentAssembler> mRxContexts;
};

} // namespace urlsock

#endif
