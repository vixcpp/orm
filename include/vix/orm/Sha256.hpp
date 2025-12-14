#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace vix::orm
{
    // Minimal SHA-256 (no external deps). Good for checksums.
    class Sha256
    {
    public:
        Sha256() { reset(); }

        void reset();
        void update(const void *data, std::size_t len);
        void update(std::string_view s) { update(s.data(), s.size()); }

        std::array<std::uint8_t, 32> digest();
        static std::string hex(std::array<std::uint8_t, 32> d);

    private:
        void transform(const std::uint8_t block[64]);

        std::uint64_t bitlen_ = 0;
        std::array<std::uint32_t, 8> state_{};
        std::array<std::uint8_t, 64> buffer_{};
        std::size_t buffer_len_ = 0;
    };

    inline std::string sha256_hex(std::string_view s)
    {
        Sha256 h;
        h.update(s);
        return Sha256::hex(h.digest());
    }
}
