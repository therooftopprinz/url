#ifndef IENDPOINTMOCK_HPP_
#define IENDPOINTMOCK_HPP_

#include "src/IEndPointMock.hpp"

namespace urlsock
{

struct IEndPointMock : public IEndPoint
{
    MOCK_METHOD2(send, void(const BufferView&, IpPort));
    MOCK_METHOD2(send, void(const Buffer&, IpPort));
    MOCK_METHOD2(send, void(Buffer&&, IpPort));
    MOCK_METHOD2(receive, size_t(Buffer&, IpPort&));
    MOCK_METHOD2(receive, size_t(BufferView&&, IpPort&));
};

} // namespace urlsock

#endif
