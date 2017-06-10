#ifndef MATCHERS_HPP_
#define MATCHERS_HPP_

namespace urlsock
{

void pbuff(const uint8_t *data, size_t size)
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

} // namespace urlsock

#endif
