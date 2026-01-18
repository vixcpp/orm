/**
 *
 *  @file Sha256.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_SHA256_HPP
#define VIX_SHA256_HPP

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace vix::orm
{
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

#endif
