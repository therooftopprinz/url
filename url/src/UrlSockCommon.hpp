#ifndef URLSOCKCOMMON_HPP_
#define URLSOCKCOMMON_HPP_

#include <string>
#include <exception>

namespace urlsock
{

enum class ENackReason{DUPLICATE_SEGMENT_MISMATCHED, RECEIVED_SEGMENT_OVERLAPPED, RECEIVED_SEGMENT_OUT_OF_BOUND};

class BadCall : public std::exception
{
public:
    BadCall(std::string message):mMessage(message){}
    const char* what(){return mMessage.c_str();}
private:
    std::string mMessage;
};

} // namespace urlsock

#endif
