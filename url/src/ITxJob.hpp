#ifndef URL_ITXJOB_HPP_
#define URL_ITXJOB_HPP_

namespace urlsock
{

struct ITxJob
{
    virtual ~ITxJob() = default;
    virtual void eventAckReceived(uint32_t offset) = 0;
    virtual void eventNackReceived(uint32_t offset, ENackReason nackReason) = 0;
    virtual ESendResult run() = 0;
};

} // namespace urlsock

#endif
