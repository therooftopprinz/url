#include <cstring>
#include <thread>
#include "IEndPointMock.hpp"

namespace urlsock
{

size_t IEndPointMock::send(const Buffer& payload, IpPort target)
{
    return sendCommon(payload.data(), payload.size(), target);
}
size_t IEndPointMock::send(const BufferView& payload, IpPort target)
{
    return sendCommon(payload.data(), payload.size(), target);
}

size_t IEndPointMock::send(Buffer&& payload, IpPort target)
{
    return sendCommon(payload.data(), payload.size(), target);
}

size_t IEndPointMock::receive(Buffer& payload, IpPort& port)
{
    return receiveCommon(payload.data(), payload.size(), port);
}
size_t IEndPointMock::receive(BufferView&& payload, IpPort& port)
{
    return receiveCommon(payload.data(), payload.size(), port);
}

void IEndPointMock::toReceive(Buffer buffer, IpPort port)
{
    std::lock_guard<std::mutex> lock(mToReceiveMutex);
    mToReceive.emplace_back(std::make_pair(buffer, port));
    mToReceiveCv.notify_all();
}

void IEndPointMock::expectSend(uint32_t id, uint32_t prerequisite, bool chainable, uint32_t cardinality,
    std::function<bool(const void *buffer, size_t size, IpPort ipPort)> matcher,
    std::function<void(const void *buffer, size_t size, IpPort ipPort)> action)
{
    ExpectationContainer con;
    con.id = id;
    con.prerequisite = prerequisite;
    con.chainable = chainable;
    con.cardinality = cardinality;
    con.occurence = 0;
    con.matcher = matcher;
    con.action = action;
    mExpectations.push_back(con);
}

size_t IEndPointMock::sendCommon(const uint8_t* buffer, size_t size, IpPort ipPort)
{
    bool matched = false;
    uint32_t index = 0;
    for (auto &i : mExpectations)
    {
        // if (i.cardinality == i.occurence && i.cardinality != 0)
        // {
        //     // log << logger::DEBUG << "Satified for index: " << (unsigned int) index;
        //     continue;
        // }
        
        // log << logger::DEBUG << "Matching for index: " << (unsigned int) index;

        bool dependencyMet = true;
        if (i.prerequisite != 0)
        {
            // log << logger::DEBUG << "Finding id for prerequisite: " << i.prerequisite;
            auto req = std::find_if(mExpectations.begin(), mExpectations.end(),
                [&i](const ExpectationContainer &pre){ return i.prerequisite == pre.id;});

            if (req == mExpectations.end())
            {
                // log << logger::DEBUG << "Prerequisite id not found...";
                dependencyMet = false;

            }
            if (req != mExpectations.end() && req->occurence < req->cardinality)
            {
                // log << logger::DEBUG << "prerequisite occurence < cardinality";
                dependencyMet = false;
            }
        }

        if (!dependencyMet)
        {
            index++;
            continue;
        }

        if (i.matcher (buffer, size, ipPort))
        {
            std::cout << "Matched! id:" << i.id << std::endl;
            i.occurence++;
            i.action(buffer, size, ipPort);
            matched = true;
            if (!i.chainable)
            {
                break;
            }
        }
        else
        {
            // log << logger::DEBUG << "Matching failed for index: " << (unsigned int) index;
        }

        index++;
    }

    if (!matched)
    {
        // log << logger::DEBUG << "No match found!";
    }
    return size;
}

size_t IEndPointMock::receiveCommon(uint8_t *data, size_t size, IpPort& port)
{
    std::unique_lock<std::mutex> lock(mToReceiveMutex);
    mToReceiveCv.wait_for(lock, std::chrono::milliseconds(100), [this](){return !mToReceive.empty();});
    if (mToReceive.empty())
    {
        return 0;
    }
    auto& front = mToReceive.front();
    auto sz = front.first.size();
    assert(sz<size);
    port = front.second;
    std::memcpy(data, front.first.data(), sz);
    mToReceive.pop_front();
    return sz;
}

void IEndPointMock::failed(std::string msg)
{
    std::cout  << "EndPointMock failed with: " << msg << std::endl;
    EXPECT_TRUE(false);
}

void IEndPointMock::waitForAllSending(double milliseconds)
{
    std::chrono::duration<double, std::milli> waitingtime(milliseconds);
    uint32_t index = 0;
    std::ostringstream os;
    bool allMatched = true;

    uint32_t waitct = (uint32_t)(milliseconds/100.0);

    for (uint32_t i = 0; i < waitct; i++)
    {
        bool stop = true;
        for (const auto& ii : mExpectations)
        {
            if ((ii.cardinality == ii.occurence && ii.cardinality != static_cast<uint32_t>(-1)) ||
                (ii.cardinality == static_cast<uint32_t>(-1) && ii.occurence > 0))
            {
                continue;
            }
            stop = false;
        }
        if (stop)
        {
            break;  
        }
        using namespace std::chrono_literals;   
        std::this_thread::sleep_for(waitingtime/waitct);
    }

    std::cout << "checking expectations: " << std::endl;
    for (const auto& i : mExpectations)
    {
        std::cout << "index:" << (unsigned int)index << " id: " << i.id << ", occurence: " <<
            i.occurence << " cardinality " << i.cardinality << std::endl;

        if ((i.cardinality == i.occurence && i.cardinality != static_cast<uint32_t>(-1)) ||
            (i.cardinality == static_cast<uint32_t>(-1) && i.occurence > 0))
        {
            index++;
            continue;
        }

        allMatched = false;
        index++;
    }

    if (!allMatched)
    {
        std::cout << "Unmatched send expectations above (number of expectations: " << mExpectations.size() << "):" << std::endl;
        failed("There are unsatified send messages!!");
    }
}


} // namespace urlsock
