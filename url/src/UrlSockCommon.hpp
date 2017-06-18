#ifndef URLSOCKCOMMON_HPP_
#define URLSOCKCOMMON_HPP_

#include <string>
#include <exception>

namespace urlsock
{

constexpr size_t UDP_MAX_SIZE = 65536u;

enum class ENackReason{DUPLICATE_SEGMENT_MISMATCHED, RECEIVED_SEGMENT_OVERLAPPED, RECEIVED_SEGMENT_OUT_OF_BOUND};

class BadCall : public std::exception
{
public:
    BadCall(std::string message):mMessage(message){}
    const char* what(){return mMessage.c_str();}
private:
    std::string mMessage;
};

enum class ESendResult {Ok, Unreachable, TooManyRetries};
enum class EReceiveResult {Ok, TimeOut, Incomplete};
using IpPort = uint64_t;
using MessageId = uint16_t;
using IpPortMessageId = std::pair<IpPort, MessageId>;

inline IpPort IpPorter(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint16_t port){return a<<56|b<<48|c<<40|d<<32|port;}

} // namespace urlsock

#endif
