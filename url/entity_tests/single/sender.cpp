#include <iostream>
#include <cstdlib>
#include <url/src/UrlSock.hpp>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

int main(int argc, const char **argv)
{
    int src = std::atoi(argv[1]);
    int dst = std::atoi(argv[2]);
    int size = std::atoi(argv[3]);
    int count = std::atoi(argv[4]);

    std::cout << "sender: " << std::endl;
    std::cout << "source port: "  << src << std::endl;
    std::cout << "destination port: "  << dst << std::endl;
    std::cout << "url packet size: "  << size << std::endl;
    std::cout << "packet count: "  << count << std::endl;

    urlsock::Buffer toSend(size);
    urlsock::UrlSock sock(src);
    auto target = urlsock::IpPorter(127,0,0,1,dst);
    // urlsock::UdpEndpoint ttt(src);
    for (int i=0; i<count; i++)
    {
        auto rv = sock.send(toSend, target);
        // auto rv = ttt.send(toSend, target);
        std::cout << "packet["<< i << "] = " << uint32_t(rv) << strerror(errno) << std::endl;
    }
}