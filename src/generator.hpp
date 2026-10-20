#pragma once

#include "options.hpp"

#include <cstddef>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace pwgen {

inline constexpr std::string_view lowercaseLetters = "abcdefghijklmnopqrstuvwxyz";
inline constexpr std::string_view uppercaseLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
inline constexpr std::string_view digitCharacters = "0123456789";
inline constexpr std::string_view symbolCharacters = "!@#$%^&*()-_=+[]{};:,.<>/?~";
inline constexpr std::string_view ambiguousCharacters = "0Oo1lI";

[[nodiscard]] std::vector<std::string> buildCharacterSets(const Options& options);

[[nodiscard]] std::size_t poolSize(std::span<const std::string> sets) noexcept;

[[nodiscard]] std::string generatePassword(std::span<const std::string> sets, std::size_t length,
                                           bool requireEach, std::random_device& device);

[[nodiscard]] double entropyBits(std::size_t length, std::size_t pool);

[[nodiscard]] constexpr std::string_view strengthLabel(double bits) noexcept {
    if (bits < 40.0) {
        return "weak";
    }
    if (bits < 60.0) {
        return "fair";
    }
    if (bits < 80.0) {
        return "strong";
    }
    return "very strong";
}

}
