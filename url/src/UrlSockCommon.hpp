#ifndef URLSOCKCOMMON_HPP_
#define URLSOCKCOMMON_HPP_

#include <string>
#include <exception>
#include <arpa/inet.h>

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

inline IpPort IpPorter(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port){
    return uint64_t(d)<<56|uint64_t(c)<<48|uint64_t(b)<<40|uint64_t(a)<<32|htons(port);}
inline IpPort IpPorter(uint32_t a, uint16_t port){return (uint64_t(a)<<32)|htons(port);}
inline uint16_t portFromIpPort(IpPort ipPort){return ntohs(ipPort&0xFFFF);}
inline uint32_t ipFromIpPort(IpPort ipPort){return ipPort>>32;}

} // namespace urlsock

#endif
