#ifndef MATCHERS_HPP_
#define MATCHERS_HPP_

namespace urlsock
{

static void pbuff(const uint8_t *data, size_t size)
{
    std::cout << "Buffer("<<(void*)data << "): ";
    for (auto i=0u; i<size; i++)
    {
        std::cout << std::hex << uint32_t(data[i]) << " ";
    }
    std::cout << std::endl;
}

MATCHER_P(IsBufferEq, b, "")
{
    std::cout << "matching send  sArg=" << std::dec << arg.size() << " sB=" << b.size() << std::endl;
    pbuff(arg.data(), arg.size());
    pbuff(b.data(), b.size());

    bool rv = arg.size()==b.size() && !std::memcmp(arg.data(), b.data(), b.size());
    std::cout << "rv: " << rv << std::endl;
    return rv;
}

class Matcher
{
public:
    MatcherFunctor get()
    {
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        return std::bind(&Matcher::match, this, _1, _2, _3);
    }
protected:
    virtual bool match(const void *, size_t, IpPort)
    {
        return false;
    }
};

class MessageMatcher : public Matcher
{
public:
    MessageMatcher(Buffer msg, IpPort ipPort):
        mMsg(msg),
        mIpPort(ipPort)
    {}

    void set(Buffer msg, IpPort ipPort)
    {
        mMsg = msg;
        mIpPort = ipPort;
    }

private:
    bool match(const void *buffer, size_t size, IpPort ipPort)
    {
        // log << logger::WARNING << "Trying full match with:";
        // utils::printRaw(msg.data(), msg.size());
        if(mMsg.size()!=size || mIpPort!=ipPort)
        {            
            // log << logger::WARNING << "Inequal sizes!";
            return false;
        }
        if (!std::memcmp(buffer, mMsg.data(), size))
        {
            pbuff(mMsg.data(), mMsg.size());
            std::cout << "Send: sent and expected." << std::endl;
            return true;
        }

        // log << logger::WARNING << "Message didn't match. ";
        return false;
    }

    Buffer mMsg;
    IpPort mIpPort;
};

class AnyMessageMatcher : public Matcher
{
public:
    AnyMessageMatcher(IpPort ipPort):
        mIpPort(ipPort)
    {
    }
private:
    bool match(const void *buffer, size_t size, IpPort ipPort)
    {
        return mIpPort==ipPort;
    }
    IpPort mIpPort;
};

} // namespace urlsock

#endif
