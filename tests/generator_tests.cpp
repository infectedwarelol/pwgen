#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "generator.hpp"
#include "options.hpp"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace {

pwgen::Options parse(std::initializer_list<std::string_view> args) {
    const std::vector<std::string_view> list(args);
    return pwgen::parseOptions(list);
}

bool containsAny(std::string_view text, std::string_view characters) {
    return text.find_first_of(characters) != std::string_view::npos;
}

}

TEST_CASE("generated password has requested length") {
    std::random_device device;
    const auto sets = pwgen::buildCharacterSets(pwgen::Options{});
    for (const std::size_t length : {1U, 20U, 1024U}) {
        CHECK(pwgen::generatePassword(sets, length, false, device).size() == length);
    }
}

TEST_CASE("characters come only from enabled sets") {
    std::random_device device;
    const auto sets = pwgen::buildCharacterSets(parse({"--no-upper", "--no-digits"}));
    const auto password = pwgen::generatePassword(sets, 500, false, device);
    CHECK(std::ranges::all_of(password, [](char character) {
        return pwgen::lowercaseLetters.find(character) != std::string_view::npos;
    }));
}

TEST_CASE("symbols are off by default and on with --symbols") {
    CHECK(pwgen::poolSize(pwgen::buildCharacterSets(pwgen::Options{})) == 62);
    CHECK(pwgen::poolSize(pwgen::buildCharacterSets(parse({"--symbols"}))) ==
          62 + pwgen::symbolCharacters.size());
}

TEST_CASE("exclusion rules remove characters") {
    std::random_device device;

    SUBCASE("ambiguous characters") {
        const auto sets = pwgen::buildCharacterSets(parse({"--exclude-ambiguous"}));
        CHECK(pwgen::poolSize(sets) == 56);
        CHECK_FALSE(containsAny(pwgen::generatePassword(sets, 1000, false, device),
                                pwgen::ambiguousCharacters));
    }

    SUBCASE("custom characters") {
        const auto sets = pwgen::buildCharacterSets(parse({"--exclude", "abcXYZ789"}));
        CHECK(pwgen::poolSize(sets) == 53);
        CHECK_FALSE(containsAny(pwgen::generatePassword(sets, 1000, false, device), "abcXYZ789"));
    }

    SUBCASE("everything excluded is rejected") {
        CHECK_THROWS_AS((void)pwgen::buildCharacterSets(
                            parse({"--no-lower", "--no-upper", "--exclude", "0123456789"})),
                        pwgen::Error);
    }
}

TEST_CASE("require-each guarantees one character from every set") {
    std::random_device device;
    const auto sets = pwgen::buildCharacterSets(parse({"--symbols", "--require-each"}));
    for (int attempt = 0; attempt < 500; ++attempt) {
        const auto password = pwgen::generatePassword(sets, 4, true, device);
        for (const auto& set : sets) {
            CHECK(containsAny(password, set));
        }
    }
}

TEST_CASE("require-each rejects length below number of sets") {
    std::random_device device;
    const auto sets = pwgen::buildCharacterSets(parse({"--symbols"}));
    CHECK_THROWS_AS((void)pwgen::generatePassword(sets, 3, true, device), pwgen::Error);
}

TEST_CASE("entropy math") {
    CHECK(pwgen::entropyBits(20, 62) == doctest::Approx(20.0 * std::log2(62.0)));
    CHECK(pwgen::entropyBits(8, 2) == doctest::Approx(8.0));
    CHECK(pwgen::entropyBits(10, 1) == doctest::Approx(0.0));
    CHECK(pwgen::strengthLabel(39.9) == "weak");
    CHECK(pwgen::strengthLabel(40.0) == "fair");
    CHECK(pwgen::strengthLabel(60.0) == "strong");
    CHECK(pwgen::strengthLabel(80.0) == "very strong");
}

TEST_CASE("option parsing accepts valid flags") {
    const auto options =
        parse({"-l", "32", "--count", "5", "--no-lower", "--symbols", "--exclude-ambiguous",
               "--exclude", "xyz", "--require-each", "--entropy"});
    CHECK(options.length == 32);
    CHECK(options.count == 5);
    CHECK_FALSE(options.lower);
    CHECK(options.upper);
    CHECK(options.symbols);
    CHECK(options.excludeAmbiguous);
    CHECK(options.exclude == "xyz");
    CHECK(options.requireEach);
    CHECK(options.showEntropy);
    CHECK(parse({"--help"}).help);
}

TEST_CASE("option parsing rejects bad input") {
    CHECK_THROWS_AS((void)parse({"--bogus"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l", "abc"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l", "12x"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l", "-5"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l", ""}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l", "0"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l", "1025"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-l"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"-c", "0"}), pwgen::Error);
    CHECK_THROWS_AS((void)parse({"--exclude"}), pwgen::Error);
}
