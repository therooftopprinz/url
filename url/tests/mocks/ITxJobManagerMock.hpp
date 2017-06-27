#ifndef ITXJOBMANAGERMOCK_HPP_
#define ITXJOBMANAGERMOCK_HPP_

#include <src/ITxJobManager.hpp>

namespace urlsock
{

struct ITxJobManagerMock : public ITxJobManager
{
    MOCK_METHOD2(createITxJob, bool(IpPortMessageId, ITxJob&));
    MOCK_METHOD1(deleteITxJob, bool(IpPortMessageId));
    MOCK_METHOD2(reportAck, bool(IpPortMessageId, uint32_t offset));
    MOCK_METHOD3(reportNack, bool(IpPortMessageId, uint32_t offset, ENackReason));
};

} // namespace urlsock

#endif
 