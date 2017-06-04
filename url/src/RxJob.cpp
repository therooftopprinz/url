#include "RxJob.hpp"
#include "ITxJobManager.hpp"
#include "IRxBufferManager.hpp"
#include "IEndPoint.hpp"
#include "RxSegmentAssembler.hpp"

namespace urlsock
{
 
RxJob::RxJob(ITxJobManager& itxManager, IRxBufferManager& rxBufferManager, IEndPoint& endpoint):
    mItxManager(itxManager),
    mRxBufferManager(rxBufferManager),
    mEnpoint(endpoint)
{
    mReceiveThread = std::thread(&RxJob::receiveThread, this);
}

RxJob::~RxJob()
{
    mReceiveThread.join();
}

void RxJob::receiveThread()
{
}

} // namespace urlsock
