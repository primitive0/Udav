export module udav.support.functional;

namespace support {

export template<typename... Fs>
class Overloaded : public Fs...
{
public:
    using Fs::operator()...;
};

}; // namespace support
