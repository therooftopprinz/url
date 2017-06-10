#ifndef IENDPOINTMOCK_HPP_
#define IENDPOINTMOCK_HPP_

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <list>
#include <src/IEndPoint.hpp>

namespace urlsock
{

class IEndPointMock : public IEndPoint
{
public:
    MOCK_METHOD2(send, void(const BufferView&, IpPort));
    MOCK_METHOD2(send, void(const Buffer&, IpPort));
    MOCK_METHOD2(send_rvalHack, void(Buffer&, IpPort));
    void send(Buffer&& payload, IpPort target)
    {
        send_rvalHack(payload, target);
    }

    size_t receive(Buffer& buffer, IpPort& port)
    {
        std::lock_guard<std::mutex> lock(mToReceiveMutex);
        if (mToReceive.empty())
        {
            return 0;
        }

        auto& front = mToReceive.front();
        auto sz = front.first.size();
        buffer = front.first;
        port = front.second;
        mToReceive.pop_front();
        return sz;
    }
    size_t receive(BufferView&& payload, IpPort& port)
    {
        std::unique_lock<std::mutex> lock(mToReceiveMutex);
        mToReceiveCv.wait_for(lock, std::chrono::milliseconds(100), [this](){return !mToReceive.empty();});
        lock.unlock();
        lock.lock();
        if (mToReceive.empty())
        {
            return 0;
        }
        auto& front = mToReceive.front();
        auto sz = front.first.size();
        port = front.second;
        std::memcpy(payload.data(), front.first.data(), sz);
        mToReceive.pop_front();
        return sz;
    }

    void toReceive(Buffer buffer, IpPort port)
    {
        std::lock_guard<std::mutex> lock(mToReceiveMutex);
        mToReceive.emplace_back(std::make_pair(buffer, port));
        mToReceiveCv.notify_all();
    }
private:
    std::list<std::pair<Buffer, IpPort>> mToReceive;
    std::condition_variable mToReceiveCv;
    std::mutex mToReceiveMutex;
};

} // namespace urlsock

#endif
