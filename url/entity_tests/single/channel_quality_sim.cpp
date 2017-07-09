#include <vector>
#include <string>
#include <cstring>
#include <url/src/UrlSock.hpp>
#include <url/src/Logger.hpp>

using Door = std::pair<urlsock::UdpEndpoint, urlsock::IpPort>;

struct Tunnel
{
    Tunnel() = delete;
    Tunnel(Door&& a, Door&& b):
        a(std::move(a)),
        b(std::move(b))
    {}
    Door a;
    Door b;
};


class Tunneler
{
public:
    Tunneler():
        mLogger("Tunneler")
    {}
    void addTunnel(urlsock::IpPort remoteA, uint16_t hostA, urlsock::IpPort remoteB, uint16_t hostB)
    {
        mLogger << LOG_DEBUG << "Adding tunnel receiving from/to: "; 
        mLogger << LOG_DEBUG << "  A: " << urlsock::iptoa(urlsock::ipFromIpPort(remoteA)) << ":"
            << urlsock::portFromIpPort(remoteA) << " host: " << hostA;
        mLogger << LOG_DEBUG << "  B: " << urlsock::iptoa(urlsock::ipFromIpPort(remoteB)) << ":"
            << urlsock::portFromIpPort(remoteB) << " host: " << hostB;
        Tunnel tunnel(
            std::make_pair(urlsock::UdpEndpoint(hostA), remoteA),
            std::make_pair(urlsock::UdpEndpoint(hostB), remoteB));
        tunnel.a.first.orRecvFlag(MSG_DONTWAIT);
        tunnel.b.first.orRecvFlag(MSG_DONTWAIT);
        tunnel.a.first.orSendFlag(MSG_DONTWAIT);
        tunnel.b.first.orSendFlag(MSG_DONTWAIT);
        mTunnels.emplace_back(std::move(tunnel));
    }
    void runTunnels()
    {
        uint8_t tbuff[65536];
        while(true)
        {
            for (auto& tunnel : mTunnels)
            {
                urlsock::IpPort from;

                auto rs = tunnel.a.first.receive(urlsock::BufferView(tbuff, 65536), from);
                if (int64_t(rs)>0)
                {
                    auto ss = tunnel.b.first.send(urlsock::BufferView(tbuff, rs), tunnel.b.second);
                    mLogger << LOG_DEBUG << "tunnel from: " << urlsock::iptoa(urlsock::ipFromIpPort(from)) << ":"
                        << urlsock::portFromIpPort(from)
                        << " to: "<< urlsock::iptoa(urlsock::ipFromIpPort(tunnel.b.second)) << ":"
                        << urlsock::portFromIpPort(tunnel.b.second) << " with: " << int64_t(rs) << " bytes. sent:" << ss;
                }
                else
                {
                    // mLogger << LOG_ERROR << "(A) -1: Error(" << errno  << "): " << strerror(errno);
                }
                rs = tunnel.b.first.receive(urlsock::BufferView(tbuff, 65536), from);
                if (int64_t(rs)>0)
                {
                    auto ss = tunnel.a.first.send(urlsock::BufferView(tbuff, rs), tunnel.a.second);
                    mLogger << LOG_DEBUG << "tunnel from: " << urlsock::iptoa(urlsock::ipFromIpPort(from)) << ":"
                        << urlsock::portFromIpPort(from)
                        << " to: " << urlsock::iptoa(urlsock::ipFromIpPort(tunnel.a.second)) << ":"
                        << urlsock::portFromIpPort(tunnel.a.second) << " with: " << int64_t(rs) << " bytes. sent:" << ss;
                }
                else
                {
                    // mLogger << LOG_ERROR << "(B) -1: Error(" << errno << "): " << strerror(errno);
                }                
            }
        }
    }

private:
    std::vector<Tunnel> mTunnels;
    urlsock::Logger mLogger;
};

int main()
{
    Tunneler tunneler;
    tunneler.addTunnel(urlsock::IpPorter(127,0,0,1,10000), 10001, urlsock::IpPorter(127,0,0,1,10003), 10002);
    tunneler.runTunnels();
}