#ifndef URL_TXJOBMANAGER_HPP_
#define URL_TXJOBMANAGER_HPP_

#include <memory>
#include <map>
#include <mutex>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"
#include "ITxJobManager.hpp"
#include "ITxJob.hpp"

namespace urlsock
{

class TxJobManager : public ITxJobManager
{
public:
    TxJobManager() = default;
    bool createITxJob(IpPortMessageId ipPortMessage, ITxJob& itxJob);
    bool deleteITxJob(IpPortMessageId ipPortMessage);
    bool reportAck(IpPortMessageId ipPortMessage, uint32_t offset);
    bool reportNack(IpPortMessageId ipPortMessage,  uint32_t offset, ENackReason reason);
private:
    std::map<IpPortMessageId,ITxJob&> mTxJobMap;
    std::mutex mTxJobMapMutex;
};

} // namespace urlsock

#endif
