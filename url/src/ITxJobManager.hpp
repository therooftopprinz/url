#ifndef ITXJOBMANAGER_HPP_
#define ITXJOBMANAGER_HPP_

#include <memory>
#include "Buffer.hpp"
#include "UrlSockCommon.hpp"
#include "ITxJob.hpp"

namespace urlsock
{

struct ITxJobManager
{
    virtual ~ITxJobManager() = default;
    virtual std::shared_ptr<ITxJob> getITxJobByIpPortMessageId(IpPortMessageId ipPortMessage) = 0;
    virtual void createITxJob(IpPortMessageId ipPortMessage, std::shared_ptr<ITxJob>& itxJob);
    virtual void deleteITxJob(IpPortMessageId ipPortMessage);
    virtual void reportAck(IpPortMessageId ipPortMessage, uint32_t offset);
    virtual void reportNack(IpPortMessageId ipPortMessage,  uint32_t offset, ENackReason reason);
};

} // namespace urlsock

#endif
