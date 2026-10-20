#pragma once

#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace pwgen {

inline constexpr std::size_t defaultLength = 20;
inline constexpr std::size_t minLength = 1;
inline constexpr std::size_t maxLength = 1024;

class Error : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

struct Options {
    std::size_t length = defaultLength;
    std::size_t count = 1;
    bool lower = true;
    bool upper = true;
    bool digits = true;
    bool symbols = false;
    bool excludeAmbiguous = false;
    std::string exclude;
    bool requireEach = false;
    bool showEntropy = false;
    bool help = false;
};

[[nodiscard]] Options parseOptions(std::span<const std::string_view> args);
[[nodiscard]] std::string_view usage() noexcept;

}
