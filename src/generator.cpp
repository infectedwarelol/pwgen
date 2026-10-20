#include "generator.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <iterator>
#include <utility>

namespace pwgen {

namespace {

bool contains(std::string_view text, char character) noexcept {
    return text.find(character) != std::string_view::npos;
}

char pickFrom(std::string_view characters, std::random_device& device) {
    std::uniform_int_distribution<std::size_t> distribution(0, characters.size() - 1);
    return characters[distribution(device)];
}

}

std::vector<std::string> buildCharacterSets(const Options& options) {
    const auto isExcluded = [&options](char character) {
        return (options.excludeAmbiguous && contains(ambiguousCharacters, character)) ||
               contains(options.exclude, character);
    };

    std::vector<std::string> sets;
    const auto addSet = [&](bool enabled, std::string_view source) {
        if (!enabled) {
            return;
        }
        std::string filtered;
        std::ranges::copy_if(source, std::back_inserter(filtered),
                             [&](char character) { return !isExcluded(character); });
        if (!filtered.empty()) {
            sets.push_back(std::move(filtered));
        }
    };

    addSet(options.lower, lowercaseLetters);
    addSet(options.upper, uppercaseLetters);
    addSet(options.digits, digitCharacters);
    addSet(options.symbols, symbolCharacters);

    if (sets.empty()) {
        throw Error("character set is empty: enable at least one set or exclude fewer characters");
    }
    return sets;
}

std::size_t poolSize(std::span<const std::string> sets) noexcept {
    std::size_t size = 0;
    for (const auto& set : sets) {
        size += set.size();
    }
    return size;
}

std::string generatePassword(std::span<const std::string> sets, std::size_t length,
                             bool requireEach, std::random_device& device) {
    if (poolSize(sets) == 0) {
        throw Error("character set is empty");
    }
    if (requireEach && length < sets.size()) {
        throw Error(std::format("length {} is smaller than the {} required character sets", length,
                                sets.size()));
    }

    std::string pool;
    for (const auto& set : sets) {
        pool += set;
    }

    std::string password;
    password.reserve(length);
    if (requireEach) {
        for (const auto& set : sets) {
            password.push_back(pickFrom(set, device));
        }
    }
    while (password.size() < length) {
        password.push_back(pickFrom(pool, device));
    }
    if (requireEach) {
        std::ranges::shuffle(password, device);
    }
    return password;
}

double entropyBits(std::size_t length, std::size_t pool) {
    if (pool == 0) {
        return 0.0;
    }
    return static_cast<double>(length) * std::log2(static_cast<double>(pool));
}

}
