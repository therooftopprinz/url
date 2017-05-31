#ifndef URLSOCK_HPP_
#define URLSOCK_HPP_

#include "Buffer.hpp"
#include "RxSegmentAssembler.hpp"

namespace urlsock
{

enum class ESendResult {Ok, Unreachable, TooManyRetries};
enum class EReceiveResult {Ok, TimeOut, Incomplete};
using IpPort = uint64_t;
using MessageId = uint32_t;
using IpPortMessageId = std::pair<IpPort, MessageId>;

struct IUrlSock
{
    virtual ~IUrlSock() = default;
    virtual ESendResult send(const BufferView& payload, IpPort target) = 0;
    virtual ESendResult send(const Buffer& payload, IpPort target) = 0;
    virtual ESendResult send(Buffer&& payload, IpPort target) = 0;
    virtual EReceiveResult receive(Buffer& payload, IpPort& targeter) = 0;
};

struct IEndPoint
{
    virtual void send(const BufferView& payload, IpPort target) = 0;
    virtual void send(const Buffer& payload, IpPort target) = 0;
    virtual void send(Buffer&& payload, IpPort target) = 0;
    virtual void receive(Buffer& payload, IpPort& target) = 0;
};

struct ITxJob
{
    virtual bool eventAckReceived(uint32_t offset) = 0;
    virtual void run() = 0;
};

struct ITxJobManager
{
    virtual std::shared_ptr<ITxJob> getITxJobByIpPortMessageId(IpPortMessageId ipPortMessage) = 0;
    virtual createITxJob(IpPortMessageId ipPortMessage, std::shared_ptr<ITxJob>& itxJob);
    virtual deleteITxJob(IpPortMessageId ipPortMessage);
};

using RxObject = std::pair<IpPort, Buffer>;
using RxList = std::list<RxObject>;

struct IRxBufferManager
{
    virtual void enqueue(IpPort ipPort, Buffer&& message) = 0;
    virtual RxObject dequeue() = 0;
}

class RxContext
{
public:
    RxContext(IpPortMessageId ipPortMessageId, IEndPoint& enpoint)
    void receive(Buffer& data, uint32_t offset);
    bool isUrlMessageReady();
    Buffer claimUrlMessage();
private:
    IpPortMessageId mIpPortMessageId;
    IEndPoint& mEnpoint;
    RxSegmentAssembler assembler;
    std::list<std::pair<Buffer, uint32_t>> rxTmpBuffer;
};

class RxContextManager
{
public:
    std::shared_ptr<RxContext> getRxContextByIpPortMessageId(IpPortMessageId ipPortMessage) = 0;
    createITxJob(IpPortMessageId ipPortMessage, std::shared_ptr<RxContext>& itxJob);
    deleteITxJob(IpPortMessageId ipPortMessage);
private:
};

class RxJob
{
public:
    RxJob(ITxJobManager& itxManager, IRxBufferManager& rxBufferManager, IEndPoint& enpoint);
    ~RxJob();
private:
    void receiveThread();
    RxContextManager rxContextManager;
};

} // namespace urlsock

#endif
