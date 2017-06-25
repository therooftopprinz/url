#ifndef IENDPOINTMOCK_HPP_
#define IENDPOINTMOCK_HPP_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <list>
#include <src/IEndPoint.hpp>

namespace urlsock
{

typedef std::function<bool(const void*,size_t,IpPort)> MatcherFunctor;
typedef std::function<void(const void*,size_t,IpPort)> ActionFunctor;

class DefaultAction
{
public:
    inline static ActionFunctor get()
    {
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        return std::bind(&DefaultAction::action, _1, _2, _3);
    }

private:
    inline static void action(const void *buffer, size_t size, IpPort ipPort)
    {
    }
};

class IEndPointMock : public IEndPoint
{
public:
    size_t send(const Buffer&, IpPort);
    size_t send(const BufferView&, IpPort);
    size_t receive(Buffer& buffer, IpPort& port);
    size_t receive(BufferView&& payload, IpPort& port);
    void toReceive(Buffer buffer, IpPort port);
    void expectSend(uint32_t id, uint32_t prerequisite, bool chainable, uint32_t cardinality,
        std::function<bool(const void *buffer, size_t size, IpPort ipPort)> matcher,
        std::function<void(const void *buffer, size_t size, IpPort ipPort)> action);
    void waitForAllSending(double milliseconds);

private:
    size_t sendCommon(const uint8_t* buffer, size_t size, IpPort ipPort);
    size_t receiveCommon(uint8_t *data, size_t size, IpPort& port);
    void failed(std::string msg);

    struct ExpectationContainer
    {
        uint32_t cardinality = 0;
        uint32_t occurence = 0;
        MatcherFunctor matcher;
        ActionFunctor action;
        uint32_t id = 0;
        uint32_t prerequisite = 0;
        bool chainable = false;
    };

    std::list<std::pair<Buffer, IpPort>> mToReceive;
    std::condition_variable mToReceiveCv;
    std::mutex mToReceiveMutex;
    std::list<ExpectationContainer> mExpectations;
};

} // namespace urlsock

#endif
