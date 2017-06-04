#ifndef ITXJOB_HPP_
#define ITXJOB_HPP_

namespace urlsock
{

struct ITxJob
{
    virtual ~ITxJob() = default;
    virtual bool eventAckReceived(uint32_t offset) = 0;
    virtual void run() = 0;
};

} // namespace urlsock

#endif
