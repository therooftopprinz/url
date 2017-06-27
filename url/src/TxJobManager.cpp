#include "TxJobManager.hpp"

namespace urlsock
{

bool TxJobManager::createITxJob(IpPortMessageId ipPortMessage, ITxJob& itxJob)
{
    std::lock_guard<std::mutex> lock(mTxJobMapMutex);
    auto  ir = mTxJobMap.emplace(ipPortMessage, itxJob);
    return ir.second;
}

bool TxJobManager::deleteITxJob(IpPortMessageId ipPortMessage)
{
    std::lock_guard<std::mutex> lock(mTxJobMapMutex);
    auto found = mTxJobMap.find(ipPortMessage);
    if (found==mTxJobMap.end())
    {
        return false;
    }
    mTxJobMap.erase(found);
    return true;
}

bool TxJobManager::reportAck(IpPortMessageId ipPortMessage, uint32_t offset)
{
    std::lock_guard<std::mutex> lock(mTxJobMapMutex);
    auto found = mTxJobMap.find(ipPortMessage);
    if (found==mTxJobMap.end())
    {
        return false;
    }
    found->second.eventAckReceived(offset);
    return true;
}

bool TxJobManager::reportNack(IpPortMessageId ipPortMessage,  uint32_t offset, ENackReason reason)
{
    std::lock_guard<std::mutex> lock(mTxJobMapMutex);
    auto found = mTxJobMap.find(ipPortMessage);
    if (found==mTxJobMap.end())
    {
        return false;
    }
    found->second.eventNackReceived(offset, reason);
    return true;
}


} // namespace urlsock
