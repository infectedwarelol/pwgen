#include "generator.hpp"
#include "options.hpp"

#include <exception>
#include <format>
#include <iostream>
#include <random>
#include <span>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    try {
        const std::span arguments(argv, static_cast<std::size_t>(argc));
        const std::vector<std::string_view> args(arguments.begin() + 1, arguments.end());
        const pwgen::Options options = pwgen::parseOptions(args);

        if (options.help) {
            std::cout << pwgen::usage();
            return 0;
        }

        const auto sets = pwgen::buildCharacterSets(options);
        std::random_device device;
        for (std::size_t index = 0; index < options.count; ++index) {
            std::cout << pwgen::generatePassword(sets, options.length, options.requireEach, device)
                      << '\n';
        }

        if (options.showEntropy) {
            const double bits = pwgen::entropyBits(options.length, pwgen::poolSize(sets));
            std::cout << std::format("entropy: {:.1f} bits ({})\n", bits,
                                     pwgen::strengthLabel(bits));
        }
        return 0;
    } catch (const pwgen::Error& error) {
        std::cerr << "pwgen: " << error.what() << "\nTry 'pwgen --help' for usage.\n";
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "pwgen: " << error.what() << '\n';
        return 1;
    }
}
