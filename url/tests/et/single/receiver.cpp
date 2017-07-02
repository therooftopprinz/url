#include <iostream>
#include <cstdlib>
#include <url/src/UrlSock.hpp>
#include <cstring>

int main(int argc, const char **argv)
{
    int src = std::atoi(argv[1]);

    std::cout << "receiver: " << std::endl;
    std::cout << "src port: "  << src << std::endl;

    urlsock::UrlSock sock(src);
    urlsock::Buffer receiveBuff;
    // urlsock::UdpEndpoint ttt(src);
    while (true)
    {
        urlsock::IpPort targeter;
        auto r = sock.receive(receiveBuff, targeter);
        if (r==urlsock::EReceiveResult::Ok)
        {
            std::cout << "packet received! size:" << receiveBuff.size() << std::endl; 
        }
        
        // auto rv = ttt.receive(receiveBuff, targeter);
        // std::cout << "packet = " << rv << " " << strerror(errno) << std::endl;
    }
}