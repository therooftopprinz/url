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
    virtual bool createITxJob(IpPortMessageId ipPortMessage, std::shared_ptr<ITxJob>& itxJob) = 0;
    virtual bool deleteITxJob(IpPortMessageId ipPortMessage) = 0;
    virtual bool reportAck(IpPortMessageId ipPortMessage, uint32_t offset) = 0;
    virtual bool reportNack(IpPortMessageId ipPortMessage,  uint32_t offset, ENackReason reason) = 0;
};

} // namespace urlsock

#endif
